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

	TCPSocketBinding::TCPSocketBinding(Host* ti_host, const std::string& host, const int port) :
		StaticBoundObject("Network.TCPSocket"),
		ti_host(ti_host),
		host(host),
		port(port),
		socket(),
		semWaitForConnect(0,1),
		sock_state(SOCK_CLOSED),
		error_state(ERROR_OFF),
		read_state(READ_CLOSED),
		write_state(WRITE_CLOSED),
		notifier(100),
		onConnect(0),
		onRead(0),
		onWrite(0),
		onTimeout(0),
		onError(0),
		onReadComplete(0)
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
		 * @tiapi(method=True,name=Network.TCPSocket.close,since=0.9) Clears the previous reported error and listens for any new errors
		 * @tiresult(for=Network.TCPSocket.close,type=Boolean) void
		 */
		this->SetMethod("clearError",&TCPSocketBinding::ClearError);
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

		// event handler callbacks
		/**
		 * @tiapi(method=True,name=Network.TCPSocket.onRead,since=0.2) Sets a callback function that will be fired when data is received from a socket
		 * @tiarg(for=Network.TCPSocket.onRead,type=Function,name=callback) callback function to be fired when data is received from a socket connection
		 */
		this->SetMethod("onRead",&TCPSocketBinding::SetOnRead);

		// event handler callbacks
		/**
		 * @tiapi(method=True,name=Network.TCPSocket.onConnect,since=0.9) Sets a callback function that will be fired when socket is connected in Non-Blocking mode.
		 * @tiarg(for=Network.TCPSocket.onConnect,type=Function,name=callback) callback function to be fired when the socket is connected in non-blocking mode and ready for read/write.
		 */
		this->SetMethod("onConnect",&TCPSocketBinding::SetOnConnect);

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
		this->SetMethod("onReadComplete",&TCPSocketBinding::SetOnReadComplete);

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
	void TCPSocketBinding::SetOnReadComplete(const ValueList& args, KValueRef result)
	{
		this->onReadComplete = args.at(0)->ToMethod();
	}
	void TCPSocketBinding::IsClosed(const ValueList& args, KValueRef result)
	{
		return result->SetBool(this->sock_state == SOCK_CLOSED);
	}
	void TCPSocketBinding::Connect(const ValueList& args, KValueRef result)
	{
		const bool nonBlocking = false;
		int timeout = (args.size() > 0)?args.at(0)->ToInt():10;
	
		this->_connect(timeout, nonBlocking);
		result->SetBool(this->sock_state == SOCK_CONNECTED);
	}
	void TCPSocketBinding::ConnectNB(const ValueList& args, KValueRef result)
	{
		const bool nonBlocking = true;
		int timeout = (args.size() > 0)?args.at(0)->ToInt():10;

		this->_connect(timeout, nonBlocking);
		result->SetBool(true);
	}

	bool TCPSocketBinding::_connect(int connectTimeout, bool nonBlocking)
	{
		bool result = false;
		const std::string eprefix = "Connect exception: ";
		if( this->sock_state != SOCK_CLOSED )
		{
			throw ValueException::FromString(eprefix + "Socket is either connected or connecting");
		}
		try
		{
			SocketAddress a(this->host.c_str(), this->port);
			this->sock_state = SOCK_CONNECTING;
			this->socket.connectNB(a);
			this->InitReactor(connectTimeout);
			if(nonBlocking)
			{
				GetLogger()->Debug("Connecting non Blocking.");
				result = true;
			}
			else
			{
				GetLogger()->Debug("Connecting Blocking.");
				this->blockForConnectionOrTimeout(connectTimeout);
			}
			///Be explicit here
			// Address Resolution Failure
			// Network Failure
		}
		catch(Poco::IOException &e)
		{
			throw ValueException::FromString(eprefix + e.displayText());
		}
		catch(std::exception &e)
		{
			throw ValueException::FromString(eprefix + e.what());
		}
		catch(...)
		{
			throw ValueException::FromString(eprefix + "Unknown exception");
		}
		return result;
	}

	void TCPSocketBinding::ClearError(const ValueList& args, KValueRef result)
	{
		this->error_state = ERROR_OFF;
		this->SetReactorDescriptors();
	}

	void TCPSocketBinding::OnReadReady(IONotification * notification)
	{
		GetLogger()->Debug("Ready for Read with %d bytes on %s", this->socket.available(), this->socket.peerAddress().toString().c_str());
		if(this->sock_state == SOCK_CONNECTING)
		{
			this->OnNonBlockingConnect();
		}
		std::string error_text("Read failed: ");
		bool error = false;
		try
		{
			if(this->socket.available() == 0 )
			{
				this->read_state = READ_CLOSED;
				this->SetReactorDescriptors();
				if( !this->onReadComplete.isNull() )
				{
					ValueList args;
					ti_host->RunOnMainThread(this->onReadComplete, args, false);
				}
				return;
			}
			// Always read bytes, so that the tubes get cleared.
			char data[BUFFER_SIZE + 1];
			int size = socket.receiveBytes(&data, BUFFER_SIZE);
			GetLogger()->Debug("Read %d bytes on %s", size, this->socket.peerAddress().toString().c_str());

			if (!this->onRead.isNull() && size > 0)
			{
				data[size] = '\0';
				ValueList args;
				args.push_back(Value::NewString(data));
				ti_host->RunOnMainThread(this->onRead, args, false);
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
			this->read_state = READ_CLOSED;
			this->SetReactorDescriptors();
			InvokeErrorHandler(error_text);
		}
	}

	void TCPSocketBinding::OnWriteReady(IONotification * notification)
	{
		GetLogger()->Debug("Ready for Write on Socket: "  + this->socket.peerAddress().toString());
		if(this->sock_state == SOCK_CONNECTING)
		{
			this->OnNonBlockingConnect();
		}		
		int count = 0;
		std::string error_text("Write failed: ");
		bool error = false;

		try
		{
			Poco::Mutex::ScopedLock lock(bufferMutex);
			if (!buffer.empty())
			{
				count = this->socket.sendBytes(buffer.c_str(), buffer.length());
				//GetLogger()->Debug("TCPSocketBinding::OnWriteReady: SendBytes Complete: "  + buffer.substr(0,count));
				GetLogger()->Debug("Written to socket %d bytes", count);
				buffer = buffer.substr(count);
			}
			if(buffer.empty())
			{
				this->write_state = WRITE_OPEN;
				this->SetReactorDescriptors();
			}
		}
		catch(Poco::Exception &e)
		{
			error_text += e.displayText().c_str();
			error = true;
		}
		catch (...)
		{
			error_text += "Unknown Exception";
			error = true;
		}
		if (error)
		{
			GetLogger()->Error(error_text);
			this->write_state = WRITE_CLOSED;
			this->SetReactorDescriptors();
			InvokeErrorHandler(error_text);
		}

		if (count >0 && !this->onWrite.isNull())
		{
			ValueList args;
			args.push_back(Value::NewInt(count));
			ti_host->RunOnMainThread(this->onWrite, args, false);
		}
	}
	void TCPSocketBinding::OnTimeout(IONotification * notification)
	{
		if (!this->onTimeout.isNull())
		{
			ValueList args;
			ti_host->RunOnMainThread(this->onTimeout, args, false);
		}
	}
	void TCPSocketBinding::OnError(IONotification * notification)
	{
		GetLogger()->Debug("Socket Error on %s:%d ", this->host.c_str(), this->port);
		if( this->sock_state == SOCK_CONNECTING )
		{
			this->OnNonBlockingConnectFailure();
		}
		this->error_state = ERROR_ON;
		this->SetReactorDescriptors();
		InvokeErrorHandler("Socket Error !!");
	}
	void TCPSocketBinding::Write(const ValueList& args, KValueRef result)
	{
		std::string eprefix = "TCPSocketBinding::Write: ";
		if (this->write_state == WRITE_CLOSED && this->sock_state != SOCK_CONNECTING)
		{
			throw ValueException::FromString(eprefix +  "Socket is not open for Write");
		}
		try
		{
			Poco::Mutex::ScopedLock lock(bufferMutex);
			buffer += args.at(0)->ToString();
			result->SetBool(true);
			this->write_state = WRITE_WAITING;
			this->error_state = ERROR_OFF;
			this->SetReactorDescriptors();
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

	void TCPSocketBinding::OnNonBlockingConnect()
	{
		this->socket.setKeepAlive(true);
		this->sock_state = SOCK_CONNECTED;
		this->read_state = READ_OPEN;
		this->write_state = WRITE_OPEN;
		this->SetReactorDescriptors();

		GetLogger()->Debug("Socket Connected to %s:%d ", this->host.c_str(), this->port);
		this->unBlockOnConnectionOrTimeout();

		if(!this->onConnect.isNull())
		{
			ValueList args;
			args.push_back(Value::NewBool(true));
			ti_host->RunOnMainThread(this->onConnect, args, false);
		}
	}

	void TCPSocketBinding::OnNonBlockingConnectFailure()
	{
		GetLogger()->Debug("Socket Connect Failure for %s:%d ", this->host.c_str(), this->port);
		this->sock_state=SOCK_CLOSED;
		this->SetReactorDescriptors();
		this->unBlockOnConnectionOrTimeout();
	}

	void TCPSocketBinding::blockForConnectionOrTimeout(int secs)
	{
		try
		{
			this->semWaitForConnect.wait(secs * 1000);			
			GetLogger()->Debug("Unblocked from connect to: %s:%d ", this->host.c_str(), this->port);
		}
		catch (Poco::TimeoutException &e)
		{
			GetLogger()->Debug("Timeout in a blocking connect to: %s:%d ", this->host.c_str(), this->port);
		}
	}
	
	void TCPSocketBinding::unBlockOnConnectionOrTimeout()
	{
		this->semWaitForConnect.set();
	}

	void TCPSocketBinding::CompleteClose()
	{
		bool socketNeedsToBeClosed = (this->sock_state != SOCK_CLOSED)?true:false;

		this->sock_state = SOCK_CLOSED;
		this->read_state = READ_CLOSED;
		this->write_state = WRITE_CLOSED;
		this->SetReactorDescriptors();

		if(socketNeedsToBeClosed)
		{
			GetLogger()->Debug("Closing socket to: %s:%d ", this->host.c_str(), this->port);
			//Ensure Reactor Descriptors are updated before the socket is closed
			//Once the socket is closed, its file descriptor will no longer be available.
			this->socket.close();
		}
		this->notifier.stop();
	}

	void TCPSocketBinding::InitReactor(int connectTimeout)
	{
		notifier.start();
		RegisterForRead();
		RegisterForWrite();
		RegisterForTimeout();
		RegisterForError();
	}

	void TCPSocketBinding::SetReactorDescriptors()
	{
		if( this->sock_state == SOCK_CLOSED ){
			this->ClearReactor();
			return;
		}
		if( this->sock_state == SOCK_CONNECTING){
			this->RegisterForRead();
			this->RegisterForWrite();
			this->RegisterForTimeout();
		}else{
			UnregisterForTimeout();
		}
		if( this->read_state == READ_CLOSED ){
			this->UnregisterForRead();
		}else{
			this->RegisterForRead();
		}
		if( this->write_state == WRITE_WAITING ){
			this->RegisterForWrite();
		}else{
			this->UnregisterForWrite();
		}
		if( this->error_state == ERROR_ON ){
			this->UnregisterForError();
		}else{
			this->RegisterForError();
		}
	}

	void TCPSocketBinding::ClearReactor()
	{
		UnregisterForRead();
		UnregisterForError();
		UnregisterForWrite();
		UnregisterForTimeout();
		notifier.stop();
	}

	void TCPSocketBinding::InvokeErrorHandler(const std::string &str)
	{
		
		if (!this->onError.isNull())
		{
			ValueList args;
			args.push_back(Value::NewString(str.c_str()));
			ti_host->RunOnMainThread(this->onError, args, false);
		}
	}
}

