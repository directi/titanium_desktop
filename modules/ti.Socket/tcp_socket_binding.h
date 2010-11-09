/**
 * @author: Mital Vora <mital.d.vora@gmail.com>
 */

#ifndef _TCP_SOCKET_BINDING_H_
#define _TCP_SOCKET_BINDING_H_


#include "Socket.h"
#include "SocketExceptions.h"


namespace ti
{
	class TCPSocketBinding
		: public Socket<tcp::socket>
	{
	public:
		TCPSocketBinding(Host *host,
			const std::string & hostname,
			const std::string& port);
		virtual ~TCPSocketBinding();

	private:

		KMethodRef onConnect;

		void SetOnConnect(const ValueList& args, KValueRef result);

		void Connect(const ValueList& args, KValueRef result);
		void ConnectNB(const ValueList& args, KValueRef result);
		void SetKeepAlives(const ValueList& args, KValueRef result);
		void SetKeepAliveTimes(const ValueList& args, KValueRef result);


		const std::string hostname;
		const std::string port;

		tcp::resolver resolver;

		bool connect(long timeout = 10);
		void connectNB();
		void on_connect();

		void setKeepAlive(bool keep_alives);
		//void setKeepAliveTimes(int inactivetime, int resendtime);

		tcp::resolver::iterator resolveHost();
		bool tryConnect(tcp::resolver::iterator endpoint_iterator);

		void registerHandleResolve();

		void handleResolve(const asio::error_code& error, tcp::resolver::iterator endpoint_iterator);
		void registerHandleConnect(tcp::resolver::iterator endpoint_iterator);
		void handleConnect(const asio::error_code& error, tcp::resolver::iterator endpoint_iterator);
	};
}

#endif
