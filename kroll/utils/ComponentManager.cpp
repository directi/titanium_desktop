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
	void safelyAddComponentTo(vector<SharedComponent>& components, SharedComponent c)
	{
		// Avoid adding duplicate components to a component vector
		for(vector<SharedComponent>::iterator
			i = components.begin(); i != components.end(); i++)
		{
			if((*i)->equal(c))
			{
				return;
			}
		}
		components.push_back(c);
	}

	void GetSubDirectoriesAt(const std::string& path, vector<PathBits> & dirs)
	{
		vector<string> paths;

		FileUtils::ListDir(path, paths);
		for(vector<string>::iterator
			i = paths.begin();
			i != paths.end();
		i++)
		{
			string& subpath(*i);
			if (subpath[0] == '.')
				continue;

			string fullPath(FileUtils::Join(path.c_str(), subpath.c_str(), NULL));
			if (FileUtils::IsDirectory(fullPath))
			{
				dirs.push_back(PathBits(subpath, fullPath));
			}
		}
	}


	void addComponentVersions(KComponentType type,
		const std::string& component_name,
		const std::string component_path,
		vector<SharedComponent>& results)
	{
		vector<PathBits> component_versions;
		GetSubDirectoriesAt(component_path, component_versions);
		for(vector<PathBits>::iterator
			oIter = component_versions.begin();
			oIter != component_versions.end();
		oIter++)
		{
			safelyAddComponentTo(results, KComponent::NewComponent(
				type, component_name, oIter->name, oIter->fullPath));
		}
	}

	void ScanRuntimesAtPath(const std::string &path, vector<SharedComponent>& results)
	{
		string runtime_path(FileUtils::Join(path.c_str(), "runtime", 0));
		addComponentVersions(RUNTIME, "runtime", runtime_path, results);
	}

	void ScanModulesAtPath(const std::string &path, vector<SharedComponent>& results)
	{
		if (!FileUtils::IsDirectory(path))
			return;

		string namesPath(FileUtils::Join(path.c_str(), "modules", 0));
		vector<PathBits> modules;
		GetSubDirectoriesAt(namesPath, modules);

		for(vector<PathBits>::iterator
			oIter = modules.begin();
			oIter != modules.end();
		oIter++)
		{
			addComponentVersions(MODULE, oIter->name, oIter->fullPath, results);
		}
	}

	void ScanBundledComponents(const std::string &path, vector<SharedComponent>& results)
	{
		ScanRuntimesAtPath(path, results);
		ScanModulesAtPath(path, results);
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
}

	bool KComponent::equal(const SharedComponent other)
	{
		if ((this->type == other->type)
			&& (this->path == other->path))
		{
			return true;
		}
		return false;
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
