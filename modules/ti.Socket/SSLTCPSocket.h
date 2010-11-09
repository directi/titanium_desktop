/**
* @author: Mital Vora <mital.d.vora@gmail.com>
*/

#ifndef _SSL_TCP_SOCKET_H_
#define _SSL_TCP_SOCKET_H_


#include "Socket.h"
#include <asio/ssl.hpp>


class SSLTCPSocket
{
public:
	SSLTCPSocket(const std::string & hostname,
		const std::string& port)
		//ssl_socket(NULL),
		//ctx(NULL),
	{
	}

	virtual ~SSLTCPSocket() {}

private:

	asio::ssl::stream<tcp::socket> *ssl_socket;
	//asio::ssl::context *ctx;
	//ctx.set_verify_mode(boost::asio::ssl::context::verify_peer);
};

#endif // _SSL_TCP_SOCKET_H_
