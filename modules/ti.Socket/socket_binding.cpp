/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008-2010 Appcelerator, Inc. All Rights Reserved.
 */
#include <sstream>

#include "socket_binding.h"
//#include "protocols/tcp/tcp_socket_binding.h"

namespace ti
{
	SocketBinding::SocketBinding(Host* host) :
		KAccessorObject("Socket"),
		host(host),
		global(host->GetGlobalObject())
	{
		this->SetMethod("createTCPSocket",&SocketBinding::_CreateTCPSocket);
		this->SetMethod("updateToSecureTCPSocket",&SocketBinding::_UpdateToSecureTCPSocket);
	}

	SocketBinding::~SocketBinding()
	{
	}

	void SocketBinding::_CreateTCPSocket(const ValueList& args, KValueRef result)
	{
		args.VerifyException("createTCPSocket", "sn");
		//result->SetObject(new TCPSocketBinding(host,
		//	args.GetString(0), args.GetInt(1)));
	}

	void SocketBinding::_UpdateToSecureTCPSocket(const ValueList& args, KValueRef result)
	{
//#ifndef OS_OSX
//		TCPSocketBinding * socket = args.GetObject(0).cast<TCPSocketBinding>();
//		SecureTCPSocketBinding::startTLS(socket);
//#endif
	}

	Host* SocketBinding::GetHost()
	{
		return this->host;
	}
}
