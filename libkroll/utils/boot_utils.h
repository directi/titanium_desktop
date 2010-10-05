/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _KR_BOOT_UTILS_H_
#define _KR_BOOT_UTILS_H_

// These UUIDs should never change and uniquely identify a package type
#define DISTRIBUTION_UUID "7F7FA377-E695-4280-9F1F-96126F3D2C2A"
#define RUNTIME_UUID "A2AC5CB5-8C52-456C-9525-601A5B0725DA"
#define MODULE_UUID "1ACE5D3A-2B52-43FB-A136-007BD166CFD0"
#define MANIFEST_FILENAME "manifest"
#define LICENSE_FILENAME "LICENSE.txt"
#define INSTALLED_MARKER_FILENAME ".installed"

namespace UTILS_NS
{
	using std::string;
	using std::vector;
	using std::pair;
	using std::map;

	enum KComponentType
	{
		MODULE,
		RUNTIME,
		UNKNOWN
	};

	/**
	 * Represents a single component dependency -- 
	 * one line in the application manifest
	 */
	class KROLL_API Dependency
	{
		Dependency(KComponentType type,
			const std::string &name,
			const std::string &version);
		Dependency(const std::string &key, const std::string &value);

		void parseInfo(const std::string &value);

	public:
		~Dependency();

		enum Requirement
		{
			EQ,
			GT,
			LT,
			GTE,
			LTE,
		};
		KComponentType type;
		std::string name;
		std::string version;
		Requirement requirement;

		/**
		* Generate a dependency from a key/value pair found in a manifest
		*/
		static SharedDependency NewDependencyFromManifestLine(
			const std::string &key, const std::string &value);

		/**
		* Generate a dependency from a set of values
		*/
		static SharedDependency NewDependencyFromValues(
			KComponentType type, const std::string &name, const std::string &version);
	};

	/**
	 * Represents a concrete Kroll components -- a runtime or module found on disk
	 */
	class KROLL_API KComponent
	{
	public:
		KComponentType type;
		std::string name;
		std::string version;
		std::string path;
		bool bundled;

		static SharedComponent NewComponent(KComponentType type,
			std::string name, std::string version,
			std::string path, bool bundled=false);
	};


	class KROLL_API ComponentManager
	{
	public:
		ComponentManager();
		~ComponentManager();

		static void GetAvailableComponentsAt(
			const std::string& path,
			vector<SharedComponent>& components,
			bool onlyBundled);
	};


	class KROLL_API ManifestHandler
	{
	public:
		/**
		 * checks for the existance of manifest file in given directory
		 * @param: dir: the directory for checking the manifest file.
		 * @return bool: true if the manifest file exists, false otherwise.
		 */
		static bool doesManifestFileExistsAtDirectory(const std::string & dir);

		static string getManifestPathAtDirectory(const string &dir);

		/**
		 * Read a manifest file. 
		 * @returns a vector of key-value pairs which represent the 
		 *    manifest's contents or an empty vector if it cannot be read.
		 */
		static void ReadManifestFile(const std::string &path, map<string, string> &manifest);
	};

	namespace BootUtils
	{

		KROLL_API void ScanBundledComponents(const std::string &path, vector<SharedComponent>& results);

		/**
		 * Compare two version strings in a piecewise way.
		 * @returns 1 if the first is larger, 0 if they are equal,
		 *     -1 if the second is larger
		 */
		KROLL_API int CompareVersions(const string &one, const string &two);

		/**
		 * Compare two version strings in a piecewise way, weakly
		 * @returns true if the first is larger or false otherwise
		 */
		KROLL_API bool WeakCompareComponents(SharedComponent, SharedComponent);

		KROLL_API std::vector<std::string>& GetComponentSearchPaths();

		KROLL_API std::vector<SharedComponent>& GetInstalledComponents(bool force=false);
		
		KROLL_API SharedComponent ResolveDependency(SharedDependency dep, std::vector<SharedComponent>&);

	};
}
#endif
