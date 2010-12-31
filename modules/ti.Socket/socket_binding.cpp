/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008-2010 Appcelerator, Inc. All Rights Reserved.
 */
#include <sstream>

#include "tcp_socket_binding.h"
#include "ssl_tcp_socket_binding.h"
#include "http/curl_http_client_binding.h"
#include "socket_binding.h"

namespace ti
{
	SocketBinding::SocketBinding(Host* host) :
		KAccessorObject("Socket"),
		host(host),
		global(host->GetGlobalObject())
	{
		this->SetMethod("createTCPSocket",&SocketBinding::_CreateTCPSocket);
		this->SetMethod("getSSLTCPSocket",&SocketBinding::_getSSLTCPSocket);
		this->SetMethod("getCURLHTTPClient",&SocketBinding::_getCURLHTTPClient);
	}

	SocketBinding::~SocketBinding()
	{
	}

	void SocketBinding::_CreateTCPSocket(const ValueList& args, KValueRef result)
	{
		args.VerifyException("createTCPSocket", "s s|n");
		result->SetObject(new TCPSocketBinding(host, args.GetString(0), args.GetString(1)));
	}

	void SocketBinding::_getSSLTCPSocket(const ValueList& args, KValueRef result)
	{
		TCPSocketBinding * socket = args.GetObject(0).cast<TCPSocketBinding>();
		result->SetObject(new SecureTCPSocket(host, socket));
	}

	void SocketBinding::_getCURLHTTPClient(const ValueList& args, KValueRef result)
	{
		result->SetObject(new CURLHTTPClientBinding(host));
	}

	Host* SocketBinding::GetHost()
	{
		return this->host;
	}
}
