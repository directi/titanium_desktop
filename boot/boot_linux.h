
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
		
		virtual string Blastoff();
		virtual void BootstrapPlatformSpecific(const std::string & moduleList);

		virtual string GetApplicationName() const;
		virtual void ShowErrorImpl(const string & msg, bool fatal) const;
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
		int argc;
		const char ** argv;
		google_breakpad::ExceptionHandler* breakpad;

		static char breakpadCallBuffer[PATH_MAX];

		static bool HandleCrash(const char* dumpPath,
				const char* dumpId,
				void* context,
				bool succeeded);
};

#endif
#endif // _BOOT_OSX_H_
