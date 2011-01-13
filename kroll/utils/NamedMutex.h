
#ifndef _NAMED_MUTEX_H_
#define _NAMED_MUTEX_H_

#include <base.h>
#include <string>

#ifdef OS_WIN32
#include <windows.h>
#endif

namespace UTILS_NS
{
	class KROLL_API NamedMutex
	{
	public:
		NamedMutex(const std::string& name);
		~NamedMutex();
		void lock();
		bool tryLock();
		void unlock();

	private:
		const std::string  _name;
#ifdef OS_WIN32
		HANDLE       _mutex;
#elif OS_OSX
		sem_t* _sem;
#else
		int _lockfd; // lock file descriptor
		int _semfd;  // file used to identify semaphore
		int _semid;  // semaphore id
#endif
	};
}
#endif // _NAMED_MUTEX_H_