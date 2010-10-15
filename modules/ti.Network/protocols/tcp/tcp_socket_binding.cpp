/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "tcp_socket_binding.h"
#include <kroll/kroll.h>

#define BUFFER_SIZE 1024   // choose a reasonable size to send back to JS

namespace ti
{
	Poco::Net::SocketReactor TCPSocketBinding::reactor;
	Poco::Thread TCPSocketBinding::pollThread;

	TCPSocketBinding::TCPSocketBinding(Host* ti_host, const std::string& host, const int port) :
		StaticBoundObject("Network.TCPSocket"),
		ti_host(ti_host),
		host(host),
		port(port),
		socket(0),
		nonBlocking(false),
		sock_state(SOCK_CLOSED),
		onConnect(0),
		onRead(0),
		onWrite(0),
		onTimeout(0),
		onError(0),
		onClose(0)
	{
		socket = new StreamSocket();
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

	}
	TCPSocketBinding::~TCPSocketBinding()
	{
		this->CompleteClose();
		if (socket)
		{
			delete socket;
		}
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
		this->onWrite = args.at(0)->ToMethod();
	}
	void TCPSocketBinding::SetOnTimeout(const ValueList& args, KValueRef result)
	{
		this->onTimeout = args.at(0)->ToMethod();
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

	void TCPSocketBinding::Connect(const ValueList& args, KValueRef result)
	{
		nonBlocking = false;
		const std::string eprefix = "Connect exception: ";
	
		SocketAddress* a = this->beforeConnect();
		GetLogger()->Debug("Connecting Blocking.");
		try 
		{
			this->socket->connect(*a);
			delete a;
			this->OnConnect();
			result->SetBool(true);
		}
		catch(Poco::IOException &e)
		{
			this->sock_state = SOCK_CLOSED;
			delete a;
			throw ValueException::FromString(eprefix + e.displayText());
		}
		catch(std::exception &e)
		{
			this->sock_state = SOCK_CLOSED;
			delete a;
			throw ValueException::FromString(eprefix + e.what());
		}
		catch(...)
		{
			this->sock_state = SOCK_CLOSED;
			delete a;
			throw ValueException::FromString(eprefix + "Unknown exception");
		}
	}

	void TCPSocketBinding::ConnectNB(const ValueList& args, KValueRef result)
	{
		nonBlocking = true;
		const std::string eprefix = "Connect exception: ";

		SocketAddress* a = this->beforeConnect();
		GetLogger()->Debug("Connecting non Blocking.");
		try 
		{
			TCPSocketBinding::addSocket(this);
			this->socket->connectNB(*a);
			delete a;
			result->SetBool(true);
		}
		catch(Poco::IOException &e)
		{
			this->sock_state = SOCK_CLOSED;
			delete a;
			throw ValueException::FromString(eprefix + e.displayText());
		}
		catch(std::exception &e)
		{
			this->sock_state = SOCK_CLOSED;
			delete a;
			throw ValueException::FromString(eprefix + e.what());
		}
		catch(...)
		{
			this->sock_state = SOCK_CLOSED;
			delete a;
			throw ValueException::FromString(eprefix + "Unknown exception");
		}
	}

	SocketAddress* TCPSocketBinding::beforeConnect()
	{
		const std::string eprefix = "Connect exception: ";
		if( this->sock_state != SOCK_CLOSED )
		{
			throw ValueException::FromString(eprefix + "Socket is either connected or connecting");
		}
		SocketAddress *a = new SocketAddress(this->host.c_str(), this->port);
		this->sock_state = SOCK_CONNECTING;
		return a;
	}

	void TCPSocketBinding::OnReadReady(ReadableNotification * notification)
	{
		bool justConnected = false;
		GetLogger()->Debug("Ready for Read with %d bytes on %s", this->socket->available(), this->socket->peerAddress().toString().c_str());
		if(this->sock_state == SOCK_CONNECTING)
		{
			this->OnConnect();
			justConnected = true;
		}
		std::string error_text("Read failed: ");
		bool error = false;
		try
		{
			if(!justConnected && this->socket->available() == 0 )
			{
				this->OnClose();
				return;
			}

			// Always read bytes, so that the tubes get cleared.
			char data[BUFFER_SIZE + 1];
			int size = socket->receiveBytes(&data, BUFFER_SIZE);
			GetLogger()->Debug("Read %d bytes on %s", size, this->socket->peerAddress().toString().c_str());
			if(size > 0) 
			{
				this->OnRead(data, size);
			} 
			else
			{
				// Theoretically impossible.
				if(! justConnected) 
				{
					this->OnClose();
				}
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
		if(this->sock_state == SOCK_CONNECTING)
		{
			this->OnConnect();
		} 
		else 
		{		
			this->OnWrite();
		}
		TCPSocketBinding::removeWriteListener(this);
	}

	void TCPSocketBinding::OnError(ErrorNotification * notification)
	{
		GetLogger()->Debug("Socket Error on %s:%d ", this->host.c_str(), this->port);
		this->OnError(notification->name());
	}

	void TCPSocketBinding::OnError(const std::string& error_text) 
	{
		this->sock_state = SOCK_CLOSED;
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

	void TCPSocketBinding::OnWrite() 
	{
		if(!this->onWrite.isNull()) 
		{
			ValueList args;
			RunOnMainThread(this->onWrite, args, false);
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
			if(nonBlocking) 
			{
				TCPSocketBinding::addWriteListener(this);
			}
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
			this->CompleteClose();
			bResult = true;
		}
		result->SetBool(bResult);
	}

	void TCPSocketBinding::CompleteClose()
	{
		if(this->sock_state != SOCK_CLOSED)
		{
			GetLogger()->Debug("Closing socket to: %s:%d ", this->host.c_str(), this->port);
			TCPSocketBinding::removeSocket(this);
			this->socket->close();
			this->sock_state = SOCK_CLOSED;
		}
	}

	void TCPSocketBinding::addSocket(TCPSocketBinding* tsb)
	{
		if(! pollThread.isRunning()) 
		{
			reactor.setTimeout(Poco::Timespan(10, 0));
			pollThread.start(reactor);
		}
		Poco::Observer<TCPSocketBinding, WritableNotification> o2(*tsb, &TCPSocketBinding::OnWriteReady);
		reactor.addEventHandler(*(tsb->socket), o2);
		Poco::Observer<TCPSocketBinding, ReadableNotification> o1(*tsb, &TCPSocketBinding::OnReadReady);
		reactor.addEventHandler(*(tsb->socket), o1);
		Poco::Observer<TCPSocketBinding, ErrorNotification> o4(*tsb, &TCPSocketBinding::OnError);
		reactor.addEventHandler(*(tsb->socket), o4);
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

	void TCPSocketBinding::addWriteListener(TCPSocketBinding* tsb)
	{
		Poco::Observer<TCPSocketBinding, WritableNotification> o2(*tsb, &TCPSocketBinding::OnWriteReady);
		reactor.addEventHandler(*(tsb->socket), o2);
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
}

