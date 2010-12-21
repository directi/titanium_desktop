/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#include "javascript_module.h"
#include "javascript_methods.h"
#include <boost/filesystem.hpp>

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

	bool hasjsSuffix(const std::string& path)
	{
		if (path.length() > jsSuffix.length())
		{
			return (path.substr(path.length() - jsSuffix.length()) == jsSuffix);
		}
		return false;
	}

	bool JavaScriptModule::IsModule(const std::string& path) const
	{
		return hasjsSuffix(path);
	}

	Module* JavaScriptModule::CreateModule(const std::string& path)
	{
		const std::string basename = boost::filesystem::basename (path);
		const std::string name = basename.substr(0,basename.length()-jsSuffix.length()+3);
		boost::filesystem::path abspath(path);
		const std::string moduledir = abspath.branch_path().string();

		Logger *logger = Logger::Get("JavaScript");
		logger->Info("Loading JS path=%s", path.c_str());

		JavaScriptModuleInstance* instance = new JavaScriptModuleInstance(this->host, path, moduledir, name);
		return instance;
	}
}
