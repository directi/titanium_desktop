/**
* @author: Mital Vora <mital.d.vora@gmail.com>
*/

#ifndef _SOCKET_UTILS_H_
#define _SOCKET_UTILS_H_

#include <kroll/kroll.h>

// TODO: This is poco UnWindows.h's curse.... to be removed with poco
#ifdef UNICODE
#define CreateEvent  CreateEventW
#else
#define CreateEvent  CreateEventA
#endif // !UNICODE

#include <string>
#include <deque>

#include <asio.hpp>
#include <asio/detail/mutex.hpp>

using asio::ip::tcp;

#include <boost/bind.hpp>

#define BUFFER_SIZE 1024   // choose a reasonable size to send back to JS

namespace ti
{
	template <class T>
	class Socket
		: public StaticBoundObject
	{
	public:
		Socket<T>(Host *host, const std::string & name);
		virtual ~Socket();

		static void initialize();
		static void uninitialize();

	protected:
		static std::auto_ptr<asio::io_service> io_service;
		static std::auto_ptr<asio::io_service::work> io_idlework;
		static std::auto_ptr<asio::thread> io_thread;


		Host* ti_host;
		T *socket;

		asio::detail::mutex write_mutex;
		std::deque<std::string> write_buffer;
		char read_data_buffer[BUFFER_SIZE + 1];
		bool non_blocking;
		enum SOCK_STATE_en { SOCK_CLOSED, SOCK_CONNECTING, SOCK_CONNECTED, SOCK_CLOSING } sock_state;

		void on_read(char * data, int size);
		void on_error(const std::string& error_text);
		void on_close();

		inline static kroll::Logger* GetLogger()
		{
			return kroll::Logger::Get("Socket.TCPSocket");
		}

		void registerHandleRead();
		virtual bool CompleteClose()=0;

	private:

		KMethodRef onRead;
		KMethodRef onError;
		KMethodRef onClose;

		void SetOnRead(const ValueList& args, KValueRef result)
		{
			this->onRead = args.at(0)->ToMethod();
		}

		void SetOnError(const ValueList& args, KValueRef result)
		{
			this->onError = args.at(0)->ToMethod();
		}

		void SetOnClose(const ValueList& args, KValueRef result)
		{
			this->onClose = args.at(0)->ToMethod();
		}
		void Write(const ValueList& args, KValueRef result);
		void Read(const ValueList& args, KValueRef result);
		void Close(const ValueList& args, KValueRef result);
		void IsClosed(const ValueList& args, KValueRef result);

		void registerHandleWrite();
		void handleWrite(const asio::error_code& error, std::size_t bytes_transferred);
		void writeAsync(const std::string &data);

		bool writeSync(const std::string &data);
		bool write(const std::string &data);
		std::string read();

		void handleRead(const asio::error_code& error, std::size_t bytes_transferred);
	};

	template <class T1, class T2>
	void copyHandlers(Socket<T1> *a, Socket<T2> *b)
	{
		a->onError = b->onError;

	}


	template <class T>
	std::auto_ptr<asio::io_service> Socket<T>::io_service(new asio::io_service());

	template <class T>
	std::auto_ptr<asio::io_service::work> Socket<T>::io_idlework(
		new asio::io_service::work(*io_service));

	template <class T>
	std::auto_ptr<asio::thread> Socket<T>::io_thread(NULL);

	template <class T>
	void Socket<T>::initialize()
	{
		io_thread.reset(new asio::thread(
			boost::bind(&asio::io_service::run, Socket::io_service.get())));
	}
	
	template <class T>
	void Socket<T>::uninitialize()
	{
		io_service->stop();
		io_thread->join();
		io_idlework.reset();
		io_thread.reset();
		io_service.reset();
	}

	template <class T>
	Socket<T>::Socket(Host *host, const std::string & name)
		: StaticBoundObject(name.c_str()),
	ti_host(host),
	socket(NULL),
	non_blocking(false),
	sock_state(SOCK_CLOSED)
	{
		this->SetMethod("onRead",&Socket::SetOnRead);
		this->SetMethod("onError",&Socket::SetOnError);
		this->SetMethod("onClose",&Socket::SetOnClose);

		this->SetMethod("read",&Socket::Read);
		this->SetMethod("write",&Socket::Write);

		this->SetMethod("isClosed",&Socket::IsClosed);
		this->SetMethod("close",&Socket::Close);
	}

	template <class T>
	Socket<T>::~Socket()
	{
		if (socket)
		{
			this->CompleteClose();
			delete socket;
			socket = NULL;
		}
	}


	template <class T>
	void Socket<T>::on_read(char * data, int size)
	{
		if(!this->onRead.isNull()) 
		{
			BytesRef bytes(new Bytes(data, size));
			ValueList args (Value::NewObject(bytes));
			RunOnMainThread(this->onRead, args, false);
			return;
		}
		GetLogger()->Warn("Socket::onRead: not read subscriber registered:  " + string(data));
	}

	template <class T>
	void Socket<T>::on_error(const std::string& error_text)
	{
		if(!this->onError.isNull()) 
		{
			ValueList args (Value::NewString(error_text.c_str()));
			RunOnMainThread(this->onError, args, false);
		}
	}

	template <class T>
	void Socket<T>::on_close()
	{
		if(!this->onClose.isNull()) 
		{
			ValueList args;
			RunOnMainThread(this->onClose, args, false);
		}
	}

	template <class T>
	void Socket<T>::Write(const ValueList& args, KValueRef result)
	{
		try
		{
			std::string data = args.at(0)->ToString();
			result->SetBool(this->write(data));
		}
		catch(SocketException &e)
		{
			throw ValueException::FromString(e.what());
		}
	}

	template <class T>
	void Socket<T>::Read(const ValueList& args, KValueRef result)
	{
		try
		{
			std::string data = this->read();
			BytesRef bytes(new Bytes(data.c_str(), data.size()));
			result->SetValue(Value::NewObject(bytes));
		}
		catch(SocketException &e)
		{
			throw ValueException::FromString(e.what());
		}
	}

	template <class T>
	void Socket<T>::Close(const ValueList& args, KValueRef result)
	{
		result->SetBool(this->CompleteClose());
	}

	template <class T>
	void Socket<T>::IsClosed(const ValueList& args, KValueRef result)
	{
		return result->SetBool(this->sock_state == SOCK_CLOSED);
	}

	

	template <class T>
	void Socket<T>::registerHandleWrite()
	{
		asio::async_write(*socket,
			asio::buffer(write_buffer.front().c_str(), write_buffer.front().size()),
			boost::bind(&Socket::handleWrite, this,
			asio::placeholders::error, asio::placeholders::bytes_transferred));

	}

	template <class T>
	void Socket<T>::handleWrite(const asio::error_code& error, std::size_t bytes_transferred)
	{
		if (error)
		{
			this->on_error(error.message());
			return;
		}
		asio::detail::mutex::scoped_lock lock(write_mutex);
		write_buffer.pop_front();
		if (!write_buffer.empty())
		{
			this->registerHandleWrite();
		}
	}

	template <class T>
	void Socket<T>::writeAsync(const std::string &data)
	{
		asio::detail::mutex::scoped_lock lock(write_mutex);
		bool write_in_progress = !write_buffer.empty();
		write_buffer.push_back(data);
		if (!write_in_progress)
		{
			this->registerHandleWrite();
		}
	}

	template <class T>
	bool Socket<T>::writeSync(const std::string &data)
	{
		try
		{
			asio::write(*socket, asio::buffer(data.c_str(), data.size()));
		}
		catch(asio::system_error & e)
		{
			this->CompleteClose();
			this->on_error(e.what());
			return false;
		}
		return true;
	}

	template <class T>
	bool Socket<T>::write(const std::string &data)
	{
		if (this->sock_state != SOCK_CONNECTED)
		{
			throw TCPSocketWriteException();
		}
		if(non_blocking)
		{
			writeAsync(data);
			return true;
		}
		return writeSync(data);
	}

	template <class T>
	void Socket<T>::handleRead(const asio::error_code& error, std::size_t bytes_transferred)
	{
		if (error)
		{
			this->on_error(error.message());
			return;
		}
		this->on_read(read_data_buffer, bytes_transferred);
		this->registerHandleRead();
	}


	template <class T>
	void Socket<T>::registerHandleRead()
	{
		asio::async_read(*socket,
			asio::buffer(read_data_buffer, BUFFER_SIZE),
			asio::transfer_at_least(1),
			boost::bind(&Socket::handleRead, this,
			asio::placeholders::error, asio::placeholders::bytes_transferred));
	}

	template <class T>
	std::string Socket<T>::read()
	{
		if (this->sock_state != SOCK_CONNECTED)
		{
			throw TCPSocketReadNotOpenException();
		}
		// TODO: implement sync read
		size_t size = 0;
		try
		{
			size = asio::read(*socket, asio::buffer(read_data_buffer, BUFFER_SIZE),
				asio::transfer_at_least(1));
		}
		catch(asio::system_error & e)
		{
			this->CompleteClose();
			this->on_error(e.what());
			throw TCPSocketReadException();
		}
		if (size > 0)
		{
			return std::string(read_data_buffer, size);
		}
		return std::string("");
	}

}
#endif
