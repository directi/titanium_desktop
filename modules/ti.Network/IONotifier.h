#ifndef _IO_NOTIFIER_H_
#define _IO_NOTIFIER_H_

#include <Poco/Mutex.h>
#include <Poco/Thread.h>
#include <Poco/Observer.h>
#include <Poco/Semaphore.h>
#include <Poco/Notification.h>

#include <list>

#include <kroll/kroll.h>

using namespace Poco;
using namespace Poco::Net;

namespace ti 
{
	enum IOEventType {IO_READ, IO_WRITE, IO_ERROR};
	static inline const char* IOEventTypeStr(IOEventType ev)
	{
		switch(ev){
			case IO_ERROR:
				return "IO_Error";
			case IO_READ:
				return "IO_Read";
			case IO_WRITE:
				return "IO_Write";
			default:
				return "IO_UNKNOWN";
		}
	}
	class IONotification : public Notification 
	{
		public:
			IONotification(int fd, IOEventType ev) : ev(ev), fd(fd)
			{
			}
			int getFd() const { return fd; }
			IOEventType getEvent() const { return ev; }
		private:
			const int fd;
			const IOEventType ev;
	};

	template <class C>
	class IOObserver: public Observer<C, IONotification>
	{
		public:
			IOObserver(int fd, C& object, Callback method):
			  Observer<C, IONotification>(object, method),
				  fd(fd)
			{
			}
			int getFd() const { return fd; }
			bool equals(const AbstractObserver& abstractObserver) const
			{
				const IOObserver* pObs = dynamic_cast<const IOObserver*>(&abstractObserver);
				return Observer::equals(*pObs) && pObs->fd == this->fd;
			}
			bool operator==(const IOObserver<C> &rhs) const
			{ 
				return this->equals(rhs);
			}

		private:
			const int fd;
	};

	template <class C>
	class IONotifier {
		static kroll::Logger* GetLogger()
		{
			return kroll::Logger::Get("Network.IONotifier");
		}

	public:
		typedef std::list<IOObserver<C> > observer_t;
		typedef typename observer_t::iterator observer_itr_t;

		IONotifier(int fdCheckInterval = 100):
			fdCheckInterval(fdCheckInterval*1000),
			waitForWriteFd(0,64),
			waitForReadFd(0,64),
			_stop(false)
		{
			readThread.setName("IONotifierRead");
			writeThread.setName("IONotifierWrite");
			readThread.start(IONotifier::_runReader, (void*) this);
			writeThread.start(IONotifier::_runWriter, (void*) this);
		}
		~IONotifier()
		{
			_stop = true;
			waitForReadFd.set();
			waitForWriteFd.set();
			writeThread.join();
			readThread.join();
		}
		//bool containsObserver(IOEventType ev, IOObserver<C> &observer);
		bool addObserver(IOEventType ev, IOObserver<C> observer);
		bool removeObserver(IOEventType ev, IOObserver<C> &observer);
		int observerCount(IOEventType ev)
		{
			return observer_set[ev].size();
		}
		void runReader();
		void runWriter();
		static void _runReader(void* ioNotifier){
			IONotifier<C> *that = (IONotifier<C>*) (ioNotifier);
			that->runReader();
		}
		static void _runWriter(void* ioNotifier){
			IONotifier<C> *that = (IONotifier<C>*) (ioNotifier);
			that->runWriter();
		}
	protected:
		int select(bool read, bool write, bool error);
		int prepareFdSet(IOEventType ev, fd_set &fdSet)
		{
			FD_ZERO(&fdSet);
			if( observer_set[ev].size() == 0)
			{
				return 0;
			}
			int count=0;
			for (observer_itr_t it = observer_set[ev].begin();
				it != observer_set[ev].end();
				++it)
			{
				IOObserver<C> & obs = *it;
				FD_SET(obs.getFd(), &fdSet);
				count++;
			}
			return count;
		}
		int lastError()
		{
			#if defined(_WIN32)
				return WSAGetLastError();
			#else
				return errno;
			#endif
		}
		void stop()
		{
			_stop = true;
			if (0 == observerCount())
			{
				waitForFd.set();
			}
		}

		observer_t  observer_set[3]; 
		void notifyFdSet(IOEventType ev, fd_set &fdSet);
	private: 
		const long fdCheckInterval;
		Semaphore waitForWriteFd;
		Semaphore waitForReadFd;
		Poco::Mutex observerLock; 
		Thread readThread;
		Thread writeThread;
		bool _stop;
	};


		template <class C>
	int IONotifier<C>::select(bool read, bool write, bool error)
	{
		fd_set fdRead;
		fd_set fdWrite;
		fd_set fdExcept;
		int nfd = 0;
		FD_ZERO(&fdRead);
		FD_ZERO(&fdWrite);
		FD_ZERO(&fdExcept);
		if(read)
		{
			nfd += this->prepareFdSet(IO_READ, fdRead);
		}
		if( write )
		{
			nfd += this->prepareFdSet(IO_WRITE, fdWrite);
		}
		if( error )
		{
			nfd += this->prepareFdSet(IO_ERROR, fdExcept);
		}
		if (nfd == 0) return 0;

		int rc;
		struct timeval tv;
		Poco::Timespan remainingTime(this->fdCheckInterval);
		tv.tv_sec  = (long) remainingTime.totalSeconds();
		tv.tv_usec = (long) remainingTime.useconds();
		Poco::Timestamp start;
		rc = ::select(nfd + 1, &fdRead, &fdWrite, &fdExcept, &tv);
		if (rc <= 0 ) 
		{
			//if interrupted or on timeout, allow check for updates in fdSet
			return rc;
		}
		this->notifyFdSet(IO_READ, fdRead);
		this->notifyFdSet(IO_WRITE, fdWrite);
		this->notifyFdSet(IO_ERROR, fdExcept);
		return rc; 
	}

	template <class C>
	void IONotifier<C>::runReader()
	{
		while(!_stop)
		{
			int readyCount = select(true, false, true);
			if( observerCount(IO_READ) + observerCount(IO_ERROR)  == 0 ){
				waitForReadFd.wait();
			}
		}
	}
	template <class C>
	void IONotifier<C>::runWriter()
	{
		while(!_stop)
		{
			int readyCount = select(false, true, false);
			if( observerCount(IO_WRITE) == 0 ){
				waitForWriteFd.wait();
			}		
		}
	}
	template <class C>
	void IONotifier<C>::notifyFdSet(IOEventType ev, fd_set &fdSet)
	{
		std::list<IOObserver<C> > observers;
		{
			Poco::Mutex::ScopedLock lock(observerLock);
			if(observer_set[ev].size() == 0 )
			{
				return;
			}
			for (observer_itr_t it = observer_set[ev].begin();
				it != observer_set[ev].end();
				++it)
			{
				IOObserver<C> & obs = *it;
				if( FD_ISSET(obs.getFd(), &fdSet) ){
					observers.push_back(obs);
				}
			}
		}
		for(std::list<IOObserver<C> >::iterator it = observers.begin(); it != observers.end(); it++)
		{
			IOObserver<C> & obs = *it;
			IONotification notification(obs.getFd(), ev);
			GetLogger()->Debug("Notifying %s on fd %d", IOEventTypeStr(ev), it->getFd());
			obs.notify(&notification);
		}
	}

	/*
	template <class C>
	bool IONotifier<C>::containsObserver(IOEventType ev, IOObserver<C> &observer)
	{
		observer_t set = observer_set[ev];
		observer_itr_t it = set.find(observer);
		if( it == set.end()){
			return false;
		}
		return true;
	}
	*/

	template <class C>
	bool IONotifier<C>::addObserver(IOEventType ev, IOObserver<C> observer)
	{
		Poco::Mutex::ScopedLock lock(observerLock);

		if(observer_set[ev].size() > 0 )
		{
			observer_itr_t it  = std::find(observer_set[ev].begin(), observer_set[ev].end(), observer);
			if(it != observer_set[ev].end())
			{
				return false;
			}
		}
		observer_set[ev].push_back(observer);
		GetLogger()->Debug("%x: Added fd %d for %s", this, observer.getFd(), IOEventTypeStr(ev));

		if( ev == IO_WRITE)
		{
			waitForWriteFd.set();
		}
		if( (ev == IO_READ || ev == IO_ERROR) )
		{
			waitForReadFd.set();
		}
		return true;

	}

	template <class C>
	bool IONotifier<C>::removeObserver(IOEventType ev, IOObserver<C> &observer)
	{
		Poco::Mutex::ScopedLock lock(observerLock);

		if(observer_set[ev].size() == 0 )
		{
			return false;
		}
		observer_itr_t it  = std::find(observer_set[ev].begin(), observer_set[ev].end(), observer);
		if(it == observer_set[ev].end())
		{
			return false;
		}
		GetLogger()->Debug("%x: Removed fd %d from %s", this, it->getFd(), IOEventTypeStr(ev));
		observer_set[ev].erase(it);
		return true;
	}
}
#endif