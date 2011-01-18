/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008-2009 Appcelerator, Inc. All Rights Reserved.
 */

#define _KROLL_BOOT_ 1
#ifndef _BOOT_H_
#define _BOOT_H_

// ensure that Kroll API is never included to create
// an artificial dependency on kroll shared library
#ifdef _KROLL_H_
#error You should not have included the kroll api!
#endif

#define BOOTSTRAP_ENV "KR_BOOTSTRAPPED"
#define DEBUG_OPT "debug"

#define HOME_ENV "KR_HOME"
#define RUNTIME_ENV "KR_RUNTIME"
#define MODULES_ENV "KR_MODULES"
#define DEBUG_ENV "KR_DEBUG"

#define CRASH_REPORT_OPT "--crash_report"
#define CRASH_REPORT_URL  STRING(_CRASH_REPORT_URL)

#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <sstream>

#include <base.h>

using std::string;
using std::vector;
using std::map;
using std::wstring;


typedef int Executor(int argc, const char ** argv);

class BootLoader
{
protected:
	int argc;
	const char** argv;

public:
	BootLoader(int _argc, const char ** _argv);
	virtual ~BootLoader();

	int Bootstrap();
	virtual int StartHost()=0;

protected:
	void ShowError(const string & msg, bool fatal=false) const;
	void setPaths(const std::string & app_path,
		const std::string & runtime_path,
		const std::string & module_paths);

	virtual string Blastoff()=0;
	virtual void ShowErrorImpl(const std::string & msg, bool fatal) const=0;
	virtual void setPlatformSpecificPaths(const std::string & runtime_path, const std::string & module_paths)=0;
};


#ifdef USE_BREAKPAD
class CrashHandler
{
public:
	CrashHandler(int _argc, const char ** _argv);
	virtual ~CrashHandler();

	virtual int SendCrashReport()=0;

protected:
	static string applicationHome;
	static string dumpFilePath;
	static string executable_name;
	static string app_name;

	static string GetCrashDetectionTitle();
	static string GetCrashDetectionHeader();
	static string GetCrashDetectionMessage();
	static string GetApplicationName();

	void InitCrashDetection();
	void GetCrashReportParameters(map<string, string> & param);
};
#endif // USE_BREAKPAD

#endif
