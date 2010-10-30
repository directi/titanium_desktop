/**
 * @author: Mital Vora <mital.d.vora@gmail.com>
 */

#ifndef _TCP_SOCKET_BINDING_H_
#define _TCP_SOCKET_BINDING_H_

#include <kroll/kroll.h>

// TODO: This is poco UnWindows.h's curse.... to be removed with poco
#ifdef UNICODE
#define CreateEvent  CreateEventW
#else
#define CreateEvent  CreateEventA
#endif // !UNICODE

#include "TCPSocket.h"


namespace ti
{
	class TCPSocketBinding
		: public StaticBoundObject,
		public TCPSocketHandler
	{
	public:
		TCPSocketBinding(Host *host,
			const std::string & hostname,
			const std::string& port);
		virtual ~TCPSocketBinding();

	private:

		Host* ti_host;
		TCPSocket socket;

		inline static kroll::Logger* GetLogger()
		{
			return kroll::Logger::Get("Socket.TCPSocket");
		}

		KMethodRef onConnect;
		KMethodRef onRead;
		KMethodRef onError;
		KMethodRef onClose;

		void SetOnConnect(const ValueList& args, KValueRef result);
		void SetOnRead(const ValueList& args, KValueRef result);
		void SetOnError(const ValueList& args, KValueRef result);
		void SetOnClose(const ValueList& args, KValueRef result);

		void Connect(const ValueList& args, KValueRef result);
		void ConnectNB(const ValueList& args, KValueRef result);
		void Write(const ValueList& args, KValueRef result);
		void Read(const ValueList& args, KValueRef result);
		void Close(const ValueList& args, KValueRef result);
		void IsClosed(const ValueList& args, KValueRef result);
		void SetKeepAlives(const ValueList& args, KValueRef result);
		void SetKeepAliveTimes(const ValueList& args, KValueRef result);

		virtual void on_connect()
		{
			if(!this->onConnect.isNull()) 
			{
				ValueList args;
				RunOnMainThread(this->onConnect, args, false);
			}
		}
		virtual void on_read(char * data, int size)
		{
			if(!this->onRead.isNull()) 
			{
				BytesRef bytes(new Bytes(data, size));
				ValueList args (Value::NewObject(bytes));
				RunOnMainThread(this->onRead, args, false);
				return;
			}
			GetLogger()->Warn("TCPSocket::onRead: not read subscriber registered:  " + string(data));
		}
		virtual void on_error(const std::string& error_text)
		{
			if(!this->onError.isNull()) 
			{
				ValueList args (Value::NewString(error_text.c_str()));
				RunOnMainThread(this->onError, args, false);
			}
		}
		virtual void on_close()
		{
			if(!this->onClose.isNull()) 
			{
				ValueList args;
				RunOnMainThread(this->onClose, args, false);
			}
		}
	};
}

#endif
