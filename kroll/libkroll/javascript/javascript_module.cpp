/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#include "javascript_module.h"
#include "javascript_methods.h"

namespace kroll
{
	JavaScriptModule* JavaScriptModule::instance = NULL;
	void JavaScriptModule::Initialize()
	{
		JavaScriptModule::instance = this;
		host->AddModuleProvider(this);

		KObjectRef global(Host::GetInstance()->GetGlobalObject());
		JavaScriptMethods::Bind(global);
	}

	void JavaScriptModule::Stop()
	{
		JavaScriptModule::instance = NULL;
	}

	const static std::string jsSuffix = "module.js";
	bool JavaScriptModule::IsModule(const std::string& path) const
	{
		int plength = path.length();
		int slength = jsSuffix.length();
		if (path.length() > jsSuffix.length())
		{
			return (path.substr(plength - slength) == jsSuffix);
		}
		else
		{
			return false;
		}
	}
	std::string strip_path(const std::string &path)
	{
		std::string::size_type pos = path.rfind('/');
		std::string::size_type pos1 = path.rfind('\\');
		if (pos != std::string::npos || pos1 != std::string::npos)
		{
			if (pos != std::string::npos && pos1 != std::string::npos)
			{
				if (pos > pos1)
				{
					return path.substr(pos+1);
				}
				else
				{
					return path.substr(pos1+1);
				}
			}
			else if (pos != std::string::npos)
				return path.substr(pos+1);
			return path.substr(pos1+1);
		}
		return path;
	}

	std::string strip_extension(const std::string &path)
	{
		std::string::size_type pos = path.rfind('.');
		if (pos != std::string::npos)
			return path.substr(0, pos);
		else
			return path;
	}

	std::string getbasename(const std::string &path)
	{
		std::string basename;

		return strip_path(strip_extension(path));
	}


	Module* JavaScriptModule::CreateModule(const std::string& path)
	{
		const std::string basename = getbasename(path);
		const std::string name = basename.substr(0,basename.length()-jsSuffix.length()+3);
		const std::string moduledir = path.substr(0,path.length()-basename.length()-3);

		Logger *logger = Logger::Get("JavaScript");
		logger->Info("Loading JS path=%s", path.c_str());

		JavaScriptModuleInstance* instance = new JavaScriptModuleInstance(this->host, path, moduledir, name);
		return instance;
	}
}
