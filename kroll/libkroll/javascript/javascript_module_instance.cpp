/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "javascript_module.h"

#include <kroll/utils/file_utils.h>
#include <kroll/utils/environment_utils.h>

namespace kroll
{
	JavaScriptModuleInstance::JavaScriptModuleInstance(Host* host, std::string path, std::string dir, std::string name) :
		Module(host, dir.c_str(), name.c_str(), "0.1"),
		path(path),
		context(0)
	{
		this->context = KJSUtil::CreateGlobalContext();
		JSGlobalContextRetain(context);
		KJSUtil::ProtectContext(context);

		try
		{
			this->Run();
		}
		catch (ValueException& e)
		{
			SharedString ss = e.GetValue()->DisplayString();
			Logger *logger = Logger::Get("JavaScript");
			logger->Error("Could not execute %s because %s", path.c_str(), (*ss).c_str());
		}
	}

	JavaScriptModuleInstance::~JavaScriptModuleInstance() 
	{
		if(context != 0)
			Stop();
	}

	void JavaScriptModuleInstance::Stop()
	{
		KJSUtil::UnprotectContext(context, true);
		JSGlobalContextRelease(context);
		context = 0;
	}

	void JavaScriptModuleInstance::Run()
	{
		std::string code(FileUtils::ReadFile(this->path));

		// Check the script's syntax.
		JSValueRef exception;
		JSStringRef jsCode = JSStringCreateWithUTF8CString(code.c_str());
		bool syntax = JSCheckScriptSyntax(context, jsCode, NULL, 0, &exception);
		if (!syntax)
		{
			KValueRef e = KJSUtil::ToKrollValue(exception, context, NULL);
			JSStringRelease(jsCode);
			throw ValueException(e);
		}

		KJSUtil::Evaluate(context, code.c_str());
	}
}

