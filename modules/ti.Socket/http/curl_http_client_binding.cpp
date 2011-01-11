/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009-2010 Appcelerator, Inc. All Rights Reserved.
 */
#include <sstream>

#include "curl_http_client_binding.h"

#include <kroll/utils/url/ParsedURL.h>

#include <boost/algorithm/string.hpp>

namespace ti
{
	static Logger* GetLogger()
	{
		static Logger* logger = Logger::Get("Socket.CURLHTTPClient");
		return logger;
	}

	CURLMULTIClient CURLHTTPClientBinding::multiClient;

	CURLHTTPClientBinding::CURLHTTPClientBinding(Host* host) :
		KEventObject("Socket.HTTPClient"),
		host(host),
		readyState(HTTP_UNSENT),
		httpMethod("GET"),
		url(""),
		filename(""),
		timeout(5 * 60 * 1000),
		maxRedirects(-1),
		easy(NULL)
	{
		this->SetMethod("open", &CURLHTTPClientBinding::Open);
		this->SetMethod("onHeaderReceived", &CURLHTTPClientBinding::SetOnHeaderReceived);
		this->SetMethod("onDataChunkReceived", &CURLHTTPClientBinding::SetOnDataChunkReceived);
		this->SetMethod("saveToFile", &CURLHTTPClientBinding::saveToFile);
		this->SetMethod("getReadyState", &CURLHTTPClientBinding::getReadyState);
		this->SetMethod("getHTTPStatus", &CURLHTTPClientBinding::getHTTPStatus);
		this->SetMethod("send", &CURLHTTPClientBinding::Send);
		this->SetMethod("setRequestHeader", &CURLHTTPClientBinding::SetRequestHeader);
		this->SetMethod("setTimeout", &CURLHTTPClientBinding::SetTimeout);
		this->SetMethod("getTimeout", &CURLHTTPClientBinding::GetTimeout);
		this->SetMethod("getMaxRedirects", &CURLHTTPClientBinding::GetMaxRedirects);
		this->SetMethod("setMaxRedirects", &CURLHTTPClientBinding::SetMaxRedirects);
		this->SetMethod("getResponseHeader", &CURLHTTPClientBinding::GetResponseHeader);
		this->SetMethod("getResponseHeaders", &CURLHTTPClientBinding::GetResponseHeaders);
	}

	CURLHTTPClientBinding::~CURLHTTPClientBinding()
	{
	}

	//void CURLHTTPClientBinding::Abort(const ValueList& args, KValueRef result)
	//{
	//	this->aborted = true;
	//}
	void CURLHTTPClientBinding::ChangeState(HTTP_STATE_en _readyState)
	{
		GetLogger()->Debug("Changing readyState from %d to %d for url:%s",
			this->readyState, _readyState, this->url.c_str());
		this->FireEvent(Event::HTTP_STATE_CHANGED);

		if (readyState == HTTP_DONE)
		{
			this->FireEvent(Event::HTTP_DONE);
		}
	}

	std::string getHTTPMethod(const std::string & method)
	{
		std::string httpMethod = method;
		boost::to_upper(httpMethod);
		if (httpMethod.empty())
			httpMethod = "GET";
		return httpMethod;
	}

	std::string getURL(const std::string & url)
	{
		// TODO: If the scheme is a app:// or ti:// we should just
		// convert the URL to a file URL here.
		const std::string scheme = WTF::ParsedURL(url).scheme();
		if (scheme != "http" && scheme != "https" && scheme != "file")
		{
			throw ValueException::FromFormat("%s scheme is not supported by HTTPClient", scheme.c_str());
		}
		return url;
	}

	void CURLHTTPClientBinding::Open(const ValueList& args, KValueRef result)
	{
		args.VerifyException("open", "s s");
		if (easy)
		{
			throw ValueException::FromString("request already being processed");
		}

		this->httpMethod = getHTTPMethod(args.GetString(0));
		this->url = getURL(args.GetString(1));
		this->ChangeState(HTTP_OPENED);
		result->SetBool(true);
	}

	void CURLHTTPClientBinding::SetOnHeaderReceived(const ValueList& args, KValueRef result)
	{
		args.VerifyException("onHeaderReceived", "m");
		this->onHeaderReceived = args.at(0)->ToMethod();
	}

	void CURLHTTPClientBinding::SetOnDataChunkReceived(const ValueList& args, KValueRef result)
	{
		args.VerifyException("onDataChunkReceived", "m");
		this->onDataChunkReceived = args.at(0)->ToMethod();
	}

	void CURLHTTPClientBinding::saveToFile(const ValueList& args, KValueRef result)
	{
		args.VerifyException("saveToFile", "s");
		this->filename = args.at(0)->ToString();
	}

	void CURLHTTPClientBinding::getReadyState(const ValueList& args, KValueRef result)
	{
		if (!easy)
		{
			throw ValueException::FromString("send() the request first :P");
		}
		result->SetInt(this->readyState);
	}

	void CURLHTTPClientBinding::getHTTPStatus(const ValueList& args, KValueRef result)
	{
		if (!easy)
		{
			throw ValueException::FromString("send() the request first :P");
		}

		result->SetInt(this->easy->getHTTPStatus());
	}

	void CURLHTTPClientBinding::Send(const ValueList& args, KValueRef result)
	{
		if (this->url == "")
		{
			throw ValueException::FromString("open() the request first :P.");
		}

		// Get send data if provided
		args.VerifyException("send", "?s|0");
		std::string requestData;
		// TODO: support blob object.
		if(args.size() > 0)
		{
			requestData = args.at(0)->ToString();
		}

		ExecuteRequest(requestData);
	}

	void CURLHTTPClientBinding::ExecuteRequest(const std::string & data)
	{
		if (easy)
		{
			throw ValueException::FromString("request already being processed");
		}
		easy = new CURLEASYClient(this->url);
		easy->setOnHeaderReceived(this->onHeaderReceived);
		easy->setOnDataChunkReceived(this->onDataChunkReceived);
		easy->setRequestHeaders(this->requestHeaders);
		easy->setTimeout(this->timeout);
		easy->setMaxRedirects(this->maxRedirects);
		CURLHTTPClientBinding::multiClient.add(easy);
	}

	void CURLHTTPClientBinding::SetRequestHeader(const ValueList& args, KValueRef result)
	{
		args.VerifyException("setRequestHeader", "s s");
		std::string key(args.GetString(0));
		std::string value(args.GetString(1));

		if (key[key.size() - 1] != ':')
			key.append(": ");

		// An empty header value tells cURL to unset this header.
		if (value.empty())
			key.append("\"\"");
		else
			key.append(value);

		this->requestHeaders.push_back(key);
	}

	void CURLHTTPClientBinding::SetTimeout(const ValueList& args, KValueRef result)
	{
		args.VerifyException("setTimeout", "i");
		this->timeout = args.GetInt(0);
	}

	void CURLHTTPClientBinding::GetTimeout(const ValueList& args, KValueRef result)
	{
		result->SetInt(this->timeout);
	}

	void CURLHTTPClientBinding::GetMaxRedirects(const ValueList& args, KValueRef result)
	{
		result->SetInt(this->maxRedirects);
	}

	void CURLHTTPClientBinding::SetMaxRedirects(const ValueList& args, KValueRef result)
	{
		args.VerifyException("setMaxRedirects", "n");
		this->maxRedirects = args.GetInt(0);
	}

	void CURLHTTPClientBinding::GetResponseHeader(const ValueList& args, KValueRef result)
	{
		args.VerifyException("getResponseHeader", "s");
		const std::string name(args.GetString(0));
		if(!easy)
		{
			throw ValueException::FromFormat("no request being processed");
		}

		std::string header = easy->getResponseHeader(name);
		if (header == "")
		{
			result->SetNull();
		}
		else
		{
			result->SetString(name);
		}
	}

	void CURLHTTPClientBinding::GetResponseHeaders(const ValueList& args, KValueRef result)
	{
		if(!easy)
		{
			throw ValueException::FromFormat("no request being processed");
		}
		
		std::map<std::string, std::string> responseHeaders;
		easy->getResponseHeaders(responseHeaders);
		KListRef headers(new StaticBoundList());

		for(std::map<std::string, std::string>::const_iterator
			i = responseHeaders.begin();
			i != responseHeaders.end();
		i++)
		{
			KListRef headerEntry(new StaticBoundList());
			headerEntry->Append(Value::NewString(i->first));
			headerEntry->Append(Value::NewString(i->second));
			headers->Append(Value::NewList(headerEntry));
		}

		result->SetList(headers);
	}

}

