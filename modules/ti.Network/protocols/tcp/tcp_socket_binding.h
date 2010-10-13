/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _TCP_SOCKET_BINDING_H_
#define _TCP_SOCKET_BINDING_H_

#ifndef WINVER
#define WINVER 0x0501
#endif

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501
#endif

#ifndef _WIN32_WINDOWS
#define _WIN32_WINDOWS 0x0410
#endif

#ifndef _WIN32_IE
#define _WIN32_IE 0x600
#endif

#include <kroll/kroll.h>
#include <Poco/Thread.h>
#include <Poco/Net/StreamSocket.h>
#include <Poco/Net/SecureStreamSocket.h>
#include <Poco/Net/SocketReactor.h>
#include <Poco/Net/SocketNotification.h>
#include <Poco/Semaphore.h>
#include <Poco/NObserver.h>
#include <Poco/Pipe.h>
#include "IONotifier.h"

using namespace Poco;
using namespace Poco::Net;

namespace ti
{

	class TiStreamSocket : public StreamSocket {
	public:
		int sockfd()
		{
			return StreamSocket::sockfd();
		}
	};

	class TiSecureStreamSocket : public SecureStreamSocket {
	public:
		int sockfd()
		{
			return SecureStreamSocket::sockfd();
		}
	};

	class TCPSocketBinding : public StaticBoundObject
	{
	public:
		TCPSocketBinding(Host *ti_host, const std::string & host, const int port);
		virtual ~TCPSocketBinding();

		void RegisterObservers();
		void UnRegisterObservers();

		StreamSocket * getSocketObject() { return socket; }
		StreamSocket * setSocketObject(StreamSocket * newSocket)
		{
			// we no more maintain the older socket pointer
			// older socket handler should now be managed by the callee of this function
			StreamSocket * old = socket;
			socket = newSocket;
			return old;
		}

	private:
		static kroll::Logger* GetLogger()
		{
			return kroll::Logger::Get("Network.TCPSocket");
		}

		Host* ti_host;
		const std::string host;
		const int port;
		StreamSocket *socket;
		std::string buffer;
		Poco::Mutex bufferMutex; 
		Poco::Semaphore semWaitForConnect;
		int sockfd;

		enum SOCK_STATE_en {SOCK_CLOSED, SOCK_CONNECTING, SOCK_CONNECTED } sock_state;
		enum ERROR_STATE_en {ERROR_OFF, ERROR_ON } error_state;
		enum READ_STATE_en { READ_OPEN, READ_CLOSED } read_state;
		enum WRITE_STATE_en { WRITE_OPEN, WRITE_WAITING, WRITE_CLOSED } write_state;
		IONotifier<TCPSocketBinding> notifier;

		KMethodRef onConnect;
		KMethodRef onRead;
		KMethodRef onWrite;
		KMethodRef onTimeout;
		KMethodRef onError;
		KMethodRef onReadComplete;

		void InitReactor();
		void ClearReactor();
		void CompleteClose();

		void Connect(const ValueList& args, KValueRef result);
		void ConnectNB(const ValueList& args, KValueRef result);
		bool _connect(int timeout, bool nonBlocking);
		
		void OnNonBlockingConnect();
		void OnNonBlockingConnectFailure();
		void blockForConnectionOrTimeout(int secs);
		void unBlockOnConnectionOrTimeout();

		void Write(const ValueList& args, KValueRef result);
		void Close(const ValueList& args, KValueRef result);
		void ClearError(const ValueList& args, KValueRef result);

		void IsClosed(const ValueList& args, KValueRef result);
		void SetOnConnect(const ValueList& args, KValueRef result);
		void SetOnRead(const ValueList& args, KValueRef result);
		void SetOnWrite(const ValueList& args, KValueRef result);
		void SetOnTimeout(const ValueList& args, KValueRef result);
		void SetOnError(const ValueList& args, KValueRef result);
		void SetOnReadComplete(const ValueList& args, KValueRef result);

		void OnReadReady(IONotification * notification);
		void OnWriteReady(IONotification * notification);
		void OnTimeout(IONotification * notification);
		void OnError(IONotification * notification);


		int getSockFDFrom(StreamSocket * _socket)
		{
			if(sockfd == -1)
			{
				TiStreamSocket* sockref = dynamic_cast<TiStreamSocket *>(_socket);
				if (sockref)
				{
					sockfd = sockref->sockfd();
				}
			}
			return sockfd;
		}
		void RegisterForRead()
		{
			int sockfd = getSockFDFrom(this->socket);
			IOObserver<TCPSocketBinding> observer(sockfd, *this, &TCPSocketBinding::OnReadReady);
			bool status = this->notifier.addObserver(IO_READ, observer);
			if(status) GetLogger()->Debug("Added Read EventHandler on Socket: %s:%d ", this->host.c_str(), this->port);
		}
		void UnregisterForRead()
		{
			int sockfd = getSockFDFrom(this->socket);
			IOObserver<TCPSocketBinding> observer(sockfd, *this, &TCPSocketBinding::OnReadReady);
			bool status = this->notifier.removeObserver(IO_READ, observer);
			if(status) GetLogger()->Debug("Removed Read EventHandler on Socket: %s:%d ", this->host.c_str(), this->port);
		}
		void RegisterForWrite()
		{
			int sockfd = getSockFDFrom(this->socket);
			IOObserver<TCPSocketBinding> observer(sockfd, *this, &TCPSocketBinding::OnWriteReady);
			bool status=this->notifier.addObserver(IO_WRITE, observer);
			if(status) GetLogger()->Debug("Added Write EventHandler on Socket: %s:%d ", this->host.c_str(), this->port);
		}

		void UnregisterForWrite()
		{
			int sockfd = getSockFDFrom(this->socket);
			IOObserver<TCPSocketBinding> observer(sockfd, *this, &TCPSocketBinding::OnWriteReady);
			bool status=this->notifier.removeObserver(IO_WRITE, observer);
			if(status) GetLogger()->Debug("Removed Write EventHandler on Socket: %s:%d ", this->host.c_str(), this->port);
		}

		// TODO: add timeouts only for connectNB()
		void RegisterForTimeout()
		{
			//int sockfd = getSockFDFrom(this->socket);
			//IOObserver<TCPSocketBinding> observer(sockfd, *this, &TCPSocketBinding::OnTimeout);
			//this->notifier.addObserver(IO_TIMEOUT, observer);
		}

		void UnregisterForTimeout()
		{
			//int sockfd = getSockFDFrom(this->socket);
			//IOObserver<TCPSocketBinding> observer(sockfd, *this, &TCPSocketBinding::OnTimeout);
			//this->notifier.removeObserver(IO_TIMEOUT, observer);
		}

		void RegisterForError()
		{
			int sockfd = getSockFDFrom(this->socket);
			IOObserver<TCPSocketBinding> observer(sockfd, *this, &TCPSocketBinding::OnError);
			bool status = this->notifier.addObserver(IO_ERROR, observer);
			if(status) GetLogger()->Debug("Added Error EventHandler on Socket: %s:%d ", this->host.c_str(), this->port);
		}

		void UnregisterForError()
		{
			int sockfd = getSockFDFrom(this->socket);
			IOObserver<TCPSocketBinding> observer(sockfd, *this, &TCPSocketBinding::OnError);
			bool status = this->notifier.removeObserver(IO_ERROR, observer);
			if(status) GetLogger()->Debug("Removed Error EventHandler on Socket: %s:%d ", this->host.c_str(), this->port);
		}

		void InvokeErrorHandler(const std::string &str);
		void SetReactorDescriptors();
	};
}

#endif
