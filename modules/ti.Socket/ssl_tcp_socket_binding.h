/**
* @author: Mital Vora <mital.d.vora@gmail.com>
*/

#ifndef _SSL_TCP_SOCKET_H_
#define _SSL_TCP_SOCKET_H_


#include "tcp_socket_binding.h"
#include <asio/ssl.hpp>


namespace ti
{
	class SecureTCPSocketBinding
		: public Socket<asio::ssl::stream<tcp::socket> >
	{
	public:
		SecureTCPSocketBinding(Host *host, TCPSocketBinding * socket);

		virtual ~SecureTCPSocketBinding() {}

	protected:

		//template <class T>
		virtual bool CompleteClose()
		{
			// Log  ->Debug("Closing socket to: %s:%d ", this->hostname.c_str(), this->port.c_str());
			if ((this->sock_state == SOCK_CONNECTED)
				|| (this->sock_state == SOCK_CONNECTING))
			{
				this->sock_state = SOCK_CLOSING;
				if (socket)
				{
					//tcp::socket &t = socket->lowest_layer();
					//t.close();
					socket->lowest_layer().close();
				}
				this->sock_state = SOCK_CLOSED;
				return true;
			}
			return false;
		}


		//asio::ssl::context *ctx;
		//ctx.set_verify_mode(boost::asio::ssl::context::verify_peer);
	};
}
#endif // _SSL_TCP_SOCKET_H_
