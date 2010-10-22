/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#include "file_utils.h"
#include "ManifestHandler.h"

using std::string;
using std::vector;
using std::pair;

namespace UTILS_NS
{

	ManifestHandler::ManifestHandler(const std::string &_manifestPath)
		: manifestPath(_manifestPath)
	{
		map<string, string> manifest;
		ManifestHandler::ReadManifestFile(this->manifestPath, manifest);
		this->parseManifest(manifest);
	}

	ManifestHandler::ManifestHandler(const std::string &_manifestPath, const map<string, string> &manifest)
		: manifestPath(_manifestPath)
	{
		this->parseManifest(manifest);
	}

	ManifestHandler::~ManifestHandler()
	{
	}


	void ManifestHandler::parseManifest(const map<string, string>& manifest)
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

	void ManifestHandler::getDependencies(vector<SharedDependency> &dependencies)
	{
		for(map<string, string>::const_iterator
			oIter = dep.begin();
			oIter != dep.end();
		oIter++)
		{
			SharedDependency d = Dependency::NewDependencyFromManifestLine(oIter->first, oIter->second);
			dependencies.push_back(d);
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
}