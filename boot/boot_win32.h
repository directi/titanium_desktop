/*
class KrollWin32Boot - win32 subclass of KrollBoot
@author: Mital Vora <mital.d.vora@gmail.com>
*/

#ifndef _KROLL_WIN32_BOOT_H_
#define _KROLL_WIN32_BOOT_H_

#include "boot.h"

#ifndef MAX_PATH
#define MAX_PATH 512
#endif

#ifdef USE_BREAKPAD
#include "client/windows/handler/exception_handler.h"
#include "common/windows/http_upload.h"
#endif

class KrollWin32Boot
	: public KrollBoot
{
	public:
		KrollWin32Boot(int _argc, const char ** _argv);
		virtual ~KrollWin32Boot();

		virtual int StartHost();

	private:
		virtual string Blastoff();
		virtual void BootstrapPlatformSpecific(const std::string & moduleList);

		HMODULE SafeLoadRuntimeDLL(string& path) const;

		virtual string GetApplicationName() const;
		virtual std::string GetApplicationHomePath() const;
		virtual void ShowErrorImpl(const string & msg, bool fatal) const;
		virtual bool RunInstaller(vector<SharedDependency> missing, bool forceInstall=false) const;

};

#ifdef USE_BREAKPAD
class Win32CrashHandler
: public CrashHandler
{
	public:
		Win32CrashHandler(int _argc, const char ** _argv);
		~Win32CrashHandler();

		virtual int SendCrashReport();
		void createHandler(wchar_t tempPath[MAX_PATH]);

	private:
		google_breakpad::ExceptionHandler* breakpad;

		virtual string GetApplicationHomePath() const;
		static wchar_t breakpadCallBuffer[MAX_PATH];

		static bool HandleCrash(
				const wchar_t* dumpPath,
				const wchar_t* id,
				void* context,
				EXCEPTION_POINTERS* exinfo,
				MDRawAssertionInfo* assertion,
				bool succeeded);

		static wstring StringToWString(string in);
		void GetCrashReportParametersW(map<wstring, wstring> & paramsW);
};

#endif

#endif // _KROLL_WIN32_BOOT_H_
