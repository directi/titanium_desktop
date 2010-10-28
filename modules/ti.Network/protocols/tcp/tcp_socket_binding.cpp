/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "tcp_socket_binding.h"
#include <kroll/kroll.h>
#include <Poco/ThreadPool.h>
#include <Poco/Runnable.h>

#define BUFFER_SIZE 1024   // choose a reasonable size to send back to JS

namespace ti
{
	const Poco::Timespan selectTime(0, SELECT_TIME_MICRO);
	QuieterSocketReactor TCPSocketBinding::reactor(selectTime);
	Poco::Thread TCPSocketBinding::pollThread;

	TCPSocketBinding::TCPSocketBinding(Host* ti_host, const std::string& host, const int port) :
		StaticBoundObject("Network.TCPSocket"),
		ti_host(ti_host),
		host(host),
		port(port),
		socket(0),
		nonBlocking(false),
		useKeepAlives(true),
		inactivetime(10),
		sock_state(SOCK_CLOSED),
		onConnect(0),
		onRead(0),
		onError(0),
		onClose(0)
	{
		/**
		 * @tiapi(method=True,name=Network.TCPSocket.connect,since=0.2) Connects a Socket object to the host specified during creation. 
		 * @tiarg(for=Network.TCPSocket.connect,type=Integer,name=timeout) the time in seconds to wait before the connect timesout 
		 * @tiresult(for=Network.TCPSocket.connect,type=Boolean) true if the Socket object successfully connects, false if otherwise
		 */
		this->SetMethod("connect",&TCPSocketBinding::Connect);
		/**
		 * @tiapi(method=True,name=Network.TCPSocket.connect,since=0.9) Connects a Socket object to the host specified during creation. 
		 * @tiarg(for=Network.TCPSocket.connectNB,type=Integer,name=timeout) the time in seconds to wait before the connect timesout 
		 * @tiresult(for=Network.TCPSocket.connect,type=Boolean) true if the Socket object successfully connects, false if otherwise
		 */
		this->SetMethod("connectNB",&TCPSocketBinding::ConnectNB);

		/**
		 * @tiapi(method=True,name=Network.TCPSocket.close,since=0.2) Close the connection of a Socket object
		 * @tiresult(for=Network.TCPSocket.close,type=Boolean) true if the connection was successfully close, false if otherwise
		 */
		this->SetMethod("close",&TCPSocketBinding::Close);

		/**
		 * @tiapi(method=True,name=Network.TCPSocket.write,since=0.2) Writes data to a socket
		 * @tiarg(for=Network.TCPSocket.write,type=String,name=data) data to write
		 * @tiresult(for=Network.TCPSocket.write,type=Boolean) true if the data was successfully written to the socket, false if otherwise
		 */
		this->SetMethod("write",&TCPSocketBinding::Write);

		this->SetMethod("read",&TCPSocketBinding::Read);

		/**
		 * @tiapi(method=True,name=Network.TCPSocket.isClosed,since=0.2) Checks whether a Socket object is closed
		 * @tiresult(for=Network.TCPSocket.isClosed,type=Boolean) true if a Socket object is closed, false if otherwise
		 */
		this->SetMethod("isClosed",&TCPSocketBinding::IsClosed);

		/**
		 * @tiapi(method=True,name=Network.TCPSocket.onConnect,since=0.9) Sets a callback function that will be fired when socket is connected in Non-Blocking mode.
		 * @tiarg(for=Network.TCPSocket.onConnect,type=Function,name=callback) callback function to be fired when the socket is connected in non-blocking mode and ready for read/write.
		 */
		this->SetMethod("onConnect",&TCPSocketBinding::SetOnConnect);

		// event handler callbacks
		/**
		 * @tiapi(method=True,name=Network.TCPSocket.onRead,since=0.2) Sets a callback function that will be fired when data is received from a socket
		 * @tiarg(for=Network.TCPSocket.onRead,type=Function,name=callback) callback function to be fired when data is received from a socket connection
		 */
		this->SetMethod("onRead",&TCPSocketBinding::SetOnRead);

		/**
		 * @tiapi(method=True,name=Network.TCPSocket.onWrite,since=0.2) Sets a callback function that will be fired when data is written to the socket
		 * @tiarg(for=Network.TCPSocket.onWrite,type=Function,name=callback) callback function to be fired when data is written to the socket
		 */
		this->SetMethod("onWrite",&TCPSocketBinding::SetOnWrite);
		/**
		 * @tiapi(method=True,name=Network.TCPSocket.onTimeout,since=0.2) Sets the callback function that will be fired when a socket times-out
		 * @tiarg(for=Network.TCPSocket.onTimeout,type=Function,name=callback) callback function to be fired when a socket times-out
		 */
		this->SetMethod("onTimeout",&TCPSocketBinding::SetOnTimeout);
		/**
		 * @tiapi(method=True,name=Network.TCPSocket.onError,version=0.5) Sets the callback function that will be fired when a socket throws an error
		 * @tiarg(for=Network.TCPSocket.onError,type=method,name=callback) callback function to be fired when a socket throws an error
		 */
		this->SetMethod("onError",&TCPSocketBinding::SetOnError);

		/**
		 * @tiapi(method=True,name=Network.TCPSocket.onReadComplete,since=0.2) Sets the callback function that will be fired when no more data is available
		 * @tiarg(for=Network.TCPSocket.onReadComplete,type=Function,name=callback) callback function be fired when no more data is available
		 */
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
	void TCPSocketBinding::SetOnWrite(const ValueList& args, KValueRef result)
	{
		GetLogger()->Warn("OnWrite events are never fired, event handler ignored");
	}
	void TCPSocketBinding::SetOnTimeout(const ValueList& args, KValueRef result)
	{
		GetLogger()->Warn("OnTimeout events are never fired, event handler ignored");
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
		bool val = args.at(0)->ToBool();
		this->useKeepAlives = val;
		if(this->socket) 
			this->socket->setKeepAlive(val);
	}

	void TCPSocketBinding::SetKeepAliveTimes(const ValueList& args, KValueRef result) 
	{
		if(this->sock_state != SOCK_CLOSED) 
			throw ValueException::FromString("You can only set the keep-alive times before connecting the socket");
		
		if(args.size() == 1 && args.at(0)->IsInt()) 
		{
			this->inactivetime = args.at(0)->ToInt();
			if(this->inactivetime < 1) 
			{
				throw ValueException::FromString("Please set an inactive time >= 2 seconds (1 may work)");
			}
		}
		else 
		{
			throw ValueException::FromString("usage: setDisconnectionNotificationTime(int inactiveTimeInSeconds)");
		}
	}

	void TCPSocketBinding::Connect(const ValueList& args, KValueRef result)
	{
		static const std::string eprefix = "Connect exception: ";
		if(this->sock_state != SOCK_CLOSED)
		{
			throw ValueException::FromString(eprefix + "Socket is either connected or connecting");
		}
		nonBlocking = false;
		long timeout = (args.size() > 0 && args.at(0)->IsInt()) ? args.at(0)->ToInt() : 10;
		Poco::Timespan t(timeout, 0);
	
		GetLogger()->Debug("Connecting Blocking.");

		this->socket = new StreamSocket();
		try 
		{
			SocketAddress a(this->host.c_str(), this->port);
			this->sock_state = SOCK_CONNECTING;
			this->socket->connect(a, t);
			this->sock_state = SOCK_CONNECTED;
			result->SetBool(true);
		}
		catch(Poco::IOException &e)
		{
			delete this->socket;
			this->sock_state = SOCK_CLOSED;
			throw ValueException::FromString(eprefix + e.displayText());
		}
		catch(std::exception &e)
		{
			delete this->socket;
			this->sock_state = SOCK_CLOSED;
			throw ValueException::FromString(eprefix + e.what());
		}
		catch(...)
		{
			delete this->socket;
			this->sock_state = SOCK_CLOSED;
			throw ValueException::FromString(eprefix + "Unknown exception");
		}
	}

	class AsyncDNSResolutionTask : public Poco::Runnable 
	{
	public:
		AsyncDNSResolutionTask(TCPSocketBinding* instance, std::string host, int port) 
			: instance(instance),
			host(host),
			port(port)
		{
		}

		virtual void run()
		{
			static const std::string error = "DNS Resolution failed";
			try 
			{
				Poco::Net::SocketAddress* a = new Poco::Net::SocketAddress(this->host.c_str(), this->port);
				instance->OnResolve(a);
			} 
			catch(...) 
			{
				instance->OnError(error);
			}
			// I think this is really bad practice, but it'll work here. :( 
			delete this;
		}
	private:
		TCPSocketBinding* instance;
		std::string host;
		int port;
	};

	void TCPSocketBinding::OnResolve(SocketAddress* a) 
	{
		static const std::string eprefix = "Connect exception: ";
		if(useKeepAlives)
			this->socket = new DisconnectAwareSocket(inactivetime);
		else
			this->socket = new StreamSocket();
		try 
		{
			TCPSocketBinding::addSocket(this);
			this->socket->connectNB(*a);
			delete a;
		}
		catch(Poco::IOException &e)
		{
			delete a;
			this->OnError(eprefix + e.displayText());
		}
		catch(std::exception &e)
		{
			delete a;
			this->OnError(eprefix + e.what());
		}
		catch(...)
		{
			delete a;
			this->OnError(eprefix + "Unknown exception");
		}
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
		AsyncDNSResolutionTask* t = new AsyncDNSResolutionTask(this, host, port);
		try 
		{
			Poco::ThreadPool::defaultPool().start(*t);
		}
		catch(Poco::NoThreadAvailableException &e)
		{
			GetLogger()->Warn("You're doing too many name resolutions at the same time... forcing sync resolution");
			t->run();
		}
		result->SetBool(true);
	}

	void TCPSocketBinding::OnReadReady(ReadableNotification * notification)
	{
		int available_bytes = this->socket->available();
		GetLogger()->Debug("Ready for Read with %d bytes on %s", available_bytes, this->socket->peerAddress().toString().c_str());
		bool justConnected = false;
		if(this->sock_state == SOCK_CONNECTING)
		{
			this->OnConnect();
			justConnected = true;
		} 
		std::string error_text("Read failed: ");
		bool error = false;
		try
		{
			if(available_bytes > 0) 
			{
				// Always read bytes, so that the tubes get cleared.
				int size;
				do 
				{
					char data[BUFFER_SIZE + 1];
					size = socket->receiveBytes(&data, BUFFER_SIZE);
					GetLogger()->Debug("Read %d bytes on %s", size, this->socket->peerAddress().toString().c_str());
					// A non-blocking socket on Linux can return 0 on read and set errno to E_WOULDBLOCK
					// indicating that the TCP data failed the checksum and would require retransmission.
					// But the Poco Doccumentation says it return of 0 means graceful shutdown...
					// Poco source seems to standardize and block if this is the case.
					if(size > 0) 
						this->OnRead(data, size);
					else 
					{
						if(!justConnected && size == 0) 
						{
							GetLogger()->Debug("Graceful shutdown detected");
							this->OnClose(); 
						} 
						if(size < 0) 
						{
							GetLogger()->Debug("Connection refused Linux style");
							// Linux connection errors should come this way.
							this->OnError("Connection Refused");
						}
					}
				} while(size == BUFFER_SIZE);
			} 
			else
			{
				// Windows connection errors should come this way.
				GetLogger()->Debug("Connection closed a-la windows");
				this->OnClose();
			}
		}
		catch(Poco::Exception &e)
		{
			error_text += e.displayText().c_str();
			error = true;
		}
		catch(...)
		{
			error_text += "Unknown Exception";
			error = true;
		}
		if( error )
		{
			GetLogger()->Error(error_text);
			this->OnError(error_text);
		}
	}

	void TCPSocketBinding::OnWriteReady(WritableNotification * notification)
	{
		GetLogger()->Debug("Ready for Write on Socket: "  + this->socket->peerAddress().toString());
		TCPSocketBinding::removeWriteListener(this);
		if(this->sock_state == SOCK_CONNECTING)
		{
			this->OnConnect();
		} 
	}

	void TCPSocketBinding::OnError(ErrorNotification * notification)
	{
		GetLogger()->Debug("Socket Error on %s:%d ", this->host.c_str(), this->port);
		// This isn't really helpful, but I don't this it would be called ever!
		this->OnError(notification->name());
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
		if(!this->onRead.isNull()) 
		{
			data[size] = '\0';
			BytesRef bytes(new Bytes(data, size));
			ValueList args (Value::NewObject(bytes));
			RunOnMainThread(this->onRead, args, false);
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

	void TCPSocketBinding::Write(const ValueList& args, KValueRef result)
	{
		std::string eprefix = "TCPSocketBinding::Write: ";
		if (this->sock_state != SOCK_CONNECTED)
		{
			throw ValueException::FromString(eprefix +  "Socket is not open for Write");
		}
		try
		{
			std::string buffer = args.at(0)->ToString();
			this->socket->sendBytes(buffer.c_str(), buffer.length());
			result->SetBool(true);
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
			int size = socket->receiveBytes(&data, BUFFER_SIZE);
			data[size] = '\0';
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
			GetLogger()->Debug("Closing socket to: %s:%d ", this->host.c_str(), this->port);
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
			TCPSocketBinding::removeSocket(this);
			if(this->socket)
			{
				this->socket->close();
				delete this->socket;
			}
			this->sock_state = SOCK_CLOSED;
		}
	}

	void TCPSocketBinding::addSocket(TCPSocketBinding* tsb)
	{
		if(! pollThread.isRunning()) 
		{
			pollThread.setName(std::string("Reactor Notification Thread"));
			reactor.setTimeout(Poco::Timespan(10, 0));
			pollThread.start(reactor);
		}
		Poco::Observer<TCPSocketBinding, WritableNotification> o2(*tsb, &TCPSocketBinding::OnWriteReady);
		reactor.addEventHandler(*(tsb->socket), o2);
		Poco::Observer<TCPSocketBinding, ReadableNotification> o1(*tsb, &TCPSocketBinding::OnReadReady);
		reactor.addEventHandler(*(tsb->socket), o1);
		Poco::Observer<TCPSocketBinding, ErrorNotification> o4(*tsb, &TCPSocketBinding::OnError);
		reactor.addEventHandler(*(tsb->socket), o4);
		reactor.wakeup();
	}

	void TCPSocketBinding::removeSocket(TCPSocketBinding* tsb)
	{
		Poco::Observer<TCPSocketBinding, ReadableNotification> o1(*tsb, &TCPSocketBinding::OnReadReady);
		reactor.removeEventHandler(*(tsb->socket), o1);
		Poco::Observer<TCPSocketBinding, WritableNotification> o2(*tsb, &TCPSocketBinding::OnWriteReady);
		reactor.removeEventHandler(*(tsb->socket), o2);
		Poco::Observer<TCPSocketBinding, ErrorNotification> o4(*tsb, &TCPSocketBinding::OnError);
		reactor.removeEventHandler(*(tsb->socket), o4);
	}

	void TCPSocketBinding::removeWriteListener(TCPSocketBinding* tsb)
	{
		Poco::Observer<TCPSocketBinding, WritableNotification> o2(*tsb, &TCPSocketBinding::OnWriteReady);
		reactor.removeEventHandler(*(tsb->socket), o2);
	}

	//CARL: TODO - Actually call this method somewhere...
	void TCPSocketBinding::shutdown() 
	{
		reactor.stop();
		pollThread.join();
	}

	QuieterSocketReactor::QuieterSocketReactor(const Poco::Timespan& t) 
		: SocketReactor(t),
		waiting(false)
	{
	}

	void QuieterSocketReactor::wakeup() 
	{
		if(waiting) idle.broadcast();
	}

	void QuieterSocketReactor::onIdle() 
	{
		Poco::FastMutex::ScopedLock l(conditionLock);
		waiting = true;
		idle.wait(conditionLock);
		waiting = false;
	}

// Headers for the keepalive implementations...
#if OS_WIN32
#include <MSTcpIp.h>
#include <windows.h>
#elif OS_LINUX
#elif OS_OSX
#endif

	DisconnectAwareSocket::DisconnectAwareSocket(int inactivitytime) 
		: StreamSocket(IPAddress::IPv4)
	{
		std::string errtxt = "Error setting socket keepalive options, we probably wouldn't detect disconnects: ";
		// Makes sure we detect unplanned disconnects within a fixed time.
		try 
		{
			this->setKeepAlive(true);
#if OS_WIN32
			// Windows XP seems to disconnect anyway after about 10 seconds, but here goes...
			// Stuff should be defined in MSTcpIP.h which we should get... 
			// FormatMessage needs special treatment.
			DWORD dwBytes;
			tcp_keepalive settings, sReturned;
			settings.onoff = 1;
			inactivitytime *= 500; 
			settings.keepalivetime = inactivitytime; 
			settings.keepaliveinterval = inactivitytime / 9;

			if (WSAIoctl(sockfd(), SIO_KEEPALIVE_VALS, &settings, sizeof(settings), 
							&sReturned, sizeof(sReturned), &dwBytes,
							NULL, NULL) != 0)
			{
				std::string rv;
				LPVOID lpMsgBuf;

				if (FormatMessageA(
						FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
						NULL,
						WSAGetLastError(),
						MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
						(LPSTR) &lpMsgBuf,
						0,
						NULL ))
				{
					rv.assign(reinterpret_cast<const char*>(lpMsgBuf));
				}
				else
				{
					rv.assign("FormatMessage API failed");
				}
				LocalFree(lpMsgBuf);
				GetLogger()->Error(errtxt + rv);
			}
#elif OS_LINUX 
#define check_error(a, s) if(a < 0) throw Poco::Exception(s + " failed: " + strerror(errno));
			// The right headers should get magically included.
			inactivitytime /= 2;
			optval = 1;
		    check_error(setsockopt(sockfd(), SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval)), "setsockopt keepalive"));
		    optval = inactivitytime; 
		    check_error(setsockopt(sockfd(), SOL_TCP, TCP_KEEPIDLE, &optval, sizeof(optval)), "setsockopt keepidle");
		    optval = 1; 
		    check_error(setsockopt(sockfd(), SOL_TCP, TCP_KEEPINTVL, &optval, sizeof(optval)), "setsockopt keepintvl");
		    optval = inactivitytime;
		    check_error(setsockopt(sockfd(), SOL_TCP, TCP_KEEPCNT, &optval, sizeof(optval)), check_error("setsockopt keepcnt");
#elif OS_OSX
			GetLogger()->Error(errtxt + "Sorry, Keepalive settings don't seem to work on MacOS");
#endif
		} 
		catch(Poco::Exception &e)
		{
			GetLogger()->Error(errtxt + e.displayText());
		} 
		catch(std::exception &e) 
		{
			GetLogger()->Error(errtxt + e.what());
		}
		catch(...)
		{
			GetLogger()->Error(errtxt + "Unknown error");
		}
	}
}
