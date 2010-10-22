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

		void ParseManifest(const map<string, string>& manifest);
		void setRuntimeProductVersion();

		const string path;

		vector<string> arguments;

		SharedComponent runtime;
		vector<SharedComponent> modules;

		vector<SharedDependency> dependencies;
		ManifestHandler manifestHandler;
		ComponentManager componentManager;

	public:
		static SharedApplication NewApplication(const std::string &appPath);
		~Application();

		string getManifestPath() const { return this->manifestHandler.getManifestPath(); }
		string getName() const { return this->manifestHandler.getName(); }
		string getVersion() const { return this->manifestHandler.getVersion(); }
		string getId() const { return this->manifestHandler.getId(); }
		string getGUID() const { return this->manifestHandler.getGUID(); }
		string getURL() const { return this->manifestHandler.getURL(); }
		string getPublisher() const { return this->manifestHandler.getPublisher(); }
		string getLogLevel() const { return this->manifestHandler.getLogLevel(); }
		string getImage() const;

		string getPath() const { return this->path; }
		SharedComponent getRuntime() const { return this->runtime; }
		std::string getRuntimePath() const;
		void getModules(vector<SharedComponent> &_modules) const;

		void getDependencies(vector<SharedDependency> &_dependencies) const;
		void getUnresolvedDependencies(vector<SharedDependency> & unresolved) const;
		void getComponents(std::vector<SharedComponent> &components) const;
		string getModulePaths() const;

		/**
		 * Get all resolved components for this application including
		 * runtimes and modules.
		 */
		void GetResolvedComponents(vector<SharedComponent> &resolved);

		/**
		 * Generate a list of all components available for this application
		 * including bundled components and any components or all the components
		 * in the bundle override directory.
		 */
		void GetAvailableComponents(
			vector<SharedComponent>& components,
			bool onlyBundled = false);

		/**
		 * Inform the application that it is using a module with the given
		 * name and version. If this is a new module, it will be registered in
		 * the application's module list.
		 */
		void UsingModule(
			const std::string &name,
			const std::string &version,
			const std::string &path);
		
		bool removeModule(const string &modulePath);

		/**
		 * Whether or not this application has a .installed file in it's path
		 */
		bool IsInstalled() const;

		/**
		 * Try to resolve all application dependencies with installed or bundled components.
		 * @returns a list of unresolved dependencies
		 */
		void ResolveDependencies(vector<SharedDependency> & unresolved);

		/**
		 * Get the path to this application's executablej
		 */
		string GetExecutablePath() const;

		/**
		 * Get an active component path given a name.
		 * @arg name a component name either the name of a module (e.g. 'tiui') or 'runtime'
		 * @returns the path to the component with the given name or an empty string if not found
		 */
		string GetComponentPath(const string &name) const;

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
