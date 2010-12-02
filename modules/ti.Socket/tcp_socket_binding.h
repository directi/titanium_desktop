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

		tcp::socket * resetSocket();

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

		void handleResolve(const boost::system::error_code& error, tcp::resolver::iterator endpoint_iterator);
		void registerHandleConnect(tcp::resolver::iterator endpoint_iterator);
		void handleConnect(const boost::system::error_code& error, tcp::resolver::iterator endpoint_iterator);

		virtual bool CompleteClose()
		{
			// Log  ->Debug("Closing socket to: %s:%d ", this->hostname.c_str(), this->port.c_str());
			if ((this->sock_state == SOCK_CONNECTED)
				|| (this->sock_state == SOCK_CONNECTING))
			{
				this->sock_state = SOCK_CLOSING;
				if (socket)
				{
					socket->close();
				}
				this->sock_state = SOCK_CLOSED;
				return true;
			}
			return false;
		}

	};
}

#endif
