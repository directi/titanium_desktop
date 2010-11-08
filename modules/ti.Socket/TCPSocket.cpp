/**
* @author: Mital Vora <mital.d.vora@gmail.com>
*/

#include "TCPSocket.h"


std::auto_ptr<asio::io_service> TCPSocket::io_service(new asio::io_service());
std::auto_ptr<asio::io_service::work> TCPSocket::io_idlework(
	new asio::io_service::work(*io_service));
std::auto_ptr<asio::thread> TCPSocket::io_thread(NULL);

void TCPSocket::initialize()
{
	io_thread.reset(new asio::thread(
		boost::bind(&asio::io_service::run, TCPSocket::io_service.get())));
}

void TCPSocket::uninitialize()
{
	io_service->stop();
	io_thread->join();
	io_idlework.reset();
	io_thread.reset();
	io_service.reset();
}


TCPSocket::~TCPSocket()
{
	this->CompleteClose();
}

bool TCPSocket::isClosed()
{
	return (this->sock_state == SOCK_CLOSED);
}

void TCPSocket::setKeepAlive(bool keep_alives)
{
	this->keep_alives = keep_alives;
	asio::socket_base::keep_alive option(this->keep_alives);
	socket.set_option(option);
}

//void TCPSocket::setKeepAliveTimes(int inactivetime, int resendtime) 
//{
//	if(this->sock_state != SOCK_CLOSED) 
//	{
//		throw TCPSocketConnectedException();
//	}
//	//this->inactivetime = inactivetime;
//	//this->resendtime = resendtime;
//}

tcp::resolver::iterator TCPSocket::resolveHost()
{
	tcp::resolver::query query(hostname, port);
	return resolver.resolve(query);
}

bool TCPSocket::tryConnect(tcp::resolver::iterator endpoint_iterator)
{
	try
	{
		socket.connect(*endpoint_iterator);
	}
	catch(asio::system_error & e)
	{
		socket.close();
		return false;
	}
	return true;
}

bool TCPSocket::connect(long timeout)
{
	if(this->sock_state != SOCK_CLOSED)
	{
		throw TCPSocketConnectedException();
	}
	non_blocking = false;
	this->sock_state = SOCK_CONNECTING;

	//TODO: implement timeout for connect
	tcp::resolver::iterator endpoint_iterator;
	try
	{
		endpoint_iterator = this->resolveHost();
	}
	catch(asio::system_error & e)
	{
		this->OnError(e.what());
		this->sock_state = SOCK_CLOSED;
		return false;
	}

	bool ret;
	while(endpoint_iterator != tcp::resolver::iterator())
	{
		ret = tryConnect(endpoint_iterator);
		if (ret)
			break;
		endpoint_iterator = ++endpoint_iterator;
	}
	this->sock_state = (ret)?SOCK_CONNECTED:SOCK_CLOSED;
	return ret;
}


void TCPSocket::connectNB()
{
	if(this->sock_state != SOCK_CLOSED)
	{
		throw TCPSocketConnectedException();
	}
	non_blocking = true;
	this->sock_state = SOCK_CONNECTING;
	this->registerHandleResolve();
}

void TCPSocket::registerHandleRead()
{
	asio::async_read(socket,
		asio::buffer(read_data_buffer, BUFFER_SIZE),
		asio::transfer_at_least(1),
		boost::bind(&TCPSocket::handleRead, this,
		asio::placeholders::error, asio::placeholders::bytes_transferred));
}

void TCPSocket::handleRead(const asio::error_code& error, std::size_t bytes_transferred)
{
	if (error)
	{
		this->OnError(error.message());
		return;
	}
	this->OnRead(read_data_buffer, bytes_transferred);
	this->registerHandleRead();
}


void TCPSocket::registerHandleWrite()
{
	asio::async_write(socket,
		asio::buffer(write_buffer.front().c_str(), write_buffer.front().size()),
		boost::bind(&TCPSocket::handleWrite, this,
		asio::placeholders::error, asio::placeholders::bytes_transferred));

}

void TCPSocket::handleWrite(const asio::error_code& error, std::size_t bytes_transferred)
{
	if (error)
	{
		this->OnError(error.message());
		return;
	}
	asio::detail::mutex::scoped_lock lock(write_mutex);
	write_buffer.pop_front();
	if (!write_buffer.empty())
	{
		this->registerHandleWrite();
	}
}

void TCPSocket::writeAsync(const std::string &data)
{
	asio::detail::mutex::scoped_lock lock(write_mutex);
	bool write_in_progress = !write_buffer.empty();
	write_buffer.push_back(data);
	if (!write_in_progress)
	{
		this->registerHandleWrite();
	}
}

bool TCPSocket::writeSync(const std::string &data)
{
	try
	{
		asio::write(socket, asio::buffer(data.c_str(), data.size()));
	}
	catch(asio::system_error & e)
	{
		this->sock_state = SOCK_CLOSED;
		socket.close();
		this->OnError(e.what());
		return false;
	}
	return true;
}

bool TCPSocket::write(const std::string &data)
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

std::string TCPSocket::read()
{
	if (this->sock_state != SOCK_CONNECTED)
	{
		throw TCPSocketReadNotOpenException();
	}
	// TODO: implement sync read
	size_t size = 0;
	try
	{
		size = asio::read(socket, asio::buffer(read_data_buffer, BUFFER_SIZE),
			asio::transfer_at_least(1));
	}
	catch(asio::system_error & e)
	{
		this->CompleteClose();
		this->OnError(e.what());
		throw TCPSocketReadException();
	}
	if (size > 0)
	{
		return std::string(read_data_buffer, size);
	}
	return std::string("");
}

