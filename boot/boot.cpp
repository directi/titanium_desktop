/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008-2009 Appcelerator, Inc. All Rights Reserved.
 */
#include "boot.h"
#include <ManifestHandler.h>
#include <file_utils.h>
#include <environment_utils.h>
#include <platform_utils.h>

using namespace UTILS_NS;

KrollBoot::KrollBoot(int _argc, const char ** _argv)
: argc(_argc), argv(_argv)
{
}
KrollBoot::~KrollBoot()
{
}

void KrollBoot::ShowError(const string & msg, bool fatal) const
{
	ShowErrorImpl(msg, fatal);
	if(fatal)
	{
		exit(1);
	}
}

int KrollBoot::Bootstrap()
{
	string app_path = FileUtils::GetExecutableDirectory();

	::MessageBoxA(0, "test", "test", 0);
	if(!ManifestHandler::doesManifestFileExistsAtDirectory(app_path))
	{
		string error("Application packaging error: no manifest was found from directory : " );
		error.append(app_path);
		ShowError(error);
		return __LINE__;
	}

	// create manifest handler
	// parse manifest
	string manifest_path = ManifestHandler::getManifestPathAtDirectory(app_path);
	ManifestHandler manifestHandler(manifest_path);

	// get dependencies from manifest
	vector<SharedDependency> dependencies;
	manifestHandler.getDependencies(dependencies);

	// create dependency mananger
	// resolve dependencies
	ComponentManager componentManager(app_path, dependencies);
	// if there are unresolved dependencies --> exit
	// else --> get paths from ther set as module paths , runtime path.

	if(!componentManager.allDependenciesResolved())
	{
		return __LINE__;
	}

	// Construct a list of module pathnames for setting up library paths
	string module_paths = componentManager.getModulePaths();

	EnvironmentUtils::Set(BOOTSTRAP_ENV, "YES");
	EnvironmentUtils::Set("KR_HOME", app_path);
	EnvironmentUtils::Set("KR_RUNTIME", componentManager.getRuntimePath());
	EnvironmentUtils::Set("KR_MODULES", module_paths);

	BootstrapPlatformSpecific(componentManager.getRuntimePath(), module_paths);
	string error = Blastoff();

	// If everything goes correctly, we should never get here
	error = string("Launching application failed: ") + error;
	ShowError(error, false);
	return __LINE__;
}

#ifdef USE_BREAKPAD
	CrashHandler::CrashHandler(int _argc, const char ** _argv)
	{
		executable_name = _argv[0];
		if (_argc > 3)
		{
			string dumpId = string(_argv[3]) + ".dmp";
			dumpFilePath = FileUtils::Join(_argv[2], dumpId.c_str(), NULL);
		}
	}

	CrashHandler::~CrashHandler()
	{
	}

	string CrashHandler::applicationHome;
	string CrashHandler::dumpFilePath;
	string CrashHandler::executable_name;

	void CrashHandler::InitCrashDetection()
	{
		// Load the application manifest so that we can get lots of debugging
		// information for the crash report.
		applicationHome = FileUtils::GetExecutableDirectory();
		string manifestPath = FileUtils::Join(applicationHome.c_str(), MANIFEST_FILENAME, NULL);

		if (FileUtils::IsFile(manifestPath))
		{
			//app = Application::NewApplication(applicationHome);
		}
	}

	string CrashHandler::GetCrashDetectionTitle()
	{
		return CrashHandler::GetApplicationName() + " encountered an error";
	}

	string CrashHandler::GetCrashDetectionHeader()
	{
		return CrashHandler::GetApplicationName() + " appears to have encountered a fatal error and cannot continue.";
	}

	string CrashHandler::GetCrashDetectionMessage()
	{
		return "The application has collected information about the error"
		" in the form of a detailed error report. If you send the crash report,"
		" we will attempt to resolve this problem.";
	}

	void CrashHandler::GetCrashReportParameters(map<string, string> & params)
	{
		if (!dumpFilePath.empty())
		{
			// send all the stuff that will help us figure out 
			// what the heck is going on and why the shiiiiiiiiit is
			// crashing... probably gonna be microsoft's fault
			// at least we can blame it on them...

			// this differentiates mobile vs desktop
			params["location"] = "desktop"; 
			//params["mid"] = PlatformUtils::GetMachineId();
			//params["mac"] = PlatformUtils::GetFirstMACAddress();
			params["os"] = OS_NAME;
			params["ostype"] = OS_TYPE;
			//params["osver"] = FileUtils::GetOSVersion();
			//params["osarch"] = FileUtils::GetOSArchitecture();
			params["ver"] = PRODUCT_VERSION;
			//params["un"] = PlatformUtils::GetUsername();

			/*
			if (!app.isNull())
			{
				params["app_name"] = app->getName();
				params["app_id"] = app->getId();
				params["app_ver"] = app->getVersion();
				params["app_guid"] = app->getGUID();
				params["app_home"] = FileUtils::GetExecutableDirectory();

				vector<SharedComponent> components;
				app->GetAvailableComponents(components);
				vector<SharedComponent>::const_iterator i = components.begin();
				while (i != components.end())
				{
					SharedComponent c = (*i++);
					string type("unknown");
					if (c->type == KrollUtils::MODULE)
					{
						type = "module";
					}
					else if (c->type == KrollUtils::RUNTIME)
					{
						type = "runtime";
					}
					params[type + "_" + c->name + "_version"] = c->version;
					params[type + "_" + c->name + "_path"] = c->path;
					params[type + "_" + c->name + "_bundled"] = c->bundled ? "1":"0";
				}
			}*/
		}
	}
	string CrashHandler::GetApplicationName()
	{
		return PRODUCT_NAME;
	}

#endif

