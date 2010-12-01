/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#include "ui_module.h"
#include <kroll/utils/url/ParsedURL.h>

namespace ti
{
	KROLL_MODULE(UIModule, STRING(MODULE_NAME), STRING(MODULE_VERSION))
	UIModule* UIModule::instance_ = 0;

	void UIModule::Initialize()
	{
		// We are keeping this object in a static variable, which means
		// that we should only ever have one copy of the UI module.
		UIModule::instance_ = this;
	}

	void UIModule::Start()
	{
#ifdef OS_WIN32
		this->uiBinding = new Win32UIBinding(this, host);
#elif OS_OSX
		this->uiBinding = new OSXUIBinding(host);
#elif OS_LINUX
		this->uiBinding = new GtkUIBinding(host);
#endif
		host->GetGlobalObject()->SetObject("UI", this->uiBinding);
		host->GetGlobalObject()->SetObject("Notification", this->uiBinding);

		AppConfig* config = AppConfig::Instance();
		if (!config)
		{
			std::string msg = "Error loading tiapp.xml. Your application "
				"is not properly configured or packaged.";
			this->uiBinding->ErrorDialog(msg);
			throw ValueException::FromString(msg.c_str());
		}

		// If there is no main window configuration, this just
		// AppConfig::GetMainWindow returns a default configuration.
		this->uiBinding->CreateMainWindow(config->GetMainWindow());
	}

	void UIModule::Stop()
	{
		if(this->uiBinding)
		{
			delete this->uiBinding;
			this->uiBinding = NULL;
		}
	}
}
