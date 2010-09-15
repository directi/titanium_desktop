/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008-2009 Appcelerator, Inc. All Rights Reserved.
 */

#include "boot_win32.h"
#include "popup_dialog_win32.h"

#include <process.h>
#include <windows.h>


int CrashReporter::argc;
const char** CrashReporter::argv;


	typedef int Executor(int, const char **);

	KrollWin32Boot::KrollWin32Boot(int _argc, const char ** _argv)
		: KrollBoot(_argc, _argv)
	{
	}

	bool KrollWin32Boot::IsWindowsXP() const
	{
		OSVERSIONINFO osVersion;
		osVersion.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		::GetVersionEx(&osVersion);
		return osVersion.dwMajorVersion == 5;
	}

	HMODULE KrollWin32Boot::SafeLoadRuntimeDLL(string& path) const
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
			string msg("Couldn't load file (");
			msg.append(path);
			msg.append("): ");
			msg.append(KrollUtils::Win32Utils::QuickFormatMessage(GetLastError()));
			ShowError(msg);
		}

		return module;
	}

	int KrollWin32Boot::StartHost()
	{
		string runtimePath(EnvironmentUtils::Get("KR_RUNTIME"));
		string dll(FileUtils::Join(runtimePath.c_str(), "khost.dll", 0));
		HMODULE khost = SafeLoadRuntimeDLL(dll);
		if (!khost)
			return __LINE__;

		Executor *executor = (Executor*) GetProcAddress(khost, "Execute");
		if (!executor)
		{
			ShowError(string("Invalid entry point 'Execute' in khost.dll"));
			return __LINE__;
		}

		return executor(argc, argv);
	}


	void KrollWin32Boot::ShowError(const string & msg, bool fatal) const
	{
		wstring wideMsg(L"Error: ");
		wideMsg.append(KrollUtils::UTF8ToWide(msg));
		wstring wideAppName = KrollUtils::UTF8ToWide(GetApplicationName());

		MessageBoxW(0, wideMsg.c_str(), wideAppName.c_str(), MB_OK|MB_ICONERROR|MB_SYSTEMMODAL);
		if (fatal)
		{
			exit(1);
		}
	}

	string KrollWin32Boot::GetApplicationHomePath() const
	{
		wchar_t widePath[MAX_PATH];
		int size = GetModuleFileNameW(GetModuleHandle(0), widePath, MAX_PATH - 1);
		if (size > 0)
		{
			widePath[size] = '\0';
			string path = KrollUtils::WideToUTF8(widePath);
			return FileUtils::Dirname(path);
		}
		else
		{
			ShowError("Could not determine application path.", true);
		}
	}

	bool KrollWin32Boot::RunInstaller(vector<SharedDependency> missing, bool forceInstall)
	{

		string installer(FileUtils::Join(app->path.c_str(), "installer", "installer.exe", 0));
		if (!FileUtils::IsFile(installer))
		{
			ShowError("Missing installer and application has additional modules that are needed.");
			return false;
		}
		return BootUtils::RunInstaller(missing, app, updateFile, "", false, forceInstall);
	}

	void KrollWin32Boot::BootstrapPlatformSpecific(string path)
	{
		// Add runtime path and all module paths to PATH
		path = app->runtime->path + ";" + path;
		string currentPath(EnvironmentUtils::Get("PATH"));
		EnvironmentUtils::Set("KR_ORIG_PATH", currentPath);

		// make sure the runtime folder is used before system DLL directories
		SetDllDirectoryW(KrollUtils::UTF8ToWide(app->runtime->path).c_str());
		
		if (!currentPath.empty())
			path = path + ";" + currentPath;
		EnvironmentUtils::Set("PATH", path);
	}

	string KrollWin32Boot::Blastoff()
	{
		// Windows boot does not normally need to restart itself,  so just
		// launch the host here and exit with the appropriate return value.

		// This may have been an install, so ensure that KR_HOME is correct
		EnvironmentUtils::Set("KR_HOME", app->path);
		exit(StartHost());
	}


	string KrollWin32Boot::GetApplicationName() const
	{
		if (!app.isNull())
		{
			return app->name.c_str();
		}
		return PRODUCT_NAME;
	}


#ifdef USE_BREAKPAD
	google_breakpad::ExceptionHandler* CrashReporter::breakpad;
	wchar_t CrashReporter::breakpadCallBuffer[MAX_PATH] = {0};

	string CrashReporter::GetApplicationHomePath()
	{
		wchar_t widePath[MAX_PATH];
		int size = GetModuleFileNameW(GetModuleHandle(0), widePath, MAX_PATH - 1);
		if (size > 0)
		{
			widePath[size] = '\0';
			string path = KrollUtils::WideToUTF8(widePath);
			return FileUtils::Dirname(path);
		}
	}

	bool CrashReporter::HandleCrash(
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
				argv[0], CRASH_REPORT_OPT, dumpPath, id);

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

	wstring CrashReporter::StringToWString(string in)
	{
		wstring out(in.length(), L' ');
		copy(in.begin(), in.end(), out.begin());
		return out;
	}

	void CrashReporter::GetCrashReportParametersW(map<wstring, wstring> & paramsW)
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

	int CrashReporter::SendCrashReport()
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
//			KrollBoot::ShowError("Error uploading crash dump.");
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
	KrollWin32Boot boot(__argc, __argv);

#ifdef USE_BREAKPAD
	// Don't install a handler if we are just handling an error.
	if (__argc > 2 && !strcmp(CRASH_REPORT_OPT, __argv[1]))
	{
		CrashReporter::argc = __argc;
		CrashReporter::argv = __argv;
		return CrashReporter::SendCrashReport();
	}

	wchar_t tempPath[MAX_PATH];
	GetTempPathW(MAX_PATH, tempPath);
	CrashReporter::breakpad = new google_breakpad::ExceptionHandler(
		tempPath,
		0,
		CrashReporter::HandleCrash,
		0,
		google_breakpad::ExceptionHandler::HANDLER_ALL);
#endif

	return boot.Bootstrap();
}
