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

	class PathBits
	{
	public:
		PathBits(const string& name, const string& fullPath)
			: name(name), fullPath(fullPath) { }
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

		bool equal(const SharedComponent other);

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
		virtual ~ComponentManager();

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
		KROLL_API SharedComponent ResolveDependency(SharedDependency dep, std::vector<SharedComponent>& components);
	};

}
#endif // _COMPONENT_MANAGER_H_
