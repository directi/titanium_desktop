/**
 * Class SecureTCPSocketBinding
 * @author: Mital Vora <mital.d.vora@gmail.com>
 */

#ifndef _TCP_SECURE_SOCKET_BINDING_H_
#define _TCP_SECURE_SOCKET_BINDING_H_

#include "tcp_socket_binding.h"

#ifndef OS_OSX

using namespace Poco;
using namespace Poco::Net;

namespace ti
{
	class SecureTCPSocketBinding
	{
	public:
		static void startTLS(TCPSocketBinding * socket);
	};
}

#endif

#endif
