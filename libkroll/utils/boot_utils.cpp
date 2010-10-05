/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#include "utils.h"

using std::string;
using std::vector;
using std::pair;

namespace UTILS_NS
{

	ManifestHandler::ManifestHandler(const std::string &_manifestPath)
		: manifestPath(_manifestPath)
	{
	}

	ManifestHandler::~ManifestHandler()
	{
	}


	void ManifestHandler::ParseManifest(const map<string, string>& manifest)
	{
		for(map<string, string>::const_iterator
			oIter = manifest.begin();
			oIter != manifest.end();
		oIter++)
		{
			string key(oIter->first);
			string value(oIter->second);

			if (key == "#appname")
			{
				this->name = value;
				continue;
			}
			else if (key == "#appid")
			{
				this->id = value;
				continue;
			}
			else if (key == "#guid")
			{
				this->guid = value;
				continue;
			}
			else if (key == "#image")
			{
				this->image = value;
				continue;
			}
			else if (key == "#publisher")
			{
				this->publisher = value;
				continue;
			}
			else if (key == "#url")
			{
				this->url = value;
				continue;
			}
			else if (key == "#version")
			{
				this->version = value;
				continue;
			}
			else if (key == "#loglevel")
			{
				this->logLevel = value;
				continue;
			}
			else if (key[0] == '#')
			{
				continue;
			}
			else
			{
				dep[key] = value;
			}
		}
	}

	void ManifestHandler::getDependencies(map<string, string> & _dep)
	{
		for(map<string, string>::const_iterator
			oIter = dep.begin();
			oIter != dep.end();
		oIter++)
		{
			_dep[oIter->first] = oIter->second;
		}
	}


	string ManifestHandler::getManifestPathAtDirectory(const string &dir)
	{
		string manifestpath(FileUtils::Join(dir.c_str(), MANIFEST_FILENAME, NULL));
		return manifestpath;
	}

	bool ManifestHandler::doesManifestFileExistsAtDirectory(const std::string & dir)
	{
		string manifestPath = FileUtils::Join(dir.c_str(), MANIFEST_FILENAME, NULL);
		if (FileUtils::IsFile(manifestPath))
		{
			return true;
		}
		return false;
	}

	void ManifestHandler::ReadManifestFile(const std::string &path, map<string, string> &manifest)
	{
		if (FileUtils::IsFile(path))
		{
			string manifestContents(FileUtils::ReadFile(path));
			if (!manifestContents.empty())
			{
				vector<string> manifestLines;
				FileUtils::Tokenize(manifestContents, manifestLines, "\n");
				for (size_t i = 0; i < manifestLines.size(); i++)
				{
					string line = FileUtils::Trim(manifestLines[i]);
					vector<string> manifestLineData;
					FileUtils::Tokenize(line, manifestLineData, ":");

					if(manifestLineData.size() == 2)
					{
						manifest[FileUtils::Trim(manifestLineData[0]) ] = FileUtils::Trim(manifestLineData[1]);
					}
				}
			}
		}
	}


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


	ComponentManager::ComponentManager()
	{
	}

	ComponentManager::~ComponentManager()
	{
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
