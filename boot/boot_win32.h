/*
class KrollWin32Boot - win32 subclass of KrollBoot
@author: Mital Vora <mital.d.vora@gmail.com>
*/

#ifndef _KROLL_WIN32_BOOT_H_
#define _KROLL_WIN32_BOOT_H_

#include "boot.h"

class KrollWin32Boot
	: public KrollBoot
{
	public:
		KrollWin32Boot(int _argc, const char ** _argv);

	private:
	bool IsWindowsXP() const;
	HMODULE SafeLoadRuntimeDLL(string& path) const;

	virtual int StartHost();
	virtual void ShowError(const string & msg, bool fatal = false) const;
	virtual string GetApplicationName() const;
	
	virtual string Blastoff();
	std::string GetApplicationHomePath() const;
	void BootstrapPlatformSpecific(string moduleList);
	virtual bool RunInstaller(vector<SharedDependency> missing, bool forceInstall=false);

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

	static string app_exe_name;
	virtual string GetApplicationHomePath();
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
