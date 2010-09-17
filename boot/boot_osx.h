
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

		virtual string Blastoff();
		virtual void BootstrapPlatformSpecific(const std::string & moduleList);

		virtual string GetApplicationName() const;
		virtual std::string GetApplicationHomePath() const;
		virtual void ShowErrorImpl(const string & msg, bool fatal) const;
		virtual bool RunInstaller(vector<SharedDependency> missing, bool forceInstall=false) const;
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

		virtual string GetApplicationHomePath() const;
		static char breakpadCallBuffer[PATH_MAX];

		static bool HandleCrash(const char* dumpPath,
				const char* dumpId,
				void* context,
				bool succeeded);
};

#endif
#endif // _BOOT_OSX_H_
