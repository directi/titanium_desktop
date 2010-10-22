/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#include "application.h"
#include <kroll/utils/file_utils.h>

#include <sstream>
#define OVERRIDE_ARG "--bundled-component-override"

using std::string;
using std::vector;
using std::pair;

namespace UTILS_NS
{
	SharedApplication Application::NewApplication(const std::string &app_path)
	{
		string manifest_path = ManifestHandler::getManifestPathAtDirectory(app_path);
		map<string, string> manifest;
		ManifestHandler::ReadManifestFile(manifest_path, manifest);
		if (manifest.empty())
		{
			return NULL;
		}
		return new Application(app_path, manifest_path, manifest);
	}

	string Application::getImage() const
	{
		return FileUtils::Join(this->GetResourcesPath().c_str(),
			this->manifestHandler.getImage().c_str(), NULL);
	}

	std::string Application::getRuntimePath() const
	{
		return componentManager.getRuntimePath();
	}
	
	Application::Application(const std::string &path,
		const std::string &manifest_path,
		const map<string, string> &manifest)
		: path(path),
		manifestHandler(manifest_path, manifest),
		componentManager(path)
	{
		manifestHandler.getDependencies(this->dependencies);
	}

	Application::~Application()
	{
	}

	bool Application::removeModule(const string &modulePath)
	{
		return componentManager.removeModule(modulePath);
	}

	string Application::GetExecutablePath() const
	{
		// TODO:
		// If this application has arguments, it's probably the currently running
		// application, so we can try to get the executable path based on argv[0]
		string exeName(this->getName() + ".exe");
		string path(FileUtils::Join(this->path.c_str(), exeName.c_str(), NULL));
		if (FileUtils::IsFile(path))
		{
			return path;
		}

		path = FileUtils::Join(this->path.c_str(), "MacOS", this->getName().c_str(), NULL);
		if (FileUtils::IsFile(path))
		{
			return path;
		}

		path = FileUtils::Join(this->path.c_str(), this->getName().c_str(), NULL);
		if (FileUtils::IsFile(path))
		{
			return path;
		}

		return string();
	}

	string Application::GetComponentPath(const string &name) const
	{
		return componentManager.GetComponentPath(name);
	}

	string Application::GetDataPath() const
	{
		return FileUtils::GetApplicationDataDirectory(this->getId());
	}

	string Application::GetResourcesPath() const
	{
		return FileUtils::Join(this->path.c_str(), "Resources", NULL);
	}

	bool Application::IsInstalled() const
	{
		string dataDirMarker(FileUtils::Join(this->GetDataPath().c_str(),
			INSTALLED_MARKER_FILENAME, NULL));
		string appDirMarker(FileUtils::Join(this->path.c_str(),
			INSTALLED_MARKER_FILENAME, NULL));
		return FileUtils::IsFile(dataDirMarker) ||
			FileUtils::IsFile(appDirMarker);
	}


	string Application::GetLicenseText() const
	{
		string license(FileUtils::Join(this->path.c_str(),
			LICENSE_FILENAME, NULL));
		if (!FileUtils::IsFile(license))
			return "";

		return FileUtils::ReadFile(license);
	}

	bool Application::ResolveDependencies()
	{
		componentManager.resolveDependencies(this->dependencies);
		return componentManager.allDependenciesResolved();
	}

	void Application::GetAvailableComponents(vector<SharedComponent>& components, bool onlyBundled)
	{
		if (this->HasArgument(OVERRIDE_ARG))
		{
			// Only scan bundled components on the override path
			string overridePath(this->GetArgumentValue(OVERRIDE_ARG));
			BootUtils::ScanBundledComponents(overridePath, components); 
			onlyBundled = true;
		}
		else
		{
			// Merge bundled and installed components
			BootUtils::ScanBundledComponents(this->path, components); 
		}

		if (!onlyBundled)
		{
			vector<SharedComponent>& installedComponents =
				BootUtils::GetInstalledComponents(true);
			for (size_t i = 0; i < installedComponents.size(); i++)
			{
				components.push_back(installedComponents.at(i));
			}
		}
		
	}

	void Application::UsingModule(
		const std::string &name,
		const std::string &version,
		const std::string &path)
	{
		componentManager.UsingModule(name, version, path);
	}

	void Application::SetArguments(int argc, const char* argv[])
	{
		for (int i = 0; i < argc; i++)
		{
			this->arguments.push_back(argv[i]);
		}
	}

	void Application::SetArguments(const vector<string>& arguments)
	{
		std::copy(arguments.begin(), arguments.end(), this->arguments.begin());
	}

	vector<string>& Application::GetArguments()
	{
		return this->arguments;
	}

	bool Application::HasArgument(const string &needle) const
	{
		string dashNeedle(string("--") + needle);
		for(vector<string>::const_iterator 
			i = this->arguments.begin();
			i != this->arguments.end();
		i++)
		{
			string arg(*i);
			if (arg.find(needle) == 0 || arg.find(dashNeedle) == 0)
			{
				return true;
			}
		}
		return false;
	}

	string Application::GetArgumentValue(const string &needle) const
	{
		string dashNeedle(string("--") + needle);
		for(vector<string>::const_iterator 
			i = this->arguments.begin();
			i != this->arguments.end();
		i++)
		{
			string arg(*i);
			size_t start;
			if ((arg.find(needle) == 0 || arg.find(dashNeedle) == 0)
				 && (start = arg.find("=")) != string::npos)
			{
				string value(arg.substr(start + 1));
				if (value[0] == '"' && value.length() > 3)
				{
					value = value.substr(1, value.length() - 2);
				}
				return value;
			}
		}
		return string();
	}
}
