/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _COMPONENT_MANAGER_H_
#define _COMPONENT_MANAGER_H_

#include <base.h>
#include "boot_utils.h"

namespace UTILS_NS
{
	using std::string;
	using std::vector;
	using std::pair;
	using std::map;

	class KROLL_API PathBits
	{
	public:
		PathBits(const string& name, const string& fullPath) :
			name(name),
			fullPath(fullPath)
		{ }
		std::string name;
		std::string fullPath;
	};


	/**
	 * Represents a concrete Kroll components -- a runtime or module found on disk
	 */

	class KComponent;
	typedef SharedPtr<KComponent> SharedComponent;
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
	private:
		const std::string path;
		SharedComponent runtime;
		vector<SharedComponent> modules;
		vector<SharedDependency> unresolved;


	public:
		ComponentManager(const std::string &path);
		ComponentManager(const std::string &path, const vector<SharedDependency> &dependencies);
		~ComponentManager();

		bool allDependenciesResolved() const { return unresolved.empty(); }
		std::string getModulePaths() const;
		std::string getRuntimePath() const;
		bool getUnresolvedDependencies(vector<SharedDependency> &_unresolved) const;
		void resolveDependencies(const vector<SharedDependency> &dependencies);
		bool removeModule(const string &modulePath);
		void UsingModule(const std::string &name,
			const std::string &version,
			const std::string &path);
		string GetComponentPath(const string &name) const;
	};

	namespace BootUtils
	{
		KROLL_API void ScanBundledComponents(const std::string &path, vector<SharedComponent>& results);

		/**
		 * Compare two version strings in a piecewise way.
		 * @returns 1 if the first is larger, 0 if they are equal,
		 *			-1 if the second is larger
		 */
		KROLL_API int CompareVersions(const string &one, const string &two);

		/**
		 * Compare two version strings in a piecewise way, weakly
		 * @returns true if the first is larger or false otherwise
		 */
		KROLL_API bool WeakCompareComponents(SharedComponent one, SharedComponent two);
		KROLL_API SharedComponent ResolveDependency(SharedDependency dep, std::vector<SharedComponent>& components);
		KROLL_API void AddToComponentVector(vector<SharedComponent>& components, SharedComponent c);
		KROLL_API vector<PathBits> GetDirectoriesAtPath(std::string& path);
		KROLL_API void ScanRuntimesAtPath(const std::string &path, vector<SharedComponent>& results, bool bundled);
		KROLL_API void ScanModulesAtPath(const std::string &path, vector<SharedComponent>& results, bool bundled);
	};

}
#endif // _COMPONENT_MANAGER_H_
