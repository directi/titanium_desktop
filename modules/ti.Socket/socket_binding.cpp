/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008-2010 Appcelerator, Inc. All Rights Reserved.
 */
#include <sstream>

#include "tcp_socket_binding.h"
#include "ssl_tcp_socket_binding.h"

#ifdef OS_WIN32
#include "http/curl_http_client_binding.h"
#endif
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

		this->SetMethod("setHTTPProxy", &SocketBinding::_SetHTTPProxy);
		this->SetMethod("setProxy", &SocketBinding::_SetHTTPProxy);
		this->SetMethod("getHTTPProxy", &SocketBinding::_GetHTTPProxy);
		this->SetMethod("getProxy", &SocketBinding::_GetHTTPProxy);
		this->SetMethod("setHTTPSProxy", &SocketBinding::_SetHTTPSProxy);
		this->SetMethod("getHTTPSProxy", &SocketBinding::_GetHTTPSProxy);

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
#ifdef OS_WIN32
		result->SetObject(new CURLHTTPClientBinding(host));
#endif
	}

	Host* SocketBinding::GetHost()
	{
		return this->host;
	}

	static SharedProxy ArgumentsToProxy(const ValueList& args, const std::string& scheme)
	{
		if (args.at(0)->IsNull())
			return 0;

		std::string entry(args.GetString(0));
		if (entry.empty())
			return 0;

		// Do not pass the third argument entryScheme, because it overrides
		// any scheme set in the proxy string.
		return ProxyConfig::ParseProxyEntry(entry, scheme, std::string());
	}

	void SocketBinding::_SetHTTPProxy(const ValueList& args, KValueRef result)
	{
		args.VerifyException("setHTTPProxy", "s|0 ?s s s");
		SharedProxy proxy(ArgumentsToProxy(args, "http"));
		ProxyConfig::SetHTTPProxyOverride(proxy);
	}

	void SocketBinding::_GetHTTPProxy(const ValueList& args, KValueRef result)
	{
		SharedProxy proxy = ProxyConfig::GetHTTPProxyOverride();

		if (proxy.isNull())
			result->SetNull();
		else
			result->SetString(proxy->ToString().c_str());
	}

	void SocketBinding::_SetHTTPSProxy(const ValueList& args, KValueRef result)
	{
		args.VerifyException("setHTTPSProxy", "s|0 ?s s s");
		SharedProxy proxy(ArgumentsToProxy(args, "https"));
		ProxyConfig::SetHTTPSProxyOverride(proxy);
	}

	void SocketBinding::_GetHTTPSProxy(const ValueList& args, KValueRef result)
	{
		SharedProxy proxy = ProxyConfig::GetHTTPSProxyOverride();

		if (proxy.isNull())
			result->SetNull();
		else
			result->SetString(proxy->ToString().c_str());
	}
}
