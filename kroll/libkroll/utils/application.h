/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _KR_UTILS_APPLICATION_H_
#define _KR_UTILS_APPLICATION_H_

#include <kroll/utils/boot_utils.h>
#include <kroll/utils/ManifestHandler.h>

namespace UTILS_NS
{
	using std::string;
	using std::vector;
	using std::pair;
	using std::map;

	/**
	 * Represents a concrete Kroll application -- found on disk
	 */
	class Application;
	typedef SharedPtr<Application> SharedApplication;

	class KROLL_API Application
	{
	private:
		Application(const std::string &path,
			const std::string &manifest_path,
			const map<string, string> &manifest);

		const string path;
		vector<string> arguments;

		vector<SharedDependency> dependencies;
		ManifestHandler manifestHandler;
		ComponentManager componentManager;

	public:
		static SharedApplication NewApplication(const std::string &appPath);
		~Application();

		string getPath() const { return this->path; }

		string getManifestPath() const { return this->manifestHandler.getManifestPath(); }
		string getName() const { return this->manifestHandler.getName(); }
		string getVersion() const { return this->manifestHandler.getVersion(); }
		string getId() const { return this->manifestHandler.getId(); }
		string getGUID() const { return this->manifestHandler.getGUID(); }
		string getURL() const { return this->manifestHandler.getURL(); }
		string getPublisher() const { return this->manifestHandler.getPublisher(); }
		string getLogLevel() const { return this->manifestHandler.getLogLevel(); }
		string getImage() const;

		void UsingModule(const std::string &name,
			const std::string &version,
			const std::string &path);
		bool removeModule(const string &modulePath);
		bool ResolveDependencies();

		std::string getRuntimePath() const;
		string GetComponentPath(const string &name) const;

		/**
		 * Whether or not this application has a .installed file in it's path
		 */
		bool IsInstalled() const;

		/**
		 * Get the path to this application's executablej
		 */
		string GetExecutablePath() const;

		/**
		 * Get the path to this application's user data directory.
		 */
		string GetDataPath() const;

		/**
		 * Get the path to this application's resources directory.
		 */
		string GetResourcesPath() const;

		/**
		 * Get the text of a license file for this application or an empty string if
		 * no license is found.
		 */
		std::string GetLicenseText() const;

		/**
		 * A mutator for this application's list of command-line arguments.
		 */
		void SetArguments(int argc, const char* argv[]);

		/**
		 * A mutator for this application's list of command-line arguments.
		 */
		void SetArguments(const vector<string>& arguments);

		/**
		 * An accessor for this application's list of command-line arguments.
		 */
		vector<string>& GetArguments();

		/**
		 * Whether or not the given argument was specified on the command-line. If "--arg"
		 * was specified, a needle equalling "--arg" or "arg" will return true.
		 */
		bool HasArgument(const string &needle) const;

		/**
		 * Get the value of an argument that was specified like arg=value or arg="value"
		 * @returns argument value or an empty string if not found
		 */
		string GetArgumentValue(const string &needle) const;
	};
}
#endif
