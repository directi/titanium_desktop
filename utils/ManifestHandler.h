/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _MANIFEST_HANDLER_H_
#define _MANIFEST_HANDLER_H_

#include <map>
#include <vector>
#include <string>
#include <iostream>

#include "boot_utils.h"

#define MANIFEST_FILENAME "manifest"

namespace UTILS_NS
{
	class ManifestHandler
	{
		const std::string manifestPath;

		string name;
		string version;
		string id;
		string guid;
		string url;
		string publisher;
		string image;
		string logLevel;

		map<string, string> dep;

	public:
		ManifestHandler(const std::string &_manifestPath);
		~ManifestHandler();

		void ParseManifest(const map<string, string>& manifest);

		std::string getManifestPath() const { return manifestPath; }
		string getName() const { return this->name; }
		string getVersion() const { return this->version; }
		string getId() const { return this->id; }
		string getGUID() const { return this->guid; }
		string getURL() const { return this->url; }
		string getPublisher() const { return this->publisher; }
		string getImage() const { return this->image; }
		string getLogLevel() const { return this->logLevel; }

		void getDependencies(map<string, string> & _dep);

		/**
		 * checks for the existance of manifest file in given directory
		 * @param: dir: the directory for checking the manifest file.
		 * @return bool: true if the manifest file exists, false otherwise.
		 */
		static bool doesManifestFileExistsAtDirectory(const std::string & dir);

		static string getManifestPathAtDirectory(const string &dir);

		/**
		 * Read a manifest file. 
		 * @param full path to the manifest file
		 * @param a map of keys-values which represent the manifest's contents.
		 */
		static void ReadManifestFile(const std::string &path, map<string, string> &manifest);
	};
}
#endif // _MANIFEST_HANDLER_H_
