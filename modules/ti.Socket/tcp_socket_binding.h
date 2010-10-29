/**
 * @author: Mital Vora <mital.d.vora@gmail.com>
 */

#ifndef _TCP_SOCKET_BINDING_H_
#define _TCP_SOCKET_BINDING_H_

#ifdef OS_WIN32
#ifndef WINVER
#define WINVER 0x0502
#endif

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0502
#endif

#ifndef _WIN32_WINDOWS
#define _WIN32_WINDOWS 0x0410
#endif
#endif

#include <asio.hpp>
using asio::ip::tcp;

#include <kroll/kroll.h>

#include <deque>

#define BUFFER_SIZE 1024   // choose a reasonable size to send back to JS


namespace ti
{
	class TCPSocketBinding : public StaticBoundObject
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
		//bool useKeepAlives;
		//int inactivetime;
		//int resendtime;
		tcp::resolver resolver;
		tcp::socket socket;
		char data[BUFFER_SIZE + 1];
		std::deque<std::string> write_msgs;

		enum SOCK_STATE_en { SOCK_CLOSED, SOCK_CONNECTING, SOCK_CONNECTED, SOCK_CLOSING } sock_state;

		void registerHandleResolve();
		void handleResolve(const asio::error_code& error, tcp::resolver::iterator endpoint_iterator);
		void registerHandleConnect(tcp::resolver::iterator endpoint_iterator);
		void handleConnect(const asio::error_code& error, tcp::resolver::iterator endpoint_iterator);

		void registerHandleRead();
		void handleRead(const asio::error_code& error, std::size_t bytes_transferred);

		void registerHandleWrite();
		void handleWrite(char * buf,
			const asio::error_code& error, std::size_t bytes_transferred);

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
	};
}

#endif
