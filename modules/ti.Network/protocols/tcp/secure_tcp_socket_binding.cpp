/**
 * Class SecureTCPSocketBinding
 * @author: Mital Vora <mital.d.vora@gmail.com>
 */
#ifndef OS_OSX
#include "secure_tcp_socket_binding.h"
#include <kroll/kroll.h>
#include <Poco/Net/Context.h>
#include <Poco/Net/SSLException.h>


using Poco::Net::Context;

#define BUFFER_SIZE 1024   // choose a reasonable size to send back to JS

namespace ti
{
	void SecureTCPSocketBinding::startTLS(TCPSocketBinding * socket)
	{
		std::string privateKeyFile("");
		std::string certificateFile = "C:\\rootcert.pem";//getCertificateFilePath();
		std::string caLocation = "";
		bool loadDefaultCAs = true;

		Poco::Net::Context *ctx = new Poco::Net::Context(
			Context::CLIENT_USE, privateKeyFile, certificateFile,
			caLocation,	Context::VERIFY_NONE, 9, loadDefaultCAs);

		TCPSocketBinding::removeSocket(socket);
		StreamSocket * old_socket = socket->getSocketObject();
		//SecureStreamSocket secureSocket_ref = SecureStreamSocket::attach(*old_socket, ctx);
		//SecureStreamSocket *secureSocket = new SecureStreamSocket(secureSocket_ref.impl());

		SecureStreamSocket *secureSocket = NULL;
		try
		{
			secureSocket = new SecureStreamSocket(SecureStreamSocket::attach(*old_socket, ctx));
		}
		catch (SSLException & e)
		{
			throw;
		}


		socket->setSocketObject(secureSocket);
		delete old_socket;

		// ReRegisterAllObservers
		TCPSocketBinding::addSocket(socket);
	}
}
#endif
