/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#include "ComponentManager.h"
#include "file_utils.h"

#include <sstream>

using std::string;
using std::vector;
using std::pair;

namespace UTILS_NS
{
namespace BootUtils
{
	void AddToComponentVector(vector<SharedComponent>& components, SharedComponent c)
	{
		// Avoid adding duplicate components to a component vector
		vector<SharedComponent>::iterator i = components.begin();
		while (i != components.end())
		{
			SharedComponent e = *i++;
			if (e->type == c->type && e->path == c->path)
			{
				return;
			}
		}

		components.push_back(c);
	}


	vector<PathBits> GetDirectoriesAtPath(std::string& path)
	{
		vector<PathBits> directories;
		vector<string> paths;

		FileUtils::ListDir(path, paths);
		vector<string>::iterator i = paths.begin();
		while (i != paths.end())
		{
			string& subpath(*i++);
			if (subpath[0] == '.')
				continue;

			string fullPath(FileUtils::Join(path.c_str(), subpath.c_str(), NULL));
			if (!FileUtils::IsDirectory(fullPath))
				continue;

			directories.push_back(PathBits(subpath, fullPath));
		}
		return directories;
	}

	SharedComponent ResolveDependency(SharedDependency dep, vector<SharedComponent>& components)
	{
		vector<SharedComponent>::iterator i = components.begin();
		while (i != components.end())
		{
			SharedComponent comp = *i++;
			if (dep->type != comp->type || dep->name != comp->name)
				continue;

			int compare = CompareVersions(comp->version, dep->version);
			if ((dep->requirement == Dependency::EQ && compare == 0)
				|| (dep->requirement == Dependency::GTE && compare >= 0)
				|| (dep->requirement == Dependency::GT && compare > 0)
				|| (dep->requirement == Dependency::LT && compare < 0))
			{
				return comp;
			}
		}

		return NULL;
	}
	void ScanRuntimesAtPath(const std::string &path, vector<SharedComponent>& results, bool bundled)
	{
		if (!FileUtils::IsDirectory(path))
			return;

		// Read everything that looks like <searchpath>/runtime/<os>/*
		string rtPath(FileUtils::Join(path.c_str(), "runtime", 0));
		if (!bundled)
			rtPath = FileUtils::Join(rtPath.c_str(), OS_NAME, 0);
		vector<PathBits> versions(GetDirectoriesAtPath(rtPath));
		for (size_t i = 0; i < versions.size(); i++)
		{
			PathBits& b = versions[i];
			AddToComponentVector(results,
				KComponent::NewComponent(RUNTIME, "runtime", b.name, b.fullPath));
		}
	}

	void ScanModulesAtPath(const std::string &path, vector<SharedComponent>& results, bool bundled)
	{
		if (!FileUtils::IsDirectory(path))
			return;

		// Read everything that looks like <searchpath>/modules/<os>/*
		string namesPath(FileUtils::Join(path.c_str(), "modules", 0));
		if (!bundled)
			namesPath = FileUtils::Join(namesPath.c_str(), OS_NAME, 0);
		vector<PathBits> moduleNames(GetDirectoriesAtPath(namesPath));

		for (size_t i = 0; i < moduleNames.size(); i++)
		{
			PathBits& moduleName = moduleNames[i];

			// Read everything that looks like <searchpath>/modules/<os>/<name>/*
			vector<PathBits> moduleVersions(GetDirectoriesAtPath(moduleName.fullPath));
			for (size_t j = 0; j < moduleVersions.size(); j++)
			{
				PathBits& moduleVersion = moduleVersions[j];
				AddToComponentVector(results, KComponent::NewComponent(
					MODULE, moduleName.name, moduleVersion.name, moduleVersion.fullPath, bundled));
			}
		}
	}

	void ScanBundledComponents(const std::string &path, vector<SharedComponent>& results)
	{
		ScanRuntimesAtPath(path, results, true);
		ScanModulesAtPath(path, results, true);
	}
	vector<SharedComponent>& GetInstalledComponents(bool force)
	{
		static std::vector<SharedComponent> installedComponents;
		if (installedComponents.empty() || force)
		{
			installedComponents.clear();
			vector<string>& paths = GetComponentSearchPaths();
			vector<string>::iterator i = paths.begin();
			while (i != paths.end())
			{
				string path(*i++);
				ScanRuntimesAtPath(path, installedComponents, false);
				ScanModulesAtPath(path, installedComponents, false);
			}

			// Sort components by version here so that the latest version of
			// any component will always be chosen. Use a stable_sort because we
			// want to give preference to components earlier on the search path.
			std::stable_sort(
				installedComponents.begin(),
				installedComponents.end(),
				BootUtils::WeakCompareComponents);
		}
		return installedComponents;
	}

	int CompareVersions(const string &one, const string &two)
	{
		if (one.empty() && two.empty())
			return 0;
		if (one.empty())
			return -1;
		if (two.empty())
			return 1;

		vector<string> listOne;
		vector<string> listTwo;
		FileUtils::Tokenize(one, listOne, ".");
		FileUtils::Tokenize(two, listTwo, ".");

		size_t min = listOne.size();
		if (listTwo.size() < listOne.size())
			min = listTwo.size();

		for (size_t i = 0; i < min; i++)
		{
			int result = listOne.at(i).compare(listTwo.at(i));
			if (result != 0)
				return result;
		}

		if (listOne.size() > listTwo.size())
			return 1;
		else if (listTwo.size() > listOne.size())
			return -1;
		else
			return 0;
	}

	bool WeakCompareComponents(SharedComponent one, SharedComponent two)
	{
		return BootUtils::CompareVersions(one->version, two->version) > 0;
	}
}
	SharedComponent KComponent::NewComponent(KComponentType type, string name,
		string version, string path, bool bundled)
	{
		KComponent* c = new KComponent();
		c->type = type;
		c->name = name;
		c->version = version;
		c->path = path;
		c->bundled = true;
		return c;
	}

	ComponentManager::ComponentManager(const std::string &path)
		: path(path)
	{
	}

	ComponentManager::ComponentManager(const std::string &path, const vector<SharedDependency> &dependencies)
		: path(path)
	{
		resolveDependencies(dependencies);
	}

	ComponentManager::~ComponentManager()
	{
	}

	void ComponentManager::resolveDependencies(const vector<SharedDependency> &dependencies)
	{
		vector<SharedComponent> components;
		BootUtils::ScanBundledComponents(this->path, components);

		for(vector<SharedDependency>::const_iterator
			i = dependencies.begin();
			i != dependencies.end();
		i++)
		{
			const SharedDependency d = *i;
			SharedComponent c = BootUtils::ResolveDependency(d, components);
			if (c.isNull())
			{
				this->unresolved.push_back(d);
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
	}

	std::string ComponentManager::getModulePaths() const
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

	std::string ComponentManager::getRuntimePath() const
	{
		if(this->runtime)
		{
			return this->runtime->path;
		}
		return string("");
	}

	bool ComponentManager::getUnresolvedDependencies(
		vector<SharedDependency> &_unresolved) const
	{
		if(!unresolved.empty())
		{
			_unresolved.reserve(unresolved.size());
					_unresolved.reserve(unresolved.size());
			for(vector<SharedDependency>::const_iterator
				oIter = unresolved.begin();
				oIter != unresolved.end();
			oIter++)
			{
				_unresolved.push_back(*oIter);
			}
			return true;
		}
		return false;
	}

	void ComponentManager::GetAvailableComponentsAt(
		const std::string& path,
		vector<SharedComponent>& components,
		bool onlyBundled)
	{
		// Merge bundled and installed components
		BootUtils::ScanBundledComponents(path, components);

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

	bool ComponentManager::removeModule(const string &modulePath)
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

	string ComponentManager::GetComponentPath(const string &name) const
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

	void ComponentManager::UsingModule(
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

}
