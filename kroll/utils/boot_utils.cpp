/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#include "boot_utils.h"
#include "file_utils.h"

#include <sstream>

using std::string;
using std::vector;
using std::pair;

namespace UTILS_NS
{
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
}
