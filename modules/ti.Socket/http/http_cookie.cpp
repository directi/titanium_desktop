/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#include "http_cookie.h"

namespace ti
{
	HTTPCookie::HTTPCookie(const std::map<std::string, std::string> & cookieParts) : 
		KAccessorObject("Socket.HTTPCookie"),
		version(0),
		secure(false),
		httpOnly(false)
	{
		this->parseCookieParts(cookieParts);
		this->InitializeBinding();
	}

	void HTTPCookie::parseCookieParts(const std::map<std::string, std::string> & cookieParts)
	{
		for (std::map<std::string, std::string>::const_iterator
			it = cookieParts.begin(); it != cookieParts.end(); ++it)
		{
			const std::string& name  = it->first;
			const std::string& value = it->second;
			if (name == "comment")
			{
				comment = value;
			}
			else if (name == "domain")
			{
				domain = value;
			}
			else if (name, "path")
			{
				path =value;
			}
			else if (name, "max-age")
			{
				expires = value;
			}
			else if (name, "secure")
			{
				secure = true;
			}
			else if (name, "expires")
			{
				expires = value;
			}
			else if (name, "version")
			{
					version = atoi(value.c_str());
			}
			else if (name, "HttpOnly")
			{
				httpOnly = true;
			}
			else
			{
				this->name  = name;
				this->value = value;
			}
		}

	}

	void HTTPCookie::InitializeBinding()
	{
		/**
		 * @tiapi(method=True,name=Network.HTTPCookie.getName,since=0.7)
		 * @tiapi Get the cookie name
		 * @tiresult[String] cookie name
		 */
		this->SetMethod("getName", &HTTPCookie::GetName);

		/**
		 * @tiapi(method=True,name=Network.HTTPCookie.setName,since=0.7)
		 * @tiapi Set the cookie name
		 * @tiarg[String,name] name of the cookie
		 */
		this->SetMethod("setName", &HTTPCookie::SetName);

		/**
		 * @tiapi(method=True,name=Network.HTTPCookie.getValue,since=0.7)
		 * @tiapi Get the cookie value
		 * @tiresult[String] cookie value
		 */
		this->SetMethod("getValue", &HTTPCookie::GetValue);

		/**
		 * @tiapi(method=True,name=Network.HTTPCookie.setValue,since=0.7)
		 * @tiapi Set the cookie value
		 * @tiarg[String,value] value to set cookie
		 */
		this->SetMethod("setValue", &HTTPCookie::SetValue);

		/**
		 * @tiapi(method=True,name=Network.HTTPCookie.getVersion,since=0.7)
		 * @tiapi Identifies to which version of the state management specification the cookie conforms. 0 = netscape 1 = RFC2109
		 * @tiresult[Integer] cookie version number (0 or 1)
		 */
		this->SetMethod("getVersion", &HTTPCookie::GetVersion);

		/**
		 * @tiapi(method=True,name=Network.HTTPCookie.setVersion,since=0.7)
		 * @tiapi Set the state management specifiction version the cookie conforms. (Default: 0)
		 * @tiarg[Integer,version] cookie version (0 or 1)
		 */
		this->SetMethod("setVersion", &HTTPCookie::SetVersion);

		/**
		 * @tiapi(method=True,name=Network.HTTPCookie.getDomain,since=0.7)
		 * @tiapi Get the domain for which the cookie is valid
		 * @tiresult[String] the domain
		 */
		this->SetMethod("getDomain", &HTTPCookie::GetDomain);

		/**
		 * @tiapi(method=True,name=Network.HTTPCookie.setDomain,since=0.7)
		 * @tiapi Set the domain for which the cookie is valid
		 * @tiarg[String,domain] the domain to set
		 */
		this->SetMethod("setDomain", &HTTPCookie::SetDomain);

		/**
		 * @tiapi(method=True,name=Network.HTTPCookie.getPath,since=0.7)
		 * @tiapi Get the subset of URLs to which this cookie applies
		 * @tiresult[String] the path
		 */
		this->SetMethod("getPath", &HTTPCookie::GetPath);

		/**
		 * @tiapi(method=True,name=Network.HTTPCookie.setPath,since=0.7)
		 * @tiapi Set the subset of URLs to which this cookie applies
		 * @tiarg[String,path] the path to set
 		 */
		this->SetMethod("setPath", &HTTPCookie::SetPath);

		/**
		 * @tiapi(method=True,name=Network.HTTPCookie.getExpires,since=0.7)
		 * @tiapi Get the lifetime of the cookie, in seconds.
		 * @tiresult[Integer] lifetime in seconds. 0 = discard, -1 = never expire
		 */
		this->SetMethod("getExpires", &HTTPCookie::GetExpires);

		/**
		 * @tiapi(method=True,name=Network.HTTPCookie.SetExpires,since=0.7)
		 * @tiapi Set the lifetime of the cookie, in seconds.
		 * @tiarg[Integer,lifetime] the lifetime in seconds. 0 = discard, -1 = never expire
		 */
		this->SetMethod("setExpires", &HTTPCookie::SetExpires);

		/**
		 * @tiapi(method=True,name=Network.HTTPCookie.getComment,since=0.7)
		 * @tiapi Get the cookie comment text
		 * @tiresult[String] comment text
		 */
		this->SetMethod("getComment", &HTTPCookie::GetComment);

		/**
		 * @tiapi(method=True,name=Network.HTTPCookie.setComment,since=0.7)
		 * @tiapi Set the cookie comment text
		 * @tiarg[String,comment] text to set as comment
 		 */
		this->SetMethod("setComment", &HTTPCookie::SetComment);

		/**
		 * @tiapi(method=True,name=Network.HTTPCookie.isHTTPOnly,since=0.7)
		 * @tiapi Check if the http only flag is set on the cookie
		 * @tiresult[Boolean] return True if http only flag is set
 		 */
		this->SetMethod("isHTTPOnly", &HTTPCookie::IsHTTPOnly);

		/**
		 * @tiapi(method=True,name=Network.HTTPCookie.setHTTPOnly,since=0.7)
		 * @tiapi Set the http only flag on the cookie
		 * @tiarg[Boolean,enableHTTPOnly] if True sets the http only flag
		 */
		this->SetMethod("setHTTPOnly", &HTTPCookie::SetHTTPOnly);

		/**
		 * @tiapi(method=True,name=Network.HTTPCookie.isSecure,since=0.7)
		 * @tiapi Check if the secure flag is set on the cookie
		 * @tiresult[Boolean] return True if cookie is secure
		 */
		this->SetMethod("isSecure", &HTTPCookie::IsSecure);

		/**
		 * @tiapi(method=True,name=Network.HTTPCookie.setSecure,since=0.7)
		 * @tiapi Set the secure flag on the cookie
		 * @tiarg[Boolean,enableSecure] if True makes the cookie secure
 		 */
		this->SetMethod("setSecure", &HTTPCookie::SetSecure);

		/**
		 * @tiapi(method=True,name=Network.HTTPCookie.toString,since=0.7)
		 * @tiapi return a string representation of the cookie
		 * @tiresult[String] cookie representation as a string (name=value;...)
 		 */
		this->SetMethod("toString", &HTTPCookie::ToString);
	}

	void HTTPCookie::GetName(const ValueList& args, KValueRef result)
	{
		result->SetString(this->getName().c_str());
	}

	void HTTPCookie::SetName(const ValueList& args, KValueRef result)
	{
		args.VerifyException("setName", "s");
		this->name = args.GetString(0);
	}

	void HTTPCookie::GetValue(const ValueList& args, KValueRef result)
	{
		result->SetString(this->value.c_str());
	}

	void HTTPCookie::SetValue(const ValueList& args, KValueRef result)
	{
		args.VerifyException("setValue", "s");
		this->value = args.GetString(0);
	}

	void HTTPCookie::GetVersion(const ValueList& args, KValueRef result)
	{
		result->SetInt(this->version);
	}

	void HTTPCookie::SetVersion(const ValueList& args, KValueRef result)
	{
		args.VerifyException("setVersion", "i");
		int version = args.GetInt(0);
		if (version > 1 || version < 0)
		{
			// Version is out of range, can only be 0 or 1	
			throw ValueException::FromString("HTTPCookie version invalid, must be 0 or 1");
		}
		this->version = version;
	}

	void HTTPCookie::GetDomain(const ValueList& args, KValueRef result)
	{
		result->SetString(this->domain.c_str());
	}

	void HTTPCookie::SetDomain(const ValueList& args, KValueRef result)
	{
		args.VerifyException("setDomain", "s");
		this->domain = args.GetString(0);
	}

	void HTTPCookie::GetPath(const ValueList& args, KValueRef result)
	{
		result->SetString(this->path.c_str());
	}

	void HTTPCookie::SetPath(const ValueList& args, KValueRef result)
	{
		args.VerifyException("setPath", "s");
		this->path = args.GetString(0);
	}

	void HTTPCookie::GetExpires(const ValueList& args, KValueRef result)
	{
		result->SetString(this->expires.c_str());
	}

	void HTTPCookie::SetExpires(const ValueList& args, KValueRef result)
	{
		args.VerifyException("SetExpires", "s");
		this->expires = args.GetString(0);
	}

	void HTTPCookie::GetComment(const ValueList& args, KValueRef result)
	{
		result->SetString(this->comment.c_str());
	}

	void HTTPCookie::SetComment(const ValueList& args, KValueRef result)
	{
		args.VerifyException("setComment", "s");
		this->comment = args.GetString(0);
	}

	void HTTPCookie::IsHTTPOnly(const ValueList& args, KValueRef result)
	{
		result->SetBool(this->httpOnly);
	}

	void HTTPCookie::SetHTTPOnly(const ValueList& args, KValueRef result)
	{
		args.VerifyException("setHTTPOnly", "b");
		this->httpOnly = args.GetBool(0);
	}

	void HTTPCookie::IsSecure(const ValueList& args, KValueRef result)
	{
		result->SetBool(this->secure);
	}

	void HTTPCookie::SetSecure(const ValueList& args, KValueRef result)
	{
		args.VerifyException("setSecure", "b");
		this->secure = args.GetBool(0);
	}

	void HTTPCookie::ToString(const ValueList& args, KValueRef result)
	{
		result->SetString(this->toString().c_str());
	}

	SharedString HTTPCookie::DisplayString(int levels)
	{
		SharedString cookieString = new std::string(this->toString());
		return cookieString;
	}

	std::string HTTPCookie::toString() const
	{
		std::string result;
		result.reserve(256);
		result.append(name);
		result.append("=");
		if (version == 0)
		{
			// Netscape cookie
			result.append(value);
			if (!domain.empty())
			{
				result.append("; domain=");
				result.append(domain);
			}
			if (!path.empty())
			{
				result.append("; path=");
				result.append(path);
			}
			if (!expires.empty())
			{
				result.append("; expires=");
				result.append(expires);
			}
			if (secure)
			{
				result.append("; secure");
			}
			if (httpOnly)
			{
				result.append("; HttpOnly");
			}
		}
		else
		{
			// RFC 2109 cookie
			result.append("\"");
			result.append(value);
			result.append("\"");
			if (!comment.empty())
			{
				result.append("; Comment=\"");
				result.append(comment);
				result.append("\"");
			}
			if (!domain.empty())
			{
				result.append("; Domain=\"");
				result.append(domain);
				result.append("\"");
			}
			if (!path.empty())
			{
				result.append("; Path=\"");
				result.append(path);
				result.append("\"");
			}
			if (!expires.empty())
			{
				result.append("; Max-Age=\"");
				result.append(expires);
				result.append("\"");
			}
			if (secure)
			{
				result.append("; secure");
			}
			if (httpOnly)
			{
				result.append("; HttpOnly");
			}
			result.append("; Version=\"1\"");
		}
		return result;
	}
}

