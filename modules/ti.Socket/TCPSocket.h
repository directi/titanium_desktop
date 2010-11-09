/**
* @author: Mital Vora <mital.d.vora@gmail.com>
*/

#ifndef _TCP_SOCKET_H_
#define _TCP_SOCKET_H_

#ifdef OS_WIN32
#ifndef WINVER
#define WINVER 0x0502
#endif

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0502
#endif

#ifndef _WIN32_WINDOWS
#define _WIN32_WINDOWS 0x0410
#endif

#ifndef _WIN32_IE
#define _WIN32_IE 0x600
#endif
#endif


#include "SocketExceptions.h"
#include "TCPSocketHandler.h"

#include <deque>

#include <asio.hpp>
#include <asio/detail/mutex.hpp>

using asio::ip::tcp;

#include <boost/bind.hpp>


#define BUFFER_SIZE 1024   // choose a reasonable size to send back to JS

class TCPSocket
{
public:
	TCPSocket(const std::string & hostname,
		const std::string& port,
		TCPSocketHandler * handler)
		: hostname(hostname),
		port(port),
		handler(handler),
		non_blocking(false),
		keep_alives(true),
		//inactivetime(1),
		//resendtime(1),
		resolver(*TCPSocket::io_service.get()),
		socket(*TCPSocket::io_service.get()),
		sock_state(SOCK_CLOSED)
	{
	}

	virtual ~TCPSocket();

	bool connect(long timeout = 10);
	void connectNB();
	bool write(const std::string &data);
	std::string read();
	bool isClosed();
	bool close()
	{
		// Log  ->Debug("Closing socket to: %s:%d ", this->hostname.c_str(), this->port.c_str());
		return this->CompleteClose();
	}

	void setKeepAlive(bool keep_alives);
	//void setKeepAliveTimes(int inactivetime, int resendtime) ;

	static void initialize();
	static void uninitialize();

private:
	static std::auto_ptr<asio::io_service> io_service;
	static std::auto_ptr<asio::io_service::work> io_idlework;
	static std::auto_ptr<asio::thread> TCPSocket::io_thread;

	const std::string hostname;
	const std::string port;
	TCPSocketHandler * handler;

	bool non_blocking;
	bool keep_alives;
	//int inactivetime;
	//int resendtime;
	tcp::resolver resolver;
	tcp::socket socket;

	char read_data_buffer[BUFFER_SIZE + 1];

	asio::detail::mutex write_mutex;
	std::deque<std::string> write_buffer;

	enum SOCK_STATE_en { SOCK_CLOSED, SOCK_CONNECTING, SOCK_CONNECTED, SOCK_CLOSING } sock_state;

	tcp::resolver::iterator resolveHost();
	bool tryConnect(tcp::resolver::iterator endpoint_iterator);
	bool writeSync(const std::string &data);

	void registerHandleResolve()
	{
		tcp::resolver::query query(hostname, port);
		resolver.async_resolve(query,
			boost::bind(&TCPSocket::handleResolve, this,
			asio::placeholders::error, asio::placeholders::iterator));
	}

	void handleResolve(const asio::error_code& error, tcp::resolver::iterator endpoint_iterator)
	{
		if (error)
		{
			this->OnError(error.message());
			return;
		}
		registerHandleConnect(endpoint_iterator);
	}

	void registerHandleConnect(tcp::resolver::iterator endpoint_iterator)
	{
		if (endpoint_iterator != tcp::resolver::iterator())
		{
			socket.async_connect(*endpoint_iterator,
				boost::bind(&TCPSocket::handleConnect, this,
				asio::placeholders::error, ++endpoint_iterator));
			return;
		}
		this->OnError("TCPSocket Host resolution Error");
	}

	void handleConnect(const asio::error_code& error, tcp::resolver::iterator endpoint_iterator)
	{
		if (!error)
		{
			this->OnConnect();
			this->registerHandleRead();
			return;
		}

		socket.close();
		if (endpoint_iterator != tcp::resolver::iterator())
		{
			this->registerHandleConnect(endpoint_iterator);
			return;
		}
		this->OnError(error.message());
	}

	void registerHandleRead();
	void handleRead(const asio::error_code& error, std::size_t bytes_transferred);

	void writeAsync(const std::string &data);
	void registerHandleWrite();
	void handleWrite(const asio::error_code& error, std::size_t bytes_transferred);

	bool CompleteClose()
	{
		if ((this->sock_state == SOCK_CONNECTED)
			|| (this->sock_state == SOCK_CONNECTING))
		{
			this->sock_state = SOCK_CLOSING;
			socket.close();
			this->sock_state = SOCK_CLOSED;
			return true;
		}
		return false;
	}

	void OnConnect()
	{
		this->sock_state = SOCK_CONNECTED;
		if (handler)
		{
			handler->on_connect();
		}
	}

	void OnRead(char * data, int size)
	{
		read_data_buffer[size] = '\0';
		if (handler)
		{
			handler->on_read(read_data_buffer, size);
		}
	}

	void OnClose()
	{
		this->CompleteClose();
		if (handler)
		{
			handler->on_close();
		}
	}

	void OnError(const std::string& error_text)
	{
		this->CompleteClose();
		if (handler)
		{
			handler->on_error(error_text);
		}
	}

};

#endif
