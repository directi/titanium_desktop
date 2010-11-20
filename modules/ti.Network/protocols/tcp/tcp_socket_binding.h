/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _TCP_SOCKET_BINDING_H_
#define _TCP_SOCKET_BINDING_H_

#include <kroll/kroll.h>
#include <Poco/Thread.h>
#include <Poco/Net/StreamSocket.h>
#include <Poco/Net/SocketReactor.h>
#include <Poco/Net/SocketNotification.h>
#include <Poco/Semaphore.h>
#include <Poco/Condition.h>
#include <Poco/NObserver.h>
#include <Poco/Pipe.h>
#include <Poco/Timespan.h>

using namespace Poco;
using namespace Poco::Net;

namespace ti
{
	class DisconnectAwareSocket : public StreamSocket 
	{
	public:
		DisconnectAwareSocket(int);
	private:
		inline static kroll::Logger* GetLogger()
		{
			return kroll::Logger::Get("Network.TCPSocket");
		}
	};

	static const long SELECT_TIME_MICRO = 1000; // 100ms

	class QuieterSocketReactor : public SocketReactor 
	{
	public:
		QuieterSocketReactor(const Timespan& t);
		void wakeup();
	protected:
		virtual void onIdle();
	private:
		bool waiting;
		Mutex conditionLock;
		Condition idle;
	};

	class TCPSocketBinding : public StaticBoundObject
	{
	public:
		TCPSocketBinding(Host *ti_host, const std::string & host, const int port);
		virtual ~TCPSocketBinding();
		void setSocketObject(StreamSocket *socket) { this->socket = socket; }
		StreamSocket* getSocketObject() { return this->socket; }

		void OnResolve(SocketAddress*);
		void OnError(const std::string& error_text);

		static void shutdown();
		static void addSocket(TCPSocketBinding* tsb);
		static void removeSocket(TCPSocketBinding* tsb);
		static void removeWriteListener(TCPSocketBinding* tsb);

	private:
		Poco::Mutex lock;
		inline static kroll::Logger* GetLogger()
		{
			return kroll::Logger::Get("Network.TCPSocket");
		}
		static QuieterSocketReactor reactor;
		static Poco::Thread pollThread;

		Host* ti_host;
		const std::string host;
		const int port;
		StreamSocket *socket;
		bool nonBlocking;
		bool useKeepAlives;
		int inactivetime;

		enum SOCK_STATE_en { SOCK_CLOSED, SOCK_CONNECTING, SOCK_CONNECTED, SOCK_CLOSING } sock_state;

		KMethodRef onConnect;
		KMethodRef onRead;
		KMethodRef onError;
		KMethodRef onClose;

		void SetOnConnect(const ValueList& args, KValueRef result);
		void SetOnRead(const ValueList& args, KValueRef result);
		void SetOnWrite(const ValueList& args, KValueRef result);
		void SetOnTimeout(const ValueList& args, KValueRef result);
		void SetOnError(const ValueList& args, KValueRef result);
		void SetOnClose(const ValueList& args, KValueRef result);
		void SetOnReadComplete(const ValueList& args, KValueRef result);

		void Connect(const ValueList& args, KValueRef result);
		void ConnectNB(const ValueList& args, KValueRef result);
		void Write(const ValueList& args, KValueRef result);
		void Read(const ValueList& args, KValueRef result);
		void Close(const ValueList& args, KValueRef result);
		void IsClosed(const ValueList& args, KValueRef result);
		void SetKeepAlives(const ValueList& args, KValueRef result);
		void SetKeepAliveTimes(const ValueList& args, KValueRef result);

		void OnReadReady(ReadableNotification * notification);
		void OnWriteReady(WritableNotification * notification);
		void OnError(ErrorNotification * notification);

		void OnConnect();
		void OnWrite();
		void OnRead(char * data, int size);
		void OnClose();
		void CompleteClose();
		void SetKeepAliveOpts();
	};
}

#endif
