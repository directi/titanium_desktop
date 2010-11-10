/**
* @author: Mital Vora <mital.d.vora@gmail.com>
*/

#include "ssl_tcp_socket_binding.h"

namespace ti
{
	SecureTCPSocketBinding::SecureTCPSocketBinding(Host *host, TCPSocketBinding * socket)
			: Socket(host, string("Socket.TCPSocketBinding"))
			//ctx(NULL),
		{
			socket = NULL;
		}
}