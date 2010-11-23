/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _KR_BOOT_UTILS_H_
#define _KR_BOOT_UTILS_H_

#include <base.h>


// These UUIDs should never change and uniquely identify a package type
#define DISTRIBUTION_UUID "7F7FA377-E695-4280-9F1F-96126F3D2C2A"
#define RUNTIME_UUID "A2AC5CB5-8C52-456C-9525-601A5B0725DA"
#define MODULE_UUID "1ACE5D3A-2B52-43FB-A136-007BD166CFD0"
#define LICENSE_FILENAME "LICENSE.txt"
#define INSTALLED_MARKER_FILENAME ".installed"

#ifdef OS_WIN32
#define MODULE_SEPARATOR ";"
#else
#define MODULE_SEPARATOR ":"
#endif

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
	class Dependency;
	typedef SharedPtr<Dependency> SharedDependency;

	class KROLL_API Dependency
	{
		Dependency(KComponentType type,
			const std::string &name,
			const std::string &version);
		Dependency(const std::string &key, const std::string &value);

		void parseInfo(const std::string &value);

	public:
		virtual ~Dependency();

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
}
#endif
