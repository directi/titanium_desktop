/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008-2009 Appcelerator, Inc. All Rights Reserved.
 */


#include <boot_utils.h>
#include <file_utils.h>
#include <environment_utils.h>

#include "boot_win32.h"
#include "popup_dialog_win32.h"

#define HOST_DLL "khost.dll"

using namespace UTILS_NS;


BootLoaderWin32::BootLoaderWin32(int _argc, const char ** _argv)
: BootLoader(_argc, _argv)
{
}

BootLoaderWin32::~BootLoaderWin32()
{
}

HMODULE BootLoaderWin32::SafeLoadRuntimeDLL(string& path) const
{
	if (!FileUtils::IsFile(path))
	{
		ShowError(string("Couldn't find required file: ") + path);
		return false;
	}

	wstring widePath(KrollUtils::UTF8ToWide(path));
	HMODULE module = LoadLibraryExW(widePath.c_str(),
			0, LOAD_WITH_ALTERED_SEARCH_PATH);
	if (!module)
	{
		const string msg = string("Couldn't load file (")
			+ path + "): "
			+ KrollUtils::Win32Utils::QuickFormatMessage(GetLastError());
		ShowError(msg);
	}
	return module;
}

int BootLoaderWin32::StartHost()
{
	string runtimePath(EnvironmentUtils::Get(RUNTIME_ENV));
	string host_path(FileUtils::Join(runtimePath.c_str(), HOST_DLL, 0));
	HMODULE khost = SafeLoadRuntimeDLL(host_path);
	if (!khost)
		return __LINE__;

	Executor *executor = (Executor*) GetProcAddress(khost, "Execute");
	if (!executor)
	{
		ShowError(string("Invalid entry point 'Execute' in ") + host_path);
		return __LINE__;
	}

	return executor(argc, argv);
}


void BootLoaderWin32::ShowErrorImpl(const string & msg, bool fatal) const
{
	wstring wideMsg(L"Error: ");
	wideMsg.append(KrollUtils::UTF8ToWide(msg));
	wstring wideAppName = KrollUtils::UTF8ToWide(PRODUCT_NAME);

	MessageBoxW(0, wideMsg.c_str(), wideAppName.c_str(), MB_OK|MB_ICONERROR|MB_SYSTEMMODAL);
}


void BootLoaderWin32::setPlatformSpecificPaths(const std::string & runtime_path, const std::string & module_paths)
{
	// Add runtime path and all module paths to PATH
	std::string newpath = runtime_path + ";" + module_paths;
	string currentPath(EnvironmentUtils::Get("PATH"));
	EnvironmentUtils::Set("KR_ORIG_PATH", currentPath);

	// make sure the runtime folder is used before system DLL directories
	SetDllDirectoryW(KrollUtils::UTF8ToWide(runtime_path).c_str());

	if (!currentPath.empty())
		newpath = newpath + ";" + currentPath;
	EnvironmentUtils::Set("PATH", newpath);
}

string BootLoaderWin32::Blastoff()
{
	// Windows boot does not normally need to restart itself,  so just
	// launch the host here and exit with the appropriate return value.
	exit(StartHost());
}

#ifdef USE_BREAKPAD

wchar_t Win32CrashHandler::breakpadCallBuffer[MAX_PATH]= {0};

Win32CrashHandler::Win32CrashHandler(int _argc, const char ** _argv)
: CrashHandler(_argc, _argv), breakpad(0)
{
}

Win32CrashHandler::~Win32CrashHandler()
{
}


bool Win32CrashHandler::HandleCrash(
		const wchar_t* dumpPath,
		const wchar_t* id,
		void* context,
		EXCEPTION_POINTERS* exinfo,
		MDRawAssertionInfo* assertion,
		bool succeeded)
{
	if (succeeded)
	{
		STARTUPINFOW startupInfo = {0};
		startupInfo.cb = sizeof(startupInfo);
		PROCESS_INFORMATION processInformation;

		_snwprintf(breakpadCallBuffer, MAX_PATH - 1, L"\"%S\" \"%S\" %s %s",
				CrashHandler::executable_name.c_str(), CRASH_REPORT_OPT, dumpPath, id);

		CreateProcessW(
				0,
				breakpadCallBuffer,
				0,
				0,
				FALSE,
				0,
				0,
				0,
				&startupInfo,
				&processInformation);
	}

	// We would not normally need to do this, but on Windows XP it
	// seems that this callback is called multiple times for a crash.
	// We should probably try to remove the following line the next
	// time we update breakpad.
	exit(__LINE__);
	return true;
}

wstring Win32CrashHandler::StringToWString(string in)
{
	wstring out(in.length(), L' ');
	copy(in.begin(), in.end(), out.begin());
	return out;
}

void Win32CrashHandler::GetCrashReportParametersW(map<wstring, wstring> & paramsW)
{
	map<string, string> params;
	GetCrashReportParameters(params);
	map<string, string>::iterator i = params.begin();
	while (i != params.end())
	{
		wstring key = StringToWString(i->first);
		wstring val = StringToWString(i->second);
		i++;

		paramsW[key] = val;
	}
}

void Win32CrashHandler::createHandler(wchar_t tempPath[MAX_PATH])
{
	breakpad = new google_breakpad::ExceptionHandler(
			tempPath,
			0,
			Win32CrashHandler::HandleCrash,
			0,
			google_breakpad::ExceptionHandler::HANDLER_ALL);
}

int Win32CrashHandler::SendCrashReport()
{
	InitCrashDetection();
	string title = GetCrashDetectionTitle();
	string msg = GetCrashDetectionHeader();
	msg.append("\n\n");
	msg.append(GetCrashDetectionMessage());

	Win32PopupDialog popupDialog(0);
	popupDialog.SetTitle(title);
	popupDialog.SetMessage(msg);
	popupDialog.SetShowCancelButton(true);
	if (popupDialog.Show() != IDYES)
	{
		return __LINE__;
	}

	wstring url = L"http://";
	url += StringToWString(CRASH_REPORT_URL);

	std::map<wstring, wstring> parameters;
	GetCrashReportParametersW(parameters);
	wstring dumpFilePathW = StringToWString(dumpFilePath);
	wstring responseBody;
	int responseCode;

	bool success = google_breakpad::HTTPUpload::SendRequest(
			url,
			parameters,
			dumpFilePathW.c_str(),
			L"dump",
			0,
			&responseBody,
			&responseCode);

	if (!success)
	{
		//#ifdef DEBUG
		//			BootLoader::ShowError("Error uploading crash dump.");
		//#endif
		return __LINE__;
	}
#ifdef DEBUG
	else
	{
		MessageBoxW(0,L"Your crash report has been submitted. Thank You!",L"Error Reporting Status",MB_OK | MB_ICONINFORMATION);
	}
#endif
	return 0;
}
#endif

#if defined(OS_WIN32) && !defined(WIN32_CONSOLE)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR command_line, int)
#else
int main(int __argc, const char* __argv[])
#endif
{
#ifdef USE_BREAKPAD
	Win32CrashHandler reporter(__argc,(const char **) __argv);
	// Don't install a handler if we are just handling an error.
	if (__argc > 2 && !strcmp(CRASH_REPORT_OPT, __argv[1]))
	{
		return reporter.SendCrashReport();
	}

	wchar_t tempPath[MAX_PATH];
	GetTempPathW(MAX_PATH, tempPath);
	reporter.createHandler(tempPath);
#endif

	BootLoaderWin32 boot(__argc, (const char **)__argv);
	return boot.Bootstrap();
}
