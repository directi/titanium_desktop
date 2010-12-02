/**
* @author: Mital Vora <mital.d.vora@gmail.com>
*/

#ifndef _SSL_TCP_SOCKET_H_
#define _SSL_TCP_SOCKET_H_


#include "tcp_socket_binding.h"
#include <boost/asio/ssl.hpp>


namespace ti
{
	class SecureTCPSocket
		: public Socket<boost::asio::ssl::stream<tcp::socket&> >
	{
	public:
		SecureTCPSocket(Host *host, TCPSocketBinding * tcp_socket_binding);
		virtual ~SecureTCPSocket();

	protected:

		virtual bool CompleteClose()
		{
			// Log  ->Debug("Closing socket to: %s:%d ", this->hostname.c_str(), this->port.c_str());
			if ((this->sock_state == SOCK_CONNECTED)
				|| (this->sock_state == SOCK_CONNECTING))
			{
				this->sock_state = SOCK_CLOSING;
				if (socket)
				{
					socket->lowest_layer().close();
				}
				this->sock_state = SOCK_CLOSED;
				return true;
			}
			return false;
		}

	private:
		KMethodRef onHandshake;

		void SetOnHandshake(const ValueList& args, KValueRef result);

		void async_handshake(const ValueList& args, KValueRef result);
		void registerAsyncHandshake();
		void handleAsyncHandshake(const boost::system::error_code& error);
		void on_handshake();


		boost::asio::ssl::context ctx;
		tcp::socket * tcp_socket;
		//ctx.set_verify_mode(boost::asio::ssl::context::verify_peer);
	};
}
#endif // _SSL_TCP_SOCKET_H_
