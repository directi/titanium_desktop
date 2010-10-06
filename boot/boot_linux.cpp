/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008-2009 Appcelerator, Inc. All Rights Reserved.
 */
#include <dlfcn.h>
#include <libgen.h>
#include <limits.h>
#include <errno.h>
#include <gtk/gtk.h>
#include "boot.h"
#include "boot_linux.h"


KrollLinuxBoot::KrollLinuxBoot(int _argc, const char ** _argv)
: KrollBoot(_argc, _argv)
{
}

KrollLinuxBoot::~KrollLinuxBoot()
{
}
void KrollLinuxBoot::ShowErrorImpl(const string & msg, bool fatal) const
{
	std::cout << msg << std::endl;
	int myargc = argc;
	gtk_init(&myargc, (char***) &argv);
	GtkWidget* dialog = gtk_message_dialog_new(
		0,
		GTK_DIALOG_MODAL,
		GTK_MESSAGE_ERROR,
		GTK_BUTTONS_CLOSE,
		"%s",
		msg.c_str());
	gtk_window_set_title(GTK_WINDOW(dialog), GetApplicationName().c_str());
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
}


void KrollLinuxBoot::BootstrapPlatformSpecific(const std::string & moduleList)
{
	std::string fullmoduleList = app->getRuntimePath() + ":" + moduleList;

	string path(fullmoduleList);
	string current(EnvironmentUtils::Get("LD_LIBRARY_PATH"));
	EnvironmentUtils::Set("KR_ORIG_LD_LIBRARY_PATH", current);

	if (!current.empty())
	{
		path.append(":" + current);
	}

	EnvironmentUtils::Set("LD_LIBRARY_PATH", path);
}

string KrollLinuxBoot::Blastoff()
{
	// Ensure that the argument list is NULL terminated
	char** myargv = (char **) calloc(sizeof(char *), argc + 1);
	memcpy(myargv, argv, sizeof(char*) * (argc + 1));
	myargv[argc] = 0;

	execv(argv[0], (char* const*) argv);

	// If we get here an error happened with the execv 
	return strerror(errno);
}

int KrollLinuxBoot::StartHost()
{
	const char* runtimePath = getenv("KR_RUNTIME");
	if (!runtimePath)
		return __LINE__;

	// now we need to load the host and get 'er booted
	string khost = FileUtils::Join(runtimePath, "libkhost.so", 0);
	if (!FileUtils::IsFile(khost))
	{
		string msg = string("Couldn't find required file:") + khost;
		ShowError(msg);
		return __LINE__;
	}

	void* lib = dlopen(khost.c_str(), RTLD_LAZY | RTLD_GLOBAL);
	if (!lib)
	{
		string msg = string("Couldn't load file:") + khost + ", error: " + dlerror();
		ShowError(msg);
		return __LINE__;
	}

	Executor* executor = (Executor*) dlsym(lib, "Execute");
	if (!executor)
	{
		string msg = string("Invalid entry point for") + khost;
		ShowError(msg);
		return __LINE__;
	}

	return executor(argc, argv);
}


string KrollLinuxBoot::GetApplicationName() const
{
	if (!app.isNull())
	{
		return app->getName().c_str();
	}
	return PRODUCT_NAME;
}

#ifdef USE_BREAKPAD

char LinuxCrashHandler::breakpadCallBuffer[PATH_MAX];

LinuxCrashHandler::LinuxCrashHandler(int _argc, const char ** _argv)
	: CrashHandler(_argc, _argv), argc(_argc), argv(_argv), breakpad(0)
{
}

LinuxCrashHandler::~LinuxCrashHandler()
{
}
	
bool LinuxCrashHandler::HandleCrash(
	const char* dump_path,
	const char* id,
	void* context,
	bool succeeded)
{
	if (succeeded)
	{
		snprintf(breakpadCallBuffer, PATH_MAX - 1,
			 "\"%s\" %s \"%s\" %s&", CrashHandler::executable_name.c_str(), CRASH_REPORT_OPT, dump_path, id);
		system(breakpadCallBuffer);
	}
#ifdef DEBUG
	return false;
#else
	return true;
#endif
}

void LinuxCrashHandler::createHandler(const std::string & tempPath)
{
	// Our blastoff execv seems to fail if this handler is installed
	// for the first stage of the boot -- so install it here.
	breakpad = new google_breakpad::ExceptionHandler(
			tempPath, 0, LinuxCrashHandler::HandleCrash, 0, true);
	breakpad = new google_breakpad::ExceptionHandler(
		tempPath,
		0,
		LinuxCrashHandler::HandleCrash,
		0,
		true);

}


int LinuxCrashHandler::SendCrashReport()
{
	gtk_init(&argc, (char***) &argv);

	InitCrashDetection();
	std::string title = GetCrashDetectionTitle();
	std::string msg = GetCrashDetectionHeader();
	msg.append("\n\n");
	msg.append(GetCrashDetectionMessage());

	string url = string("http://") + CRASH_REPORT_URL;
	std::map<string, string> parameters;
	GetCrashReportParameters(parameters);

	GtkWidget* dialog = gtk_message_dialog_new(
		0,
		GTK_DIALOG_MODAL,
		GTK_MESSAGE_ERROR,
		GTK_BUTTONS_NONE,
		"%s",
		msg.c_str());
	gtk_dialog_add_buttons(GTK_DIALOG(dialog),
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
		"Send Report", GTK_RESPONSE_OK,
		0);
	gtk_window_set_title(GTK_WINDOW(dialog), title.c_str());
	int response = gtk_dialog_run(GTK_DIALOG(dialog));
	if (response != GTK_RESPONSE_OK)
	{
		return __LINE__;
	}

	string filePartName = "dump";
	string proxy;
	string proxyUserPassword;
	string responseBody;
	string errorDescription;
	bool success = google_breakpad::HTTPUpload::SendRequest(
		url, parameters, dumpFilePath.c_str(), filePartName,
		proxy, proxyUserPassword, &responseBody, &errorDescription);

	if (!success)
	{
		std::cerr << "Error uploading crash dump: " << errorDescription << std::endl;
		return __LINE__;
	}
	return 0;
}
#endif


int main(int argc, const char* argv[])
{
	KrollLinuxBoot bootloader(argc, argv);
#ifdef USE_BREAKPAD
	LinuxCrashHandler handler(argc, argv);
	if (argc > 2 && !strcmp(CRASH_REPORT_OPT, argv[1]))
	{
		return handler.SendCrashReport();
	}

	// Don't install a handler if we are just handling an error (above).
	string dumpPath = "/tmp";
	handler.createHandler(dumpPath);
#endif

	if (!EnvironmentUtils::Has(BOOTSTRAP_ENV))
	{
		return bootloader.Bootstrap();
	}
	EnvironmentUtils::Unset(BOOTSTRAP_ENV);
	return bootloader.StartHost();
}

