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
#include <Poco/Net/SocketReactor.h>
#include <Poco/Net/SocketNotification.h>
#include <Poco/Semaphore.h>

using namespace Poco;
using namespace Poco::Net;

namespace ti
{
	class TCPSocketBinding : public StaticBoundObject
	{
	public:
		TCPSocketBinding(Host *ti_host, const std::string & host, const int port);
		virtual ~TCPSocketBinding();
	private:
		Host* ti_host;
		const std::string host;
		const int port;
		StreamSocket socket;
		SocketReactor reactor;
		Thread *thread;
		bool opened;
		bool nbConnecting;
		bool waitingForWriteReady;
		std::string buffer;
		Poco::Mutex bufferMutex; 
		Poco::Semaphore semWaitForConnect;

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
		void OnNonBlockingConnect();
		void OnNonBlockingConnectFailure();
		void waitForConnectionOrTimeout(int secs);

		void Write(const ValueList& args, KValueRef result);
		void Close(const ValueList& args, KValueRef result);
		void IsClosed(const ValueList& args, KValueRef result);
		void SetOnConnect(const ValueList& args, KValueRef result);
		void SetOnRead(const ValueList& args, KValueRef result);
		void SetOnWrite(const ValueList& args, KValueRef result);
		void SetOnTimeout(const ValueList& args, KValueRef result);
		void SetOnError(const ValueList& args, KValueRef result);
		void SetOnReadComplete(const ValueList& args, KValueRef result);

		void OnRead(const Poco::AutoPtr<ReadableNotification>& n);
		void OnWrite(const Poco::AutoPtr<WritableNotification>& n);
		void OnTimeout(const Poco::AutoPtr<TimeoutNotification>& n);
		void OnError(const Poco::AutoPtr<ErrorNotification>& n);

		void RegisterForWriteReady();
		void UnregisterForWriteReady();
		void UnregisterForReadReady();
		void InvokeErrorHandler(const std::string &str, bool readError = false);

	};
}

#endif
