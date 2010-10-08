/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _KR_BASE_H_
#define _KR_BASE_H_

#ifdef __cplusplus
#include <string>
#include <map>
#include <vector>
#include <iostream>
#include <algorithm>
#endif

#ifdef OS_WIN32
#define OS_NAME "win32"
#elif defined(OS_OSX)
#define OS_NAME "osx"
#elif defined(OS_LINUX)
#define OS_NAME "linux"
#endif

#ifdef OS_64
#define OS_TYPE "64bit"
#else
#define OS_TYPE "32bit"
#endif

#ifdef USE_NO_EXPORT
#define KROLL_API
#else
#if defined(OS_OSX) || defined(OS_LINUX)
#define EXPORT __attribute__((visibility("default")))
#define KROLL_API EXPORT
#elif defined(OS_WIN32)
# ifdef KROLL_API_EXPORT
#  define KROLL_API __declspec(dllexport)
# else
#  define KROLL_API __declspec(dllimport)
# endif
# define EXPORT __declspec(dllexport)
#endif
#endif

#ifdef OS_WIN32
#ifndef WINVER
#define WINVER 0x0502
//#define _CRT_SECURE_NO_WARNINGS  // turn off string safe warnings
#endif

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0502
#endif

#ifndef _WIN32_WINDOWS
#define _WIN32_WINDOWS 0x0410
#endif

#ifndef _WIN32_IE
#define _WIN32_IE 0x600
#endif
#endif


#define VAL(str) #str
#define STRING(str) VAL(str)

#ifndef _PRODUCT_NAME
   #define _PRODUCT_NAME Kroll
#endif

#define PRODUCT_NAME STRING(_PRODUCT_NAME)

#ifndef GLOBAL_NS_VARNAME
  #define GLOBAL_NS_VARNAME STRING(_GLOBAL_NS_VARNAME)
#endif

#ifndef CONFIG_FILENAME
  #define CONFIG_FILENAME STRING(_CONFIG_FILENAME)
#endif

#define PRODUCT_VERSION STRING(_PRODUCT_VERSION)


// define a macro that points to the name of the enclosing function where
// referenced or if not supported on target compiler, just default to filename
#ifndef OS_WIN32
#define KR_FUNC __PRETTY_FUNCTION__
#else
#define KR_FUNC __FILE__
#endif

#ifdef DEBUG
#define KR_DUMP_LOCATION std::cout << "[" << KR_FUNC << "::" << __LINE__ << "]" << std::endl;
#else
#define KR_DUMP_LOCATION
#endif

#define KR_UNUSED(o) if (o) {}

// this was borrowed from Google Gears
// A macro to disallow the evil copy constructor and operator= functions.
// This should be used in the private: declaration for a class.
#define DISALLOW_EVIL_CONSTRUCTORS(TypeName)    \
  TypeName(const TypeName&);                    \
  void operator=(const TypeName&)

#if defined(KROLL_API_EXPORT) || defined(_KROLL_H_)
	#include <Poco/SharedPtr.h>
	using Poco::SharedPtr;
	#define UTILS_NS kroll
#else
	#define UTILS_NS KrollUtils
	#include "utils/poco/KSharedPtr.h"
	using KPoco::SharedPtr;
#endif


namespace kroll
{
	typedef SharedPtr<std::string> SharedString;
	typedef std::vector<SharedString> StringList;
	typedef SharedPtr<StringList> SharedStringList;
};
#endif
