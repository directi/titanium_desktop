/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#include <boot_utils.h>
#include <file_utils.h>

using std::string;
using std::vector;
using std::pair;

namespace UTILS_NS
{
namespace BootUtils
{
	static void ScanRuntimesAtPath(const std::string &path, vector<SharedComponent>&, bool=true);
	static void ScanModulesAtPath(const std::string &path, vector<SharedComponent>&, bool=true);
	static void AddToComponentVector(vector<SharedComponent>&, SharedComponent);

	static void AddToComponentVector(vector<SharedComponent>& components,
		SharedComponent c)
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

	class PathBits
	{
	public:
		PathBits(const string& name, const string& fullPath) :
			name(name),
			fullPath(fullPath)
		{ }
		std::string name;
		std::string fullPath;
	};

	static vector<PathBits> GetDirectoriesAtPath(std::string& path)
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

	static void ScanRuntimesAtPath(const std::string &path, vector<SharedComponent>& results, bool bundled)
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

	static void ScanModulesAtPath(const std::string &path, vector<SharedComponent>& results, bool bundled)
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


	Dependency::Dependency(KComponentType type,
		const std::string &name,
		const std::string &version)
		: type(type),
		name(name),
		version(version),
		requirement(EQ)
	{
	}

	Dependency::Dependency(const std::string &key, const std::string &value)
		: type(UNKNOWN),
		name(key),
		version(""),
		requirement(EQ)
	{
		parseInfo(value);
		if (key == "runtime")
		{
			this->type = RUNTIME;
		}
		else
		{
			this->type = MODULE;
		}

	}

	Dependency::~Dependency()
	{
	}

	void Dependency::parseInfo(const std::string &value)
	{
		size_t versionStart;
		if (value.find(">=") != string::npos)
		{
			this->requirement = GTE;
			versionStart = 2;
		}
		else if (value.find("<=") != string::npos)
		{
			this->requirement = LTE;
			versionStart = 2;
		}
		else if (value.find("<") != string::npos)
		{
			this->requirement = LT;
			versionStart = 1;
		}
		else if (value.find(">") != string::npos)
		{
			this->requirement = GT;
			versionStart = 1;
		}
		else if (value.find("=") != string::npos)
		{
			this->requirement = EQ;
			versionStart = 1;
		}
		else
		{
			this->requirement = EQ;
			versionStart = 0;
		}

		this->version = value.substr(versionStart);
	}

	SharedDependency Dependency::NewDependencyFromValues(
		KComponentType type, const std::string &name, const std::string &version)
	{
		Dependency* d = new Dependency(type, name, version);
		return d;
	}

	SharedDependency Dependency::NewDependencyFromManifestLine(
		const std::string &key, const std::string &value)
	{
		Dependency* d = new Dependency(key, value);
		return d;
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
}
