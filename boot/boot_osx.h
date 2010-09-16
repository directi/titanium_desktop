
#ifndef _BOOT_OSX_H_
#define _BOOT_OSX_H_

#include "boot.h"

#ifdef USE_BREAKPAD
#import "client/mac/handler/exception_handler.h"
#import "common/mac/HTTPMultipartUpload.h"
#include <Foundation/Foundation.h>
#endif

class KrollOSXBoot
: public KrollBoot
{
	public:
		KrollOSXBoot(int _argc, const char ** _argv);
		virtual ~KrollOSXBoot();
		virtual int StartHost();
	private:
		virtual void ShowError(const string & msg, bool fatal = false) const;
		virtual string GetApplicationName() const;

		virtual string Blastoff();
		virtual std::string GetApplicationHomePath() const;
		virtual void BootstrapPlatformSpecific(const std::string & moduleList);
		virtual bool RunInstaller(vector<SharedDependency> missing, bool forceInstall=false);
};

#ifdef USE_BREAKPAD
class OSXCrashHandler
: public CrashHandler
{
	public:
		OSXCrashHandler(int _argc, const char ** _argv);
		~OSXCrashHandler();

		virtual int SendCrashReport();
		void createHandler(const std::string & tempPath);

	private:
		google_breakpad::ExceptionHandler* breakpad;

		static string app_exe_name;
		virtual string GetApplicationHomePath() const;
		static char breakpadCallBuffer[PATH_MAX];

		static bool HandleCrash(const char* dumpPath,
				const char* dumpId,
				void* context,
				bool succeeded);

		//static wstring StringToWString(string in);
		//void GetCrashReportParametersW(map<wstring, wstring> & paramsW);
};

#endif
#endif // _BOOT_OSX_H_
