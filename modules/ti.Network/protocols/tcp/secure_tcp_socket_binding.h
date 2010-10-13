/**
 * Class SecureTCPSocketBinding
 * @author: Mital Vora <mital.d.vora@gmail.com>
 */

#ifndef _TCP_SECURE_SOCKET_BINDING_H_
#define _TCP_SECURE_SOCKET_BINDING_H_

#include "tcp_socket_binding.h"

using namespace Poco;
using namespace Poco::Net;

namespace ti
{
	class SecureTCPSocketBinding : public StaticBoundObject
	{
	public:
		SecureTCPSocketBinding(TCPSocketBinding * socket);
		virtual ~SecureTCPSocketBinding();
	private:
		static kroll::Logger* GetLogger()
		{
			return kroll::Logger::Get("Network.SecureTCPSocketBinding");
		}

		void startTLS(TCPSocketBinding * socket);
		std::string getCertificateFilePath() const;
	};
}

#endif
