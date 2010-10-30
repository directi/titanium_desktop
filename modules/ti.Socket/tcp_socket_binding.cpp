/**
 * @author: Mital Vora <mital.d.vora@gmail.com>
 */

#include "tcp_socket_binding.h"

#include <boost/bind.hpp>


namespace ti
{
	asio::io_service TCPSocketBinding::io_service;
	std::auto_ptr<asio::io_service::work> TCPSocketBinding::io_idlework(
		new asio::io_service::work(io_service));


	void TCPSocketBinding::Initialize()
	{
		asio::thread *io_thread = new asio::thread(
				boost::bind(&asio::io_service::run, &TCPSocketBinding::io_service));
	}

	void TCPSocketBinding::UnInitialize()
	{
	}


	TCPSocketBinding::TCPSocketBinding(Host* host, const std::string& hostname, const std::string& port) :
		StaticBoundObject("Network.TCPSocket"),
		ti_host(host),
		hostname(hostname),
		port(port),
		nonBlocking(false),
		useKeepAlives(true),
		//inactivetime(1),
		//resendtime(1),
		resolver(TCPSocketBinding::io_service),
		socket(TCPSocketBinding::io_service),
		sock_state(SOCK_CLOSED),
		onConnect(0),
		onRead(0),
		onError(0),
		onClose(0)
	{
		this->SetMethod("connect",&TCPSocketBinding::Connect);
		this->SetMethod("connectNB",&TCPSocketBinding::ConnectNB);

		this->SetMethod("read",&TCPSocketBinding::Read);
		this->SetMethod("write",&TCPSocketBinding::Write);

		this->SetMethod("isClosed",&TCPSocketBinding::IsClosed);
		this->SetMethod("close",&TCPSocketBinding::Close);

		this->SetMethod("onConnect",&TCPSocketBinding::SetOnConnect);
		this->SetMethod("onRead",&TCPSocketBinding::SetOnRead);
		this->SetMethod("onError",&TCPSocketBinding::SetOnError);
		this->SetMethod("onClose",&TCPSocketBinding::SetOnClose);

		// Enables/disables keepalives.
		this->SetMethod("setDisconnectionNotifications", &TCPSocketBinding::SetKeepAlives);

		// Sets the timeouts for the keepalive times, should be called before connecting the socket.
		this->SetMethod("setDisconnectionNotificationTime", &TCPSocketBinding::SetKeepAliveTimes);
	}

	TCPSocketBinding::~TCPSocketBinding()
	{
		this->CompleteClose();
	}

	void TCPSocketBinding::SetOnConnect(const ValueList& args, KValueRef result)
	{
		this->onConnect = args.at(0)->ToMethod();
	}

	void TCPSocketBinding::SetOnRead(const ValueList& args, KValueRef result)
	{
		this->onRead = args.at(0)->ToMethod();
	}

	void TCPSocketBinding::SetOnError(const ValueList& args, KValueRef result)
	{
		this->onError = args.at(0)->ToMethod();
	}

	void TCPSocketBinding::SetOnClose(const ValueList& args, KValueRef result)
	{
		this->onClose = args.at(0)->ToMethod();
	}

	void TCPSocketBinding::IsClosed(const ValueList& args, KValueRef result)
	{
		return result->SetBool(this->sock_state == SOCK_CLOSED);
	}

	void TCPSocketBinding::setKeepAlive(bool keepAlive)
	{
		asio::socket_base::keep_alive option(this->useKeepAlives);
		socket.set_option(option);
	}

	void TCPSocketBinding::SetKeepAlives(const ValueList& args, KValueRef result)
	{
		this->useKeepAlives = args.at(0)->ToBool();
		this->setKeepAlive(this->useKeepAlives);
	}

	void TCPSocketBinding::SetKeepAliveTimes(const ValueList& args, KValueRef result) 
	{
		// TODO:
		if(this->sock_state != SOCK_CLOSED) 
			throw ValueException::FromString("You can only set the keep-alive times before connecting the socket");

		if(args.size() > 1 && args.at(0)->IsInt() && args.at(1)->IsInt()) 
		{
			//this->inactivetime = args.at(0)->ToInt();
			//this->resendtime = args.at(1)->ToInt();
		}
		else 
		{
			throw ValueException::FromString("usage: setDisconnectionNotificationTime(int firstCheck, int subsequentChecks) -> If 9 subsequent checks fail the connection is considered lost");
		}
	}

	void TCPSocketBinding::Connect(const ValueList& args, KValueRef result)
	{
		static const std::string eprefix = "Connect exception: ";
		throw ValueException::FromString(eprefix + "connect not Implemented... use connectNB()");
		if(this->sock_state != SOCK_CLOSED)
		{
			throw ValueException::FromString(eprefix + "Socket is either connected or connecting");
		}
		nonBlocking = false;
		this->sock_state = SOCK_CONNECTING;
		long timeout = (args.size() > 0 && args.at(0)->IsInt()) ? args.at(0)->ToInt() : 10;

		//TODO: handle timeout for connect
		try
		{
			//tcp::resolver::iterator endpoint_iterator = ;
			//socket.connect(*endpoint_iterator);
		}
		catch(asio::system_error & e)
		{
			socket.close();
			this->OnError(e.what());
		}

	
		GetLogger()->Debug("Connecting Blocking.");
	}

	void TCPSocketBinding::registerHandleResolve()
	{
		tcp::resolver::query query(hostname, port);
		resolver.async_resolve(query,
			boost::bind(&TCPSocketBinding::handleResolve, this,
			asio::placeholders::error, asio::placeholders::iterator));
	}

	void TCPSocketBinding::handleResolve(const asio::error_code& error,
	  tcp::resolver::iterator endpoint_iterator)
	{
		if (error)
		{
			this->OnError(error.message());
			return;
		}
		registerHandleConnect(endpoint_iterator);
	}


	void TCPSocketBinding::registerHandleConnect(tcp::resolver::iterator endpoint_iterator)
	{
		if (endpoint_iterator != tcp::resolver::iterator())
		{
			socket.async_connect(*endpoint_iterator,
				boost::bind(&TCPSocketBinding::handleConnect, this,
				asio::placeholders::error, ++endpoint_iterator));
			return;
		}
		this->OnError("TCPSocket Host resolution Error");
	}

	void TCPSocketBinding::handleConnect(const asio::error_code& error,
	  tcp::resolver::iterator endpoint_iterator)
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

	void TCPSocketBinding::ConnectNB(const ValueList& args, KValueRef result)
	{
		if(this->sock_state != SOCK_CLOSED)
		{
			throw ValueException::FromString("Connect exception: Socket is either connected or connecting");
		}
		nonBlocking = true;
		GetLogger()->Debug("Connecting non Blocking.");
		this->sock_state = SOCK_CONNECTING;
		this->registerHandleResolve();
	}

	void TCPSocketBinding::registerHandleRead()
	{
		asio::async_read(socket,
			asio::buffer(read_data_buffer, BUFFER_SIZE),
			asio::transfer_at_least(1),
			boost::bind(&TCPSocketBinding::handleRead, this,
			asio::placeholders::error, asio::placeholders::bytes_transferred));
	}

	void TCPSocketBinding::handleRead(const asio::error_code& error, std::size_t bytes_transferred)
	{
		if (error)
		{
			this->OnError(error.message());
			return;
		}
		this->OnRead(read_data_buffer, bytes_transferred);
		this->registerHandleRead();
	}

	void TCPSocketBinding::OnError(const std::string& error_text) 
	{
		this->CompleteClose();
		this->on_error(error_text);
	}

	void TCPSocketBinding::OnConnect() 
	{
		this->sock_state = SOCK_CONNECTED;
		this->on_connect();
	}

	void TCPSocketBinding::OnRead(char * read_data_buffer, int size) 
	{
		read_data_buffer[size] = '\0';
		this->on_read(read_data_buffer, size);
	}

	void TCPSocketBinding::OnClose()
	{
		this->CompleteClose();
		this->on_close();
	}

	void TCPSocketBinding::registerHandleWrite()
	{
		asio::async_write(socket,
			asio::buffer(write_buffer.front().c_str(), write_buffer.front().size()),
			boost::bind(&TCPSocketBinding::handleWrite, this,
			asio::placeholders::error, asio::placeholders::bytes_transferred));

	}

	void TCPSocketBinding::handleWrite(const asio::error_code& error, std::size_t bytes_transferred)
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

	void TCPSocketBinding::writeAsync(const std::string &data)
	{
		asio::detail::mutex::scoped_lock lock(write_mutex);
		bool write_in_progress = !write_buffer.empty();
		write_buffer.push_back(data);
		if (!write_in_progress)
		{
			this->registerHandleWrite();
		}
	}

	void TCPSocketBinding::Write(const ValueList& args, KValueRef result)
	{
		std::string eprefix = "TCPSocketBinding::Write: ";
		if (this->sock_state != SOCK_CONNECTED)
		{
			throw ValueException::FromString(eprefix +  "Socket is not open for Write");
		}
		try
		{
			std::string data = args.at(0)->ToString();
			if(nonBlocking)
			{
				writeAsync(data);

			}
			else
			{
				// TODO:
				result->SetBool(true);
			}
			
		}
		catch(Poco::Exception &e)
		{
			throw ValueException::FromString(eprefix + e.displayText());
		}
	}

	void TCPSocketBinding::Read(const ValueList& args, KValueRef result)
	{
		std::string eprefix = "TCPSocketBinding::Read: ";
		if (this->sock_state != SOCK_CONNECTED)
		{
			throw ValueException::FromString(eprefix +  "Socket is not open for Read");
		}
		try
		{
			char data[BUFFER_SIZE + 1];
			int size = 0;
			BytesRef bytes(new Bytes(data, size));
			result->SetValue(Value::NewObject(bytes));
		}
		catch(Poco::Exception &e)
		{
			throw ValueException::FromString(eprefix + e.displayText());
		}
	}

	void TCPSocketBinding::Close(const ValueList& args, KValueRef result)
	{
		bool bResult = false;
		if (this->sock_state != SOCK_CLOSED)
		{
			GetLogger()->Debug("Closing socket to: %s:%d ", this->hostname.c_str(), this->port.c_str());
			this->CompleteClose();
			bResult = true;
		}
		result->SetBool(bResult);
	}

	void TCPSocketBinding::CompleteClose()
	{
		if(this->sock_state == SOCK_CONNECTED || this->sock_state == SOCK_CONNECTING) 
		{
			this->sock_state = SOCK_CLOSING;
			this->sock_state = SOCK_CLOSED;
		}
	}
}
