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
	class TCPSocketBinding : public StaticBoundObject
	{
	public:
		TCPSocketBinding(Host *ti_host, const std::string & host, const int port);
		virtual ~TCPSocketBinding();
		void setSocketObject(StreamSocket *socket) { this->socket = socket; }
		StreamSocket* getSocketObject() { return this->socket; }

		static void shutdown();
		static void addSocket(TCPSocketBinding* tsb);
		static void removeSocket(TCPSocketBinding* tsb);
		static void addWriteListener(TCPSocketBinding* tsb);
		static void removeWriteListener(TCPSocketBinding* tsb);

	private:
		static kroll::Logger* GetLogger()
		{
			return kroll::Logger::Get("Network.TCPSocket");
		}
		static Poco::Net::SocketReactor reactor;
		static Poco::Thread pollThread;

		Host* ti_host;
		const std::string host;
		const int port;
		StreamSocket *socket;
		bool nonBlocking;

		enum SOCK_STATE_en {SOCK_CLOSED, SOCK_CONNECTING, SOCK_CONNECTED } sock_state;

		KMethodRef onConnect;
		KMethodRef onRead;
		KMethodRef onWrite;
		KMethodRef onTimeout;
		KMethodRef onError;
		KMethodRef onClose;

		void SetOnConnect(const ValueList& args, KValueRef result);
		void SetOnRead(const ValueList& args, KValueRef result);
		void SetOnWrite(const ValueList& args, KValueRef result);
		void SetOnTimeout(const ValueList& args, KValueRef result);
		void SetOnError(const ValueList& args, KValueRef result);
		void SetOnClose(const ValueList& args, KValueRef result);

		void Connect(const ValueList& args, KValueRef result);
		void ConnectNB(const ValueList& args, KValueRef result);
		void Write(const ValueList& args, KValueRef result);
		void Read(const ValueList& args, KValueRef result);
		void Close(const ValueList& args, KValueRef result);
		void IsClosed(const ValueList& args, KValueRef result);

		void OnReadReady(ReadableNotification * notification);
		void OnWriteReady(WritableNotification * notification);
		void OnError(ErrorNotification * notification);

		void OnConnect();
		void OnWrite();
		void OnRead(char * data, int size);
		void OnError(const std::string& error_text);
		SocketAddress* beforeConnect();
		void OnClose();
		void CompleteClose();
	};
}

#endif
