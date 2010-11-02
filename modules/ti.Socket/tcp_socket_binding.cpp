/**
 * @author: Mital Vora <mital.d.vora@gmail.com>
 */

#include "tcp_socket_binding.h"

namespace ti
{
	TCPSocketBinding::TCPSocketBinding(Host* host, const std::string& hostname, const std::string& port) :
		StaticBoundObject("Network.TCPSocket"),
		ti_host(host),
		socket(hostname, port, this),
		onConnect(0),
		onRead(0),
		onError(0),
		onClose(0)
	{
		this->SetMethod("connect",&TCPSocketBinding::Connect);
		this->SetMethod("connectNB",&TCPSocketBinding::ConnectNB);

		this->SetMethod("read",&TCPSocketBinding::Read);
		this->SetMethod("write",&TCPSocketBinding::Write);

		this->SetMethod("isClosed",&TCPSocketBinding::IsClosed);
		this->SetMethod("close",&TCPSocketBinding::Close);

		this->SetMethod("onConnect",&TCPSocketBinding::SetOnConnect);
		this->SetMethod("onRead",&TCPSocketBinding::SetOnRead);
		this->SetMethod("onError",&TCPSocketBinding::SetOnError);
		this->SetMethod("onClose",&TCPSocketBinding::SetOnClose);

		// Enables/disables keepalives.
		this->SetMethod("setDisconnectionNotifications", &TCPSocketBinding::SetKeepAlives);

		// Sets the timeouts for the keepalive times, should be called before connecting the socket.
		this->SetMethod("setDisconnectionNotificationTime", &TCPSocketBinding::SetKeepAliveTimes);
	}

	TCPSocketBinding::~TCPSocketBinding()
	{
	}

	void TCPSocketBinding::SetOnConnect(const ValueList& args, KValueRef result)
	{
		this->onConnect = args.at(0)->ToMethod();
	}

	void TCPSocketBinding::SetOnRead(const ValueList& args, KValueRef result)
	{
		this->onRead = args.at(0)->ToMethod();
	}

	void TCPSocketBinding::SetOnError(const ValueList& args, KValueRef result)
	{
		this->onError = args.at(0)->ToMethod();
	}

	void TCPSocketBinding::SetOnClose(const ValueList& args, KValueRef result)
	{
		this->onClose = args.at(0)->ToMethod();
	}

	void TCPSocketBinding::IsClosed(const ValueList& args, KValueRef result)
	{
		return result->SetBool(socket.isClosed());
	}

	void TCPSocketBinding::SetKeepAlives(const ValueList& args, KValueRef result)
	{
		// TODO: verify args
		socket.setKeepAlive(args.at(0)->ToBool());
	}

	void TCPSocketBinding::SetKeepAliveTimes(const ValueList& args, KValueRef result) 
	{
		// TODO:
		//if(this->sock_state != SOCK_CLOSED)
		//	throw ValueException::FromString("You can only set the keep-alive times before connecting the socket");

		if(args.size() > 1 && args.at(0)->IsInt() && args.at(1)->IsInt()) 
		{
			//int inactivetime = args.at(0)->ToInt();
			//int resendtime = args.at(1)->ToInt();
			//socket.setKeepAliveTimes(inactivetime, resendtime);
		}
		else 
		{
			throw ValueException::FromString("usage: setDisconnectionNotificationTime(int firstCheck, int subsequentChecks) -> If 9 subsequent checks fail the connection is considered lost");
		}
	}

	void TCPSocketBinding::Connect(const ValueList& args, KValueRef result)
	{
		try
		{
			long timeout = (args.size() > 0 && args.at(0)->IsInt()) ? args.at(0)->ToInt() : 10;
			GetLogger()->Debug("Connecting Blocking.");
			socket.connect(timeout);
		}
		catch(SocketException & e)
		{
			throw ValueException::FromString(e.what());
		}
	}


	void TCPSocketBinding::ConnectNB(const ValueList& args, KValueRef result)
	{
		try
		{
			GetLogger()->Debug("Connecting non Blocking.");
			socket.connectNB();
		}
		catch(SocketException &e)
		{
			throw ValueException::FromString(e.what());
		}
	}

	void TCPSocketBinding::Write(const ValueList& args, KValueRef result)
	{
		try
		{
			std::string data = args.at(0)->ToString();
			result->SetBool(socket.write(data));
		}
		catch(SocketException &e)
		{
			throw ValueException::FromString(e.what());
		}
	}

	void TCPSocketBinding::Read(const ValueList& args, KValueRef result)
	{
		try
		{
			std::string data = socket.read();
			BytesRef bytes(new Bytes(data.c_str(), data.size()));
			result->SetValue(Value::NewObject(bytes));
		}
		catch(SocketException &e)
		{
			throw ValueException::FromString(e.what());
		}
	}

	void TCPSocketBinding::Close(const ValueList& args, KValueRef result)
	{
		result->SetBool(socket.close());
	}
}
