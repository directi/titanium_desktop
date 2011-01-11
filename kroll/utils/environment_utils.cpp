/** * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#include "environment_utils.h"

#include <cstdlib>
#include <sstream>

#ifdef OS_WIN32
#include "win32/win32_utils.h"
// See http://msdn.microsoft.com/en-us/library/ms686206(VS.85).aspx
#define MAX_ENV_VALUE_SIZE 32767 
#define REASONABLE_MAX_ENV_VALUE_SIZE 1024
#elif OS_OSX
#include <crt_externs.h>
#define environ (*_NSGetEnviron())
#else
extern char** environ;
#endif

namespace UTILS_NS
{
namespace EnvironmentUtils
{
	bool Has(std::string name)
	{
#ifdef OS_WIN32
		std::wstring wideName = UTF8ToWide(name);
		DWORD len = GetEnvironmentVariableW(wideName.c_str(), 0, 0);
		return len > 0;
#else
		return getenv(name.c_str()) != 0;
#endif
	}

	std::string Get(std::string name)
	{
#ifdef OS_WIN32
		// Allocate a small buffer first, before taking the plunge
		// and allocating the maximum size. Hopefully this will prevent
		// expensive allocations.
		wchar_t* buffer = new wchar_t[REASONABLE_MAX_ENV_VALUE_SIZE];
		std::wstring wideName(UTF8ToWide(name));

		DWORD size = GetEnvironmentVariableW(wideName.c_str(), buffer, REASONABLE_MAX_ENV_VALUE_SIZE - 1);
		if (size > REASONABLE_MAX_ENV_VALUE_SIZE)
		{
			// This is a humongous environment variable value, so we need to allocate the
			// max size and grab it that way.
			delete [] buffer;
			buffer = new wchar_t[MAX_ENV_VALUE_SIZE];
			size = GetEnvironmentVariableW(wideName.c_str(), buffer, MAX_ENV_VALUE_SIZE - 1);
		}

		if (size > 0)
		{
			buffer[size] = '\0';
			std::string result(WideToUTF8(buffer));
			delete [] buffer;
			return result;
		}
		else
		{
			delete [] buffer;
			return std::string();
		}
#else
		const char* val = getenv(name.c_str());
		if (val)
			return std::string(val);
		else
			return std::string();
#endif
	}

	void Set(std::string name, std::string value)
	{
#ifdef OS_WIN32
		std::wstring wideName(UTF8ToWide(name));
		std::wstring wideValue(UTF8ToWide(value));
		if (SetEnvironmentVariableW(wideName.c_str(), wideValue.c_str()) == 0)
		{
			throw std::string("Cannot set environment variable: ") + name;
		}
#else
		if (setenv(name.c_str(), value.c_str(), 1))
		{
			throw std::string("Cannot set environment variable: ") + name;
		}
#endif
	}

	void Unset(std::string name)
	{
#ifdef OS_WIN32
		std::wstring wideName(UTF8ToWide(name));
		SetEnvironmentVariableW(wideName.c_str(), NULL);
#else
		unsetenv(name.c_str());
#endif
	}

	std::map<std::string, std::string> GetEnvironment()
	{
		std::map<std::string, std::string> environment;
#ifdef OS_WIN32
		LPWCH env = GetEnvironmentStringsW();
		while (env[0] != '\0')
		{
			std::wstring entryW(env);
			std::string entry(WideToUTF8(entryW));
			std::string key(entry.substr(0, entry.find("=")));
			std::string val(entry.substr(entry.find("=")+1));
			environment[key] = val;
			env += entry.size() + 1;
		}
#else
		char** current = environ;
		while (*current)
		{
			std::string entry(*current);
			std::string key(entry.substr(0, entry.find("=")));
			std::string val(entry.substr(entry.find("=")+1));
			environment[key] = val;
			current++;
		}
#endif
		return environment;
	}

	std::string GetOSVersion()
	{
#ifdef OS_WIN32
		OSVERSIONINFOW vi;
		vi.dwOSVersionInfoSize = sizeof(vi);
		if (GetVersionExW(&vi) == 0)
		{
			return std::string("Unknown");
		}
		std::ostringstream str;
		str << vi.dwMajorVersion << "." << vi.dwMinorVersion << " (Build " << (vi.dwBuildNumber & 0xFFFF);
		std::string version = WideToUTF8(vi.szCSDVersion);
		if (!version.empty()) str << ": " << version;
		str << ")";
		return str.str();
#elif OS_OSX
	// Do not use /System/Library/CoreServices/SystemVersion.plist.
	// See http://www.cocoadev.com/index.pl?DeterminingOSVersion
	SInt32 major, minor, bugfix;
	if (Gestalt(gestaltSystemVersionMajor, &major) != noErr ||
		Gestalt(gestaltSystemVersionMinor, &minor) != noErr ||
		Gestalt(gestaltSystemVersionBugFix, &bugfix) != noErr)
	{
		logger()->Error("Failed to get OS version");
		return "Unknown";
	}

	return [[NSString stringWithFormat:@"%d.%d.%d", major, minor, bugfix] UTF8String];
#else
	struct utsname uts;
	uname(&uts);
	return uts.release;
#endif
	}

	std::string GetOSName()
	{
#ifdef OS_WIN32
		OSVERSIONINFO vi;
		vi.dwOSVersionInfoSize = sizeof(vi);
		if (GetVersionEx(&vi) == 0)
		{
			return std::string("Unknown");
		}
		switch (vi.dwPlatformId)
		{
		case VER_PLATFORM_WIN32s:
			return "Windows 3.x";
		case VER_PLATFORM_WIN32_WINDOWS:
			return vi.dwMinorVersion == 0 ? "Windows 95" : "Windows 98";
		case VER_PLATFORM_WIN32_NT:
			return "Windows NT";
		default:
			return "Unknown";
		}
#elif OS_OSX
		// TODO: implement it for osx
#else
		struct utsname uts;
		uname(&uts);
		return uts.sysname;
#endif
	}

	std::string GetOSArchitecture()
	{
#ifdef OS_WIN32
		SYSTEM_INFO si;
		GetSystemInfo(&si);
		switch (si.wProcessorArchitecture)
		{
		case PROCESSOR_ARCHITECTURE_INTEL:
			return "IA32";
		case PROCESSOR_ARCHITECTURE_MIPS:
			return "MIPS";
		case PROCESSOR_ARCHITECTURE_ALPHA:
			return "ALPHA";
		case PROCESSOR_ARCHITECTURE_PPC:
			return "PPC";
		case PROCESSOR_ARCHITECTURE_IA64:
			return "IA64";
#ifdef PROCESSOR_ARCHITECTURE_IA32_ON_WIN64
		case PROCESSOR_ARCHITECTURE_IA32_ON_WIN64:
			return "IA64/32";
#endif
#ifdef PROCESSOR_ARCHITECTURE_AMD64
		case PROCESSOR_ARCHITECTURE_AMD64:
			return "AMD64";
#endif
		default:
			return "Unknown";
		}
#elif OS_OSX
		// TODO: implement it for osx
#else
		struct utsname uts;
		uname(&uts);
		return uts.machine;
#endif
	}

	std::string GetHomePath()
	{
#ifdef OS_WIN32
		std::string result = Get("HOMEDRIVE");
		result.append(Get("HOMEPATH"));
		std::string::size_type n = result.size();
		if (n > 0 && result[n - 1] != '\\')
			result.append("\\");

		// cygwin customization
		if (result.size() == 3)
		{
			std::string odir = Get("USERPROFILE");
			if (!odir.empty())
			{
				result = odir;
			}
		}

		return result;
#else
	std::string path;
	struct passwd* pwd = getpwuid(getuid());
	if (pwd)
		path = pwd->pw_dir;
	else
	{
		pwd = getpwuid(geteuid());
		if (pwd)
			path = pwd->pw_dir;
		else
			path = EnvironmentImpl::getImpl("HOME");
	}
	std::string::size_type n = path.size();
	if (n > 0 && path[n - 1] != '/') path.append("/");
	return path;
#endif
	}

}
}
