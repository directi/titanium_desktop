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

typedef int Executor(int argc, const char ** argv);

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
	virtual int StartHost()=0;

protected:
	void FindUpdate();
	vector<SharedDependency> FilterForSDKInstall(
		vector<SharedDependency> dependencies);

	/**
	 * Implemented platform specifically
	 */	
	virtual string Blastoff()=0;
	virtual void BootstrapPlatformSpecific(const std::string & moduleList)=0;
	virtual string GetApplicationName() const=0;
	virtual void ShowError(const string & msg, bool fatal=false) const=0;
	virtual std::string GetApplicationHomePath() const=0;
	virtual bool RunInstaller(vector<SharedDependency> missing, bool forceInstall=false)=0;
};


#ifdef USE_BREAKPAD
class CrashHandler
{
public:
	CrashHandler(int _argc, const char ** _argv);
	virtual ~CrashHandler();

	virtual int SendCrashReport()=0;

protected:
	int argc;
	const char** argv;
	static string applicationHome;
	SharedApplication app;
	static string dumpFilePath;
	static string executable_name;


	void InitCrashDetection();
	static string GetCrashDetectionTitle();
	static string GetCrashDetectionHeader();
	static string GetCrashDetectionMessage();
	void GetCrashReportParameters(map<string, string> & param);
	static string GetApplicationName();

	virtual string GetApplicationHomePath() const=0;
};
#endif // USE_BREAKPAD

#endif
