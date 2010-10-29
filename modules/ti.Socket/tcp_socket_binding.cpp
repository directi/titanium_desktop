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
	std::auto_ptr<asio::thread> TCPSocketBinding::io_thread;


	void TCPSocketBinding::Initialize()
	{
		io_thread = new asio::thread(
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
		//useKeepAlives(true),
		//inactivetime(1),
		//resendtime(1),
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

	void TCPSocketBinding::SetKeepAlives(const ValueList& args, KValueRef result)
	{
		// TODO:
		bool val = args.at(0)->ToBool();
		//this->useKeepAlives = val;
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
	
		GetLogger()->Debug("Connecting Blocking.");
	}

	tcp::resolver::iterator TCPSocketBinding::resolveHost()
	{
		tcp::resolver resolver(TCPSocketBinding::io_service);
		tcp::resolver::query query(hostname, port);
		return resolver.resolve(query);
	}

	void TCPSocketBinding::registerHandleConnect(tcp::resolver::iterator endpoint_iterator)
	{
		socket.async_connect(*endpoint_iterator,
			boost::bind(&TCPSocketBinding::handleConnect, this,
			  asio::placeholders::error, ++endpoint_iterator));
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
		this->registerHandleConnect(this->resolveHost());
	}

	void TCPSocketBinding::registerHandleRead()
	{
		asio::async_read(socket,
			asio::buffer(data, BUFFER_SIZE),
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
		this->OnRead(data, bytes_transferred);
		this->registerHandleRead();
	}

	void TCPSocketBinding::OnError(const std::string& error_text) 
	{
		this->CompleteClose();
		if(!this->onError.isNull()) 
		{
			ValueList args (Value::NewString(error_text.c_str()));
			RunOnMainThread(this->onError, args, false);
		}
	}

	void TCPSocketBinding::OnConnect() 
	{
		this->sock_state = SOCK_CONNECTED;
		if(!this->onConnect.isNull()) 
		{
			ValueList args;
			RunOnMainThread(this->onConnect, args, false);
		}
	}

	void TCPSocketBinding::OnRead(char * data, int size) 
	{
		data[size] = '\0';
		if(!this->onRead.isNull()) 
		{
			BytesRef bytes(new Bytes(data, size));
			ValueList args (Value::NewObject(bytes));
			RunOnMainThread(this->onRead, args, false);
		}
		else
		{
			GetLogger()->Warn("TCPSocket::onRead: not subscribed by JavaScript: data Read: " + string(data));
		}
	}

	void TCPSocketBinding::OnClose()
	{
		this->CompleteClose();
		if(!this->onClose.isNull()) 
		{
			ValueList args;
			RunOnMainThread(this->onClose, args, false);
		}
	}

	void TCPSocketBinding::registerHandleWrite()
	{
		char * buf;
		asio::async_write(socket,
			asio::buffer(write_msgs.front().c_str(), write_msgs.front().size()),
			boost::bind(&TCPSocketBinding::handleWrite, this,
			buf, asio::placeholders::error, asio::placeholders::bytes_transferred));

	}

	void TCPSocketBinding::handleWrite(char * buf,
		const asio::error_code& error, std::size_t bytes_transferred)
	{
		if (error)
		{
			this->OnError(error.message());
			return;
		}
		delete [] buf;
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
				char * buf = new char(data.size());
				strcpy(buf, data.c_str());
				asio::async_write(socket,
					asio::buffer(buf, data.size()),
					boost::bind(&TCPSocketBinding::handleWrite, this,
					buf, asio::placeholders::error, asio::placeholders::bytes_transferred));
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
