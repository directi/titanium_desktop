/*
class BootLoaderWin32 - win32 subclass of BootLoader
@author: Mital Vora <mital.d.vora@gmail.com>
*/

#ifndef _KROLL_WIN32_BOOT_H_
#define _KROLL_WIN32_BOOT_H_

#include <process.h>
#include <windows.h>

#include "boot.h"

#include <win32/win32_utils.h>

#ifndef MAX_PATH
#define MAX_PATH 512
#endif

#ifdef USE_BREAKPAD
#include "client/windows/handler/exception_handler.h"
#include "common/windows/http_upload.h"
#endif

class BootLoaderWin32
	: public BootLoader
{
	public:
		BootLoaderWin32(int _argc, const char ** _argv);
		virtual ~BootLoaderWin32();
		virtual int StartHost();

	private:
		virtual string Blastoff();
		virtual void ShowErrorImpl(const string & msg, bool fatal) const;
		virtual void setPlatformSpecificPaths(const std::string & runtime_path, const std::string & module_paths);

		HMODULE SafeLoadRuntimeDLL(string& path) const;
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
