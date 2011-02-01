/**
* @author: Mital Vora <mital.d.vora@gmail.com>
*/

#include "ssl_tcp_socket_binding.h"

namespace ti
{
	SecureTCPSocket::SecureTCPSocket(Host *host, TCPSocketBinding * tcp_socket_binding)
		: Socket<boost::asio::ssl::stream<tcp::socket&> >(host, string("Socket.SecureTCPSocket")),
		ctx(*SocketService::getIOService(), boost::asio::ssl::context::sslv23),
		tcp_socket(NULL)
	{
		if(!tcp_socket_binding)
		{
			// TODO: throw errors
			return;
		}

		tcp_socket = tcp_socket_binding->resetSocket();
		this->copyHandlers(tcp_socket_binding);

		try
		{
			ctx.set_verify_mode(boost::asio::ssl::context::verify_none);
			ctx.use_certificate_file(FileUtils::Join(
				host->GetApplication()->getRuntimePath().c_str(),
				"rootcert.pem", 0), boost::asio::ssl::context::pem);
			socket = new boost::asio::ssl::stream<tcp::socket&>(*tcp_socket, ctx);
		}
		catch (std::exception &e)
		{
			string error("SecureTCPSocket::: caught exception.");
			error += e.what();
			GetLogger()->Error(error);
		}
		this->SetMethod("onHandshake",&SecureTCPSocket::SetOnHandshake);
		this->SetMethod("async_handshake",&SecureTCPSocket::async_handshake);

	}
	SecureTCPSocket::~SecureTCPSocket()
	{
		this->CompleteClose();
		if (this->tcp_socket)
		{
			delete this->tcp_socket;
			this->tcp_socket = NULL;
		}
	}

	void SecureTCPSocket::SetOnHandshake(const ValueList& args, KValueRef result)
	{
		this->onHandshake = args.at(0)->ToMethod();
	}

	void SecureTCPSocket::async_handshake(const ValueList& args, KValueRef result)
	{
		this->registerAsyncHandshake();
	}

	void SecureTCPSocket::registerAsyncHandshake()
	{
		this->sock_state = SOCK_HANDSHAKE_IN_PROGRESS;
		socket->async_handshake(boost::asio::ssl::stream_base::client,
			boost::bind(&SecureTCPSocket::handleAsyncHandshake,
			this, boost::asio::placeholders::error));
	}

	void SecureTCPSocket::handleAsyncHandshake(const boost::system::error_code& error)
	{
		if (error)
		{
			if (error == boost::asio::error::operation_aborted)
			{
				this->sock_state = SOCK_CONNECTED;
				GetLogger()->Warn("SecureTCPSocket::handleAsyncHandshake: operation aborted.");
				return;
			}
			this->on_error(error.message());
			return;
		}
		this->sock_state = SOCK_CONNECTED;
		this->on_handshake();
		this->registerHandleRead();
	}

	void SecureTCPSocket::on_handshake()
	{
		if(!this->onHandshake.isNull())
		{
			RunOnMainThread(this->onHandshake);
			return;
		}
		GetLogger()->Warn("SecureTCPSocket::onHandshakeComplete: callback not registererd.");
	}
}
