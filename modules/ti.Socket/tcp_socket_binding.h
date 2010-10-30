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
		TCPSocketBinding(Host *host, const std::string & hostname, const std::string& port);
		virtual ~TCPSocketBinding();

		void OnError(const std::string& error_text);

		static void Initialize();
		static void UnInitialize();
	private:

		static asio::io_service io_service;
		static std::auto_ptr<asio::io_service::work> io_idlework;

		Host* ti_host;
		const std::string hostname;
		const std::string port;

		bool nonBlocking;
		bool useKeepAlives;
		//int inactivetime;
		//int resendtime;
		tcp::resolver resolver;
		tcp::socket socket;

		char read_data_buffer[BUFFER_SIZE + 1];

		asio::detail::mutex write_mutex;
		std::deque<std::string> write_buffer;

		enum SOCK_STATE_en { SOCK_CLOSED, SOCK_CONNECTING, SOCK_CONNECTED, SOCK_CLOSING } sock_state;

		void setKeepAlive(bool keepAlive);

		void registerHandleResolve();
		void handleResolve(const asio::error_code& error, tcp::resolver::iterator endpoint_iterator);
		void registerHandleConnect(tcp::resolver::iterator endpoint_iterator);
		void handleConnect(const asio::error_code& error, tcp::resolver::iterator endpoint_iterator);

		void registerHandleRead();
		void handleRead(const asio::error_code& error, std::size_t bytes_transferred);

		void writeAsync(const std::string &data);
		void registerHandleWrite();
		void handleWrite(const asio::error_code& error, std::size_t bytes_transferred);

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

		//void OnReadReady(ReadableNotification * notification);
		//void OnWriteReady(WritableNotification * notification);
		//void OnError(ErrorNotification * notification);

		void OnConnect();
		void OnWrite();
		void OnRead(char * data, int size);
		void OnClose();
		void CompleteClose();
		void SetKeepAliveOpts();

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
			}
			else
			{
				GetLogger()->Warn("TCPSocket::onRead: not read subscriber registered:  " + string(read_data_buffer));
			}
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
