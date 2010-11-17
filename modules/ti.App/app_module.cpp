/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#include "app_module.h"
#include "app_config.h"
#include "app_binding.h"
#include <Poco/File.h>

namespace ti
{
	KROLL_MODULE(AppModule, STRING(MODULE_NAME), STRING(MODULE_VERSION));

	static Logger* GetLogger()
	{
		return Logger::Get("App");
	}

	void AppModule::Initialize()
	{
		// Nudge the creation of the first app config instance.
		AppConfig* config = AppConfig::Instance();
		if (config)
			GetLogger()->Debug("Loaded config file successfully");

		KObjectRef global(host->GetGlobalObject());
		KObjectRef binding = new AppBinding(host, host->GetGlobalObject());
		host->GetGlobalObject()->SetObject("App", binding);

		// Create the data directory for the app, if it doesn't exist.
		std::string dataPath(host->GetApplication()->GetDataPath());
		Poco::File dataPathFile(dataPath);
		if (!dataPathFile.exists())
			dataPathFile.createDirectories();
	}

	void AppModule::Stop()
	{
		host->GetGlobalObject()->SetNull("App");
	}
}
