/**
 * Class SecureTCPSocketBinding
 * @author: Mital Vora <mital.d.vora@gmail.com>
 */
#include "secure_tcp_socket_binding.h"
#include <kroll/kroll.h>
//#include <Poco/Net/SSLManager.h>
#include <Poco/Net/Context.h>

using Poco::Net::Context;

#define BUFFER_SIZE 1024   // choose a reasonable size to send back to JS

namespace ti
{

	SecureTCPSocketBinding::SecureTCPSocketBinding(TCPSocketBinding * socket)
		: StaticBoundObject("Network.SecureTCPSocket")
	{
		startTLS(socket);
	}

	SecureTCPSocketBinding::~SecureTCPSocketBinding()
	{
	}

	std::string SecureTCPSocketBinding::getCertificateFilePath() const
	{
		std::string certificateFile("");
		return certificateFile;
	}

	void SecureTCPSocketBinding::startTLS(TCPSocketBinding * socket)
	{
		std::string privateKeyFile("");
		std::string certificateFile = "C:\\rootcert.pem";//getCertificateFilePath();
		std::string caLocation = "";
		bool loadDefaultCAs = true;

		Poco::Net::Context *ctx = new Poco::Net::Context(
			Context::CLIENT_USE, privateKeyFile, certificateFile,
			caLocation,	Context::VERIFY_RELAXED, 9, loadDefaultCAs);

		TCPSocketBinding::removeSocket(socket);
		StreamSocket * old_socket = socket->getSocketObject();
		SecureStreamSocket *secureSocket = new SecureStreamSocket(SecureStreamSocket::attach(*old_socket, ctx));

		socket->setSocketObject(secureSocket);
		delete old_socket;

		// ReRegisterAllObservers
		TCPSocketBinding::addSocket(socket);
	}
}

