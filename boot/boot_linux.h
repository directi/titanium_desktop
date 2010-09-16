
#ifndef _BOOT_OSX_H_
#define _BOOT_OSX_H_

#include "boot.h"

#ifdef USE_BREAKPAD
#include "client/linux/handler/exception_handler.h"
#include "common/linux/http_upload.h"
#endif

class KrollLinuxBoot
: public KrollBoot
{
	public:
		KrollLinuxBoot(int _argc, const char ** _argv);
		virtual ~KrollLinuxBoot();
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
class LinuxCrashHandler
: public CrashHandler
{
	public:
		LinuxCrashHandler(int _argc, const char ** _argv);
		~LinuxCrashHandler();

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
