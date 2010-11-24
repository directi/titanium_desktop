/**
 * Appcelerator Kroll - licensed under the Apache Public License 2  
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#include <string>
#include <vector>
#include <sstream>

using std::string;
using std::vector;
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

#include "proxy_config.h"


#include <kroll/utils/file_utils.h>
#include <kroll/utils/url/ParsedURL.h>
#include <kroll/utils/environment_utils.h>

namespace kroll
{

static std::string& ProxyTypeToString(ProxyType type)
{
	static std::string httpString("http");
	static std::string httpsString("https");
	static std::string ftpString("ftp");
	static std::string socksString("socks");
	switch (type)
	{
		case HTTP:
			return httpString;
			break;
		case HTTPS:
			return httpsString;
			break;
		case FTP:
			return ftpString;
			break;
		case SOCKS:
		default:
			return socksString;
			break;
	}
}

/*static*/
ProxyType Proxy::SchemeToProxyType(string scheme)
{
	scheme = FileUtils::Trim(scheme);
	boost::to_lower(scheme);

	if (scheme == "https")
		return HTTPS;
	else if (scheme == "ftp")
		return FTP;
	else if (scheme == "socks")
		return SOCKS;
	else
		return HTTP;
}

std::string Proxy::ToString()
{
	std::stringstream ss;
	ss << ProxyTypeToString(type) << "://";

	if (!username.empty() || !password.empty())
		ss << username << ":" << password << "@";

	ss << host;

	if (port != 0)
		ss << ":" << port;

	return ss.str();
}

static SharedProxy GetProxyFromEnvironment(std::string prefix)
{
	boost::to_upper(prefix);
	std::string envName = prefix + "_PROXY";

	std::string proxyString(EnvironmentUtils::Get(envName));
	if (!proxyString.empty())
		return ProxyConfig::ParseProxyEntry(proxyString, prefix, std::string());

	boost::to_lower(prefix);
	envName = prefix + "_proxy";
	proxyString = EnvironmentUtils::Get(envName);
	if (!proxyString.empty())
		return ProxyConfig::ParseProxyEntry(proxyString, prefix, std::string());

	return 0;
}

namespace ProxyConfig
{

SharedProxy httpProxyOverride(0);
SharedProxy httpsProxyOverride(0);

void SetHTTPProxyOverride(SharedProxy newProxyOverride)
{
	httpProxyOverride = newProxyOverride;
}

SharedProxy GetHTTPProxyOverride()
{
	return httpProxyOverride;
}

void SetHTTPSProxyOverride(SharedProxy newProxyOverride)
{
	httpsProxyOverride = newProxyOverride;
}

SharedProxy GetHTTPSProxyOverride()
{
	return httpsProxyOverride;
}

SharedProxy GetProxyForURL(string& url)
{
	static Logger* logger = GetLogger();
	WTF::ParsedURL uri(url);

	// Don't try to detect proxy settings for URLs we know are local
	std::string scheme(uri.scheme());
	if (scheme == "app" || scheme == "ti" || scheme == "file")
		return 0;

	if (scheme == "http" && !httpProxyOverride.isNull())
		return httpProxyOverride;

	if (scheme == "https" && !httpsProxyOverride.isNull())
		return httpsProxyOverride;

	SharedProxy environmentProxy(GetProxyFromEnvironment(scheme));
	if (!environmentProxy.isNull())
	{
		logger->Debug("Found proxy (%s) in environment",
			environmentProxy->ToString().c_str());
		return environmentProxy;
	}

	logger->Debug("Looking up proxy information for: %s", url.c_str());
	SharedProxy proxy(ProxyConfig::GetProxyForURLImpl(url));

	if (proxy.isNull())
		logger->Debug("Using direct connection.");
	else
		logger->Debug("Using proxy: %s", proxy->ToString().c_str());

	return proxy;
}

static inline bool EndsWith(string haystack, string needle)
{
	return haystack.find(needle) == (haystack.size() - needle.size());
}

static bool ShouldBypassWithEntry(const std::string & url, SharedPtr<BypassEntry> entry)
{
	WTF::ParsedURL uri(url);
	const std::string& uriHost = uri.host();
	const std::string& uriScheme = uri.scheme();
	const std::string& uriPort = uri.port();
	int uri_port = ::atoi(uriPort.c_str());
	const std::string& entryHost = entry->host;
	const std::string& entryScheme = entry->scheme;
	unsigned short entryPort = entry->port;

	GetLogger()->Debug("bypass entry: scheme='%s' host='%s' port='%i'", 
		entry->scheme.c_str(), entry->host.c_str(), entry->port);

	// An empty bypass entry equals an unconditional bypass.
	if (entry.isNull())
	{
		return true;
	}
	else
	{
		if (entryHost == "<local>" && uriHost.find(".") == string::npos)
		{
			return true;
		}
		else if (EndsWith(uriHost, entryHost) &&
			(entryScheme.empty() || entryScheme == uriScheme) &&
			(entryPort == 0 || entryPort == uri_port))
		{
			return true;
		}
	}

	return false;
}

bool ShouldBypass(const std::string & url, vector<SharedPtr<BypassEntry> >& bypassList)
{
	GetLogger()->Debug("Checking whether %s should be bypassed.", 
		url.c_str());

	for (size_t i = 0; i < bypassList.size(); i++)
	{
		if (ShouldBypassWithEntry(url, bypassList.at(i)))
			return true;
	}

	GetLogger()->Debug("No bypass");
	return false;
}

SharedPtr<BypassEntry> ParseBypassEntry(string entry)
{
	// Traditionally an endswith comparison is always done with the host
	// part, so we throw away explicit wildcards at the beginning. If the
	// entire string is a wildcard this is an unconditional bypass.
	if (entry.at(0) == '*' && entry.size() == 1)
	{
		// Null URI means always bypass
		return 0;
	}
	else if (entry.at(0) == '*' && entry.size() > 1)
	{
		entry = entry.substr(1);
	}

	SharedPtr<BypassEntry> bypass(new BypassEntry());
	size_t endScheme = entry.find("://");
	if (endScheme != string::npos)
	{
		bypass->scheme = entry.substr(0, endScheme);
		entry = entry.substr(endScheme + 3);
	}

	size_t scan = entry.size() - 1;
	while (scan > 0 && isdigit(entry[scan]))
		scan--;

	if (entry[scan] == ':' && scan != entry.size() - 1)
	{
		string portString = entry.substr(scan + 1);
		bypass->port = atoi(portString.c_str());
		entry = entry.substr(0, scan);
	}

	bypass->host = entry;
	return bypass;
}

Logger* GetLogger()
{
	static Logger* logger = Logger::Get("Proxy");
	return logger;
}

SharedProxy ParseProxyEntry(string entry, const string& urlScheme,
	const string& entryScheme)
{
	// If the hostname came with a scheme:// speicifier, read this,
	// though it has lower precedence than the other two places the
	// scheme can be defined.
	entry = FileUtils::Trim(entry);
	size_t schemeEnd = entry.find("://");
	std::string hostScheme;
	if (schemeEnd != string::npos)
	{
		hostScheme = entry.substr(0, schemeEnd);
		entry = entry.substr(schemeEnd + 3);
	}

	// We need to pull out the credentials before the port, because
	// the port just searches for the first ':', which can be in the
	// credentials section.
	string username, password;
	size_t credentialsEnd = entry.find('@');
	if (credentialsEnd != string::npos && credentialsEnd > 0 && entry.size() > 1)
	{
		username = entry.substr(0, credentialsEnd);
		entry = entry.substr(credentialsEnd + 1);

		size_t usernameEnd = username.find(':');
		if (usernameEnd != string::npos)
		{
			password = username.substr(usernameEnd + 1);
			username = username.substr(0, usernameEnd);
		}
	}

	size_t portStart = entry.find(':');
	unsigned port = 0;
	if (portStart != string::npos && portStart != entry.size())
	{
		std::string portString(entry.substr(portStart + 1));
		entry = entry.substr(0, portStart);
		try
		{
			portString = FileUtils::Trim(portString);
			port = boost::lexical_cast<int>(portString.c_str());
		}
		catch (const boost::bad_lexical_cast& e)
		{
			port = 0;
		}
	}

	std::string scheme;
	if (!entryScheme.empty())
		scheme = entryScheme;
	else if (!hostScheme.empty())
		scheme = hostScheme;
	else
		scheme = urlScheme;

	if (scheme == "direct")
		return 0;

	SharedProxy proxy = new Proxy();
	proxy->type = Proxy::SchemeToProxyType(scheme);
	proxy->host = FileUtils::Trim(entry);
	proxy->port = port;

	if (!username.empty())
		proxy->username = username;
	if (!password.empty())
		proxy->password = password;

	return proxy;
}
} // namespace ProxyConfig
} // namespace kroll
