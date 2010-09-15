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
#define CRASH_REPORT_OPT "--crash_report"

#define CRASH_REPORT_URL  STRING(_CRASH_REPORT_URL)

#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <utils.h>

using namespace KrollUtils;
using KrollUtils::Application;
using KrollUtils::Dependency;
using KrollUtils::KComponent;
using KrollUtils::SharedApplication;
using KrollUtils::SharedDependency;
using KrollUtils::SharedComponent;
using std::string;
using std::vector;
using std::map;
using std::wstring;

#ifdef OS_WIN32
#define MODULE_SEPARATOR ";"
#else
#define MODULE_SEPARATOR ":"
#endif

#ifndef MAX_PATH
#define MAX_PATH 512
#endif

#ifdef USE_BREAKPAD
#include "client/windows/handler/exception_handler.h"
#include "common/windows/http_upload.h"
#endif

class KrollBoot
{
protected:
	int argc;
	const char** argv;

	SharedApplication app;
	string updateFile;
public:

	KrollBoot(int _argc, const char ** _argv);
	virtual ~KrollBoot();
	/**
	 * Implemented platform independently
	 */
	int Bootstrap();
	

protected:
	void FindUpdate();
	vector<SharedDependency> FilterForSDKInstall(
		vector<SharedDependency> dependencies);

	/**
	 * Implemented platform specifically
	 */	
	virtual int StartHost()=0;
	virtual string Blastoff()=0;
	virtual void BootstrapPlatformSpecific(string moduleList)=0;
	virtual string GetApplicationName() const=0;
	virtual void ShowError(const string & msg, bool fatal=false) const=0;
	virtual std::string GetApplicationHomePath() const=0;
	virtual bool RunInstaller(vector<SharedDependency> missing, bool forceInstall=false)=0;
};


#ifdef USE_BREAKPAD
class CrashReporter
{
public:
	static int argc;
	static const char** argv;
	static string applicationHome;
	static SharedApplication app;
	static string dumpFilePath;

	static void InitCrashDetection();
	static string GetCrashDetectionTitle();
	static string GetCrashDetectionHeader();
	static string GetCrashDetectionMessage();
	static void GetCrashReportParameters(map<string, string> & param);
	static string GetApplicationName();

	// TODO: Windows Specific
	static string GetApplicationHomePath();
	static wchar_t breakpadCallBuffer[MAX_PATH];
	static google_breakpad::ExceptionHandler* breakpad;
	static bool HandleCrash(
		const wchar_t* dumpPath,
		const wchar_t* id,
		void* context,
		EXCEPTION_POINTERS* exinfo,
		MDRawAssertionInfo* assertion,
		bool succeeded);
	
	static wstring StringToWString(string in);
	static void GetCrashReportParametersW(map<wstring, wstring> & paramsW);
	static int SendCrashReport();

};
#endif // CRASH_REPORTER

#endif
