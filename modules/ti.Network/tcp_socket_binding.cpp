/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "tcp_socket_binding.h"
#include <Poco/NObserver.h>
#include <kroll/kroll.h>

#define BUFFER_SIZE 1024   // choose a reasonable size to send back to JS

namespace ti
{
	static kroll::Logger* GetLogger()
	{
		return kroll::Logger::Get("Network.TCPSocket");
	}

	TCPSocketBinding::TCPSocketBinding(Host* ti_host, const std::string& host, const int port) :
		StaticBoundObject("Network.TCPSocket"),
		ti_host(ti_host),
		host(host),
		port(port),
		opened(false),
		nbConnecting(false),
		onRead(0),
		onWrite(0),
		onTimeout(0),
		onConnect(0),
		onReadComplete(0),
		semWaitForConnect(0,1),
		waitingForWriteReady(false)
	{
		/**
		 * @tiapi(method=True,name=Network.TCPSocket.connect,since=0.2) Connects a Socket object to the host specified during creation. 
		 * @tiarg(for=Network.TCPSocket.connect,type=Integer,name=timeout) the time in seconds to wait before the connect timesout 
		 * @tiarg(for=Network.TCPSocket.connect,type=Boolean,name=nonBlocking) if set to true, attempts a non-blocking connect and calls back onConnect
		 * @tiresult(for=Network.TCPSocket.connect,type=Boolean) true if the Socket object successfully connects, false if otherwise
		 */
		this->SetMethod("connect",&TCPSocketBinding::Connect);
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
		return result->SetBool(!this->opened);
	}
	void TCPSocketBinding::Connect(const ValueList& args, KValueRef result)
	{
		int timeout = 1;
		bool nonBlocking = false;
		if (args.size() > 0)
		{
			timeout = args.at(0)->ToInt();
		}
		if ( args.size() > 1 )
		{
			nonBlocking = args.at(1)->ToBool();
		}
		std::string eprefix = "Connect exception: ";
		if (this->opened)
		{
			throw ValueException::FromString(eprefix + "Socket is already open");
		}
		if (this->nbConnecting)
		{
			throw ValueException::FromString(eprefix + "Socket is already connecing in non-blocking mode");
		}
		try
		{
			SocketAddress a(this->host.c_str(), this->port);
			this->reactor.setTimeout(Poco::Timespan(timeout, 0));
			this->InitReactor();
			this->socket.connectNB(a);
			if(nonBlocking)
			{
				GetLogger()->Debug("Connecting non Blocking.");
				this->nbConnecting = true;
				result->SetBool(true);
			}
			else
			{
				GetLogger()->Debug("Connecting Blocking.");
				this->waitForConnectionOrTimeout(timeout);
				result->SetBool(this->opened);
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
	}
	// called when the socket is ready for read.
	void TCPSocketBinding::OnRead(const Poco::AutoPtr<ReadableNotification>& n)
	{
		if(!this->socket.getBlocking())
		{
			GetLogger()->Debug("TCPSocketBinding::OnRead making socket blocking");
			this->socket.setBlocking(true);
			this->OnNonBlockingConnect();
		}
		try
		{
			// Always read bytes, so that the tubes get cleared.
			char data[BUFFER_SIZE + 1];
			int size = socket.receiveBytes(&data, BUFFER_SIZE);

			bool read_complete = (size <= 0);
			if (read_complete && !this->onReadComplete.isNull())
			{
				ValueList args;
				this->UnregisterForReadReady();
				ti_host->InvokeMethodOnMainThread(this->onReadComplete, args, false);
			}
			else if (!read_complete && !this->onRead.isNull())
			{
				data[size] = '\0';

				ValueList args;
				args.push_back(Value::NewString(data));
				ti_host->InvokeMethodOnMainThread(this->onRead, args, false);
			}
		}
		catch(ValueException& e)
		{
			GetLogger()->Error("Read Failed: %s", e.ToString().c_str());
			InvokeErrorHandler(e.ToString());
		}
		catch(Poco::Exception &e)
		{
			std::string error_text = "Read failed: %s";
			error_text += e.displayText().c_str();
			GetLogger()->Error(error_text.c_str());
			InvokeErrorHandler(error_text, true);
		}
		catch(...)
		{
			GetLogger()->Error("Read failed: Unknown Exception");
			InvokeErrorHandler(std::string("Unknown exception"), true);
		}
	}
	//Called when the socket is ready for write
	void TCPSocketBinding::OnWrite(const Poco::AutoPtr<WritableNotification>& n)
	{
		if(!this->socket.getBlocking())
		{
			GetLogger()->Debug("TCPSocketBinding::OnWrite making socket blocking");
			this->socket.setBlocking(true);
			this->OnNonBlockingConnect();
		}		
		int count = 0;
		GetLogger()->Debug("TCPSocketBinding::OnWrite Callback on Socket: "  + this->socket.address().toString());
		try
		{
			Poco::Mutex::ScopedLock lock(bufferMutex);
			if (!buffer.empty())
			{
				count = this->socket.sendBytes(buffer.c_str(), buffer.length());
				GetLogger()->Debug("TCPSocketBinding::OnWrite: SendBytes Complete: "  + buffer.substr(0,count));
				buffer = buffer.substr(count);
			}
			if(buffer.empty())
			{
				this->UnregisterForWriteReady();
				GetLogger()->Debug("TCPSocketBinding::OnWrite: Removed Write EventHandler on Socket: "  + this->socket.address().toString());
			}
		}
		catch (...)
		{
			GetLogger()->Error("TCPSocketBinding::OnWrite: Write failed: Unknown Exception");
			InvokeErrorHandler(std::string("Unknown exception"));
		}

		if (count >0 && !this->onWrite.isNull())
		{
			ValueList args;
			args.push_back(Value::NewInt(count));
			ti_host->InvokeMethodOnMainThread(this->onWrite, args, false);
		}
	}
	void TCPSocketBinding::OnTimeout(const Poco::AutoPtr<TimeoutNotification>& n)
	{
		if (this->onTimeout.isNull())
		{
			return;
		}
		ValueList args;
		ti_host->InvokeMethodOnMainThread(this->onTimeout, args, false);
	}
	void TCPSocketBinding::OnError(const Poco::AutoPtr<ErrorNotification>& n)
	{
		GetLogger()->Error("Socket Error!");
		if(!this->socket.getBlocking()){
			this->OnNonBlockingConnectFailure();
		}
		InvokeErrorHandler(n->name());
	}
	void TCPSocketBinding::Write(const ValueList& args, KValueRef result)
	{
		std::string eprefix = "TCPSocketBinding::Write: ";
		if (!this->opened)
		{
			throw ValueException::FromString(eprefix +  "Socket is not open");
		}

		try
		{
			Poco::Mutex::ScopedLock lock(bufferMutex);
			if(buffer.length() == 0 ){
				this->RegisterForWriteReady();
			}
			buffer += args.at(0)->ToString();
			result->SetBool(true);
		}
		catch(Poco::Exception &e)
		{
			throw ValueException::FromString(eprefix + e.displayText());
		}

	}
	void TCPSocketBinding::Close(const ValueList& args, KValueRef result)
	{
		if (this->opened)
		{
			this->CompleteClose();
			result->SetBool(true);
		}
		else
		{
			result->SetBool(false);
		}
	}

	void TCPSocketBinding::OnNonBlockingConnect(){
		this->socket.setKeepAlive(true);
		this->opened = true;
		this->nbConnecting = false;
		this->semWaitForConnect.set();
		if(!this->onConnect.isNull()){
			ValueList args;
			args.push_back(Value::NewBool(true));
			ti_host->InvokeMethodOnMainThread(this->onConnect, args, false);
		}
	}

	void TCPSocketBinding::OnNonBlockingConnectFailure(){
		this->CompleteClose();
		this->semWaitForConnect.set();
	}

	void TCPSocketBinding::waitForConnectionOrTimeout(int secs)
	{
		try
		{
			this->semWaitForConnect.wait(secs*1000);			
			GetLogger()->Debug("Socket connect to : "  + this->socket.address().toString());
		}
		catch (Poco::TimeoutException &e)
		{
			GetLogger()->Debug("Timeout in a blocking connect to : "  + this->socket.address().toString());
		}
	}

	void TCPSocketBinding::CompleteClose()
	{
		this->reactor.stop();
		if(this->opened)
		{
			this->socket.close();
		}
		this->opened = false;
		this->nbConnecting = false;
		this->ClearReactor();
	}

	void TCPSocketBinding::RegisterForWriteReady()
	{
		if( ! this->waitingForWriteReady ){
			this->reactor.addEventHandler(this->socket,NObserver<TCPSocketBinding, WritableNotification>(*this, &TCPSocketBinding::OnWrite));
			this->waitingForWriteReady = true;
			GetLogger()->Debug("Added Write Listener for " + this->host);
		}
	}

	void TCPSocketBinding::UnregisterForWriteReady()
	{
		if(this->waitingForWriteReady) {
			this->reactor.removeEventHandler(this->socket,NObserver<TCPSocketBinding, WritableNotification>(*this, &TCPSocketBinding::OnWrite));
			this->waitingForWriteReady = false;
			GetLogger()->Debug("Removed Write Listener for " + this->host);
		}
	}

	void TCPSocketBinding::UnregisterForReadReady(){
		this->reactor.removeEventHandler(this->socket,NObserver<TCPSocketBinding, ReadableNotification>(*this, &TCPSocketBinding::OnRead));
	}


	void TCPSocketBinding::InitReactor()
	{
		this->reactor.addEventHandler(this->socket,NObserver<TCPSocketBinding, ReadableNotification>(*this, &TCPSocketBinding::OnRead));
		RegisterForWriteReady();
		this->reactor.addEventHandler(this->socket,NObserver<TCPSocketBinding, TimeoutNotification>(*this, &TCPSocketBinding::OnTimeout));
		this->reactor.addEventHandler(this->socket,NObserver<TCPSocketBinding, ErrorNotification>(*this, &TCPSocketBinding::OnError));
		this->thread = new Poco::Thread();
		this->thread->setName("SIO " + host);
		this->thread->start(this->reactor);
	}

	void TCPSocketBinding::ClearReactor()
	{
		this->UnregisterForReadReady();
		this->UnregisterForWriteReady();
		this->reactor.removeEventHandler(this->socket,NObserver<TCPSocketBinding, TimeoutNotification>(*this, &TCPSocketBinding::OnTimeout));
		this->reactor.removeEventHandler(this->socket,NObserver<TCPSocketBinding, ErrorNotification>(*this, &TCPSocketBinding::OnError));
		if(this->thread)
		{
			this->thread->join();
			delete this->thread;
			this->thread = NULL;
		}
	}

	void TCPSocketBinding::InvokeErrorHandler(const std::string &str, bool readError)
	{
		if(readError)
		{
			this->UnregisterForReadReady();
		}

		if (!this->onError.isNull())
		{
			ValueList args;
			args.push_back(Value::NewString(str.c_str()));
			ti_host->InvokeMethodOnMainThread(this->onError, args, false);
		}
	}
}

