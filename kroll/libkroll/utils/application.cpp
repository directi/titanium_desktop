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
		return Application::NewApplication(manifest_path, app_path);
	}

	SharedApplication Application::NewApplication(const std::string &manifest_path, const std::string &app_path)
	{
		map<string, string> manifest;
		ManifestHandler::ReadManifestFile(manifest_path, manifest);
		if (manifest.empty())
		{
			return NULL;
		}

		Application* application = new Application(app_path, manifest_path);
		return application;
	}

	SharedApplication Application::NewApplication(const map<string, string>& manifest)
	{
		Application* application = new Application();
		application->ParseManifest(manifest);
		return application;
	}

	string Application::getImage() const
	{
		return FileUtils::Join(this->GetResourcesPath().c_str(),
			this->manifestHandler.getImage().c_str(), NULL);
	}

	std::string Application::getRuntimePath() const
	{
		return this->runtime->path;
	}
	
	void Application::ParseManifest(const map<string, string>& manifest)
	{
		manifestHandler.parseManifest(manifest);

		map<string, string> dep;
		manifestHandler.getDependencies(dep);
		for(map<string, string>::const_iterator
			oIter = dep.begin();
			oIter != dep.end();
		oIter++)
		{
			 SharedDependency d = Dependency::NewDependencyFromManifestLine(oIter->first, oIter->second);
			 this->dependencies.push_back(d);
		}
	}

	Application::Application(const std::string &path, const std::string &manifestPath)
		: path(path),
		manifestHandler(manifestPath),
		componentManager(path)
	{
		// TODO: to be moved in component Manager
		map<string, string> dep;
		manifestHandler.getDependencies(dep);
		for(map<string, string>::const_iterator
			oIter = dep.begin();
			oIter != dep.end();
		oIter++)
		{
			 SharedDependency d = Dependency::NewDependencyFromManifestLine(oIter->first, oIter->second);
			 this->dependencies.push_back(d);
		}
	}

	Application::Application(const std::string &path)
		: path(path),
		manifestHandler(),
		componentManager(path)
	{
	}

	Application::~Application()
	{
		this->modules.clear();
		this->runtime = NULL;
	}

	void Application::getDependencies(vector<SharedDependency> &_dependencies) const
	{
		_dependencies.reserve(dependencies.size());
		std::copy(dependencies.begin(), dependencies.end(), _dependencies.begin());
	}

	void Application::getUnresolvedDependencies(vector<SharedDependency> & unresolved) const
	{
		// We cannot resolve dependencies in the normal way, since we aren't
		// installed yet. Instead, go through the dependencies and try to
		// resolve them manuallly.
		vector<SharedComponent>& components = BootUtils::GetInstalledComponents(true);
		for (size_t i = 0; i < dependencies.size(); i++)
		{
			const SharedDependency dependency(dependencies[i]);
			if (BootUtils::ResolveDependency(dependency, components).isNull())
				unresolved.push_back(dependency);
		}
	}

	void Application::getComponents(std::vector<SharedComponent> &components) const
	{
		// Do not use a reference here, because we don't want to modify the
		// application's modules list.
		this->getModules(components);

		if (!runtime.isNull())
		{
			components.push_back(runtime);
		}
	}

	void Application::getModules(vector<SharedComponent> &_modules) const
	{
		_modules.reserve(modules.size());
		std::copy(modules.begin(), modules.end(), _modules.begin());
	}

	string Application::getModulePaths() const
	{
		std::ostringstream moduleList;
		vector<SharedComponent>::const_iterator i = modules.begin();
		while (i != modules.end())
		{
			SharedComponent module = *i++;
			moduleList << module->path << MODULE_SEPARATOR;
		}
		return moduleList.str();
	}

	bool Application::removeModule(const string &modulePath)
	{
		bool bRet = false;
		std::vector<SharedComponent>::iterator i = modules.begin();
		while (i != modules.end())
		{
			if (modulePath == (*i)->path)
			{
				i = modules.erase(i);
				bRet = true;
			}
			else
			{
				i++;
			}
		}
		return bRet;
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
		string transName(name);
		std::transform(transName.begin(), transName.end(), transName.begin(), tolower);
		if (transName == "runtime")
		{
			return this->runtime->path;
		}
		else
		{
			for(vector<SharedComponent>::const_iterator
				i = this->modules.begin();
				i != this->modules.end();
			i++)
			{
				SharedComponent comp = *i;
				if (comp->name == name)
				{
					return comp->path;
				}
			}
		}
		return string();
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

	void Application::setRuntimeProductVersion()
	{
		if (!runtime.isNull())
		{
			runtime->version = PRODUCT_VERSION;
		}
	}


	void Application::ResolveDependencies(vector<SharedDependency> & unresolved)
	{
		this->modules.clear(); // Blank slate
		this->runtime = NULL;
		vector<SharedComponent> components;
		this->GetAvailableComponents(components);

		vector<SharedDependency>::iterator i = this->dependencies.begin();
		while (i != this->dependencies.end())
		{
			SharedDependency d = *i++;
			SharedComponent c = BootUtils::ResolveDependency(d, components);
			if (c.isNull())
			{
				unresolved.push_back(d);
			}
			else if (c->type == MODULE)
			{
				this->modules.push_back(c);
			}
			else if (c->type == RUNTIME)
			{
				this->runtime = c;
			}
		}
		setRuntimeProductVersion();
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
		// Ensure that this module is not already in our list of modules.
		vector<SharedComponent>::iterator i = this->modules.begin();
		while (i != this->modules.end())
		{
			SharedComponent c = *i++;
			if (c->name == name)
			{
				// Bundled modules currently do not know their version until
				// they are loaded, so update the version field of the component.
				c->version = version;
				return;
			}
		}

		// It's not in the list so add it.
		SharedComponent c = KComponent::NewComponent(MODULE, name, version, path);
		this->modules.push_back(c);
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

	void Application::GetResolvedComponents(vector<SharedComponent> &resolved)
	{
		if (this->runtime)
		{
			resolved.push_back(this->runtime);
		}

		resolved.reserve(resolved.size() + this->modules.size());
		resolved.insert(resolved.end(), this->modules.begin(), this->modules.end());
	}
}