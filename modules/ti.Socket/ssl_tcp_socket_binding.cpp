/**
* @author: Mital Vora <mital.d.vora@gmail.com>
*/

#include "ssl_tcp_socket_binding.h"

namespace ti
{
	SecureTCPSocket::SecureTCPSocket(Host *host, TCPSocketBinding * tcp_socket_binding)
		: Socket(host, string("Socket.SecureTCPSocket")),
		ctx(*(Socket::io_service.get()), asio::ssl::context::sslv23),
		tcp_socket(tcp_socket_binding->resetSocket())
	{
		this->copyHandlers(tcp_socket_binding);
		ctx.set_verify_mode(asio::ssl::context::verify_none);

		socket = new asio::ssl::stream<tcp::socket&>(*tcp_socket, ctx);
	}
	SecureTCPSocket::~SecureTCPSocket()
	{
		this->CompleteClose();
	}

}