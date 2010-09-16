/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008-2009 Appcelerator, Inc. All Rights Reserved.
 */
#import <Cocoa/Cocoa.h>
#import <dlfcn.h>
#import "boot.h"
#import "boot_osx.h"


@interface KrollApplicationDelegate : NSObject
-(BOOL)application:(NSApplication*)theApplication
	openFile:(NSString*)filename;
-(BOOL)application:(NSApplication*)theApplication
	openFiles:(NSArray*)filenames;
@end

@implementation KrollApplicationDelegate
-(BOOL)application:(NSApplication*)theApplication openFile:(NSString*)filename
{
	return YES;
}

-(BOOL)application:(NSApplication*)theApplication openFiles:(NSArray*)filenames
{
	return YES;
}
@end


KrollOSXBoot::KrollOSXBoot(int _argc, const char ** _argv)
: KrollBoot(_argc, _argv)
{
}

KrollOSXBoot::~KrollOSXBoot()
{
}

void KrollOSXBoot::ShowError(const std::string& msg, bool fatal) const
{
	NSApplicationLoad();

	NSString* buttonText = @"Continue";
	if (fatal)
		buttonText = @"Quit";

	NSRunCriticalAlertPanel(
		[NSString stringWithUTF8String:GetApplicationName().c_str()],
		[NSString stringWithUTF8String:msg.c_str()], buttonText, nil, nil);

	if (fatal)
		exit(1);
}

string KrollOSXBoot::GetApplicationHomePath() const
{
	NSString *bundle = [[NSBundle mainBundle] bundlePath];
	NSString *contents = [NSString stringWithFormat:@"%@/Contents", bundle];
	return std::string([contents UTF8String]);
}

void KrollOSXBoot::BootstrapPlatformSpecific(const string & moduleList)
{
	std::string fullModuleList = app->runtime->path + ":" + moduleList;

	string path(fullModuleList);
	string currentFWPath(EnvironmentUtils::Get("DYLD_FRAMEWORK_PATH"));
	EnvironmentUtils::Set("KR_ORIG_DYLD_FRAMEWORK_PATH", currentFWPath);
	if (!currentFWPath.empty())
		path = path + ":" + currentFWPath;

	EnvironmentUtils::Set("DYLD_FRAMEWORK_PATH", path);

	path = fullModuleList;
	string currentLibPath(EnvironmentUtils::Get("DYLD_LIBRARY_PATH"));
	EnvironmentUtils::Set("KR_ORIG_DYLD_LIBRARY_PATH", currentLibPath);
	if (!currentLibPath.empty())
		path = path + ":" + currentLibPath;

	EnvironmentUtils::Set("DYLD_LIBRARY_PATH", path);

	const char* executablePath = 
		[[[NSBundle mainBundle] executablePath] fileSystemRepresentation];
	EnvironmentUtils::Set("WEBKIT_UNSET_DYLD_FRAMEWORK_PATH", "YES");
	EnvironmentUtils::Set("WebKitAppPath", executablePath);
}

string KrollOSXBoot::Blastoff()
{
	// Ensure that the argument list is NULL terminated
	char** myargv = (char **) calloc(sizeof(char *), argc + 1);
	memcpy(myargv, argv, sizeof(char*) * (argc + 1));
	myargv[argc] = 0;

	NSString *executablePath = [[NSBundle mainBundle] executablePath];
	execv([executablePath fileSystemRepresentation], myargv);

	// If we get here an error happened with the execv 
	return strerror(errno);
}

int KrollOSXBoot::StartHost()
{
	// now we need to load the host and get 'er booted
	const char* runtimePath = getenv("KR_RUNTIME");
	if (!runtimePath)
		return __LINE__;

	std::string khost = FileUtils::Join(runtimePath, "libkhost.dylib", 0);
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

	Executor *executor = (Executor*)dlsym(lib, "Execute");
	if (!executor)
	{
		string msg = string("Invalid entry point for") + khost;
		ShowError(msg);
		return __LINE__;
	}
	return executor(argc, (const char**)argv);
}

bool KrollOSXBoot::RunInstaller(vector<SharedDependency> missing, bool forceInstall)
{
	string exec = FileUtils::Join(
			app->path.c_str(),
			"installer",
			"Installer App.app",
			"Contents", 
			"MacOS",
			"Installer App", 0);
	if (!FileUtils::IsFile(exec))
	{
		ShowError("Missing installer and application has additional modules that are needed.");
		return false;
	}

	return BootUtils::RunInstaller(missing, app, updateFile);
}

string KrollOSXBoot::GetApplicationName() const
{
	if (!app.isNull())
	{
		return app->name.c_str();
	}
	else
	{
		// fall back to the info.plist if we haven't loaded the application
		// which happens in a crash situation
		NSDictionary *infoDictionary = [[NSBundle mainBundle] infoDictionary];
		NSString *applicationName = [infoDictionary objectForKey:@"CFBundleName"];
		if (!applicationName) 
		{
			applicationName = [infoDictionary objectForKey:@"CFBundleExecutable"];
		}
		return [applicationName UTF8String];
	}
}

#ifdef USE_BREAKPAD
	// Allocate this statically because after a crash we want to access
	// the heap as little as possible.
//	google_breakpad::ExceptionHandler* breakpad;

char OSXCrashHandler::breakpadCallBuffer[PATH_MAX];

OSXCrashHandler::OSXCrashHandler(int _argc, const char ** _argv)
	: CrashHandler(_argc, _argv), breakpad(0)
{
}

OSXCrashHandler::~OSXCrashHandler()
{
}


bool OSXCrashHandler::HandleCrash(const char* dumpPath, const char* dumpId, void* context, bool succeeded)
{
	if (succeeded)
	{
		snprintf(breakpadCallBuffer, PATH_MAX - 1,
				"\"%s\" %s \"%s\" %s &", CrashHandler::executable_name.c_str(), CRASH_REPORT_OPT, dumpPath, dumpId);
		system(breakpadCallBuffer);
	}
	return true;
}

void OSXCrashHandler::createHandler(const std::string &tempPath)
{
	// Our blastoff execv seems to fail if this handler is installed
	// for the first stage of the boot -- so install it here.
	breakpad = new google_breakpad::ExceptionHandler(
			tempPath, 0, OSXCrashHandler::HandleCrash, 0, true);

}

int OSXCrashHandler::SendCrashReport()
{
	if (argc < 3)
	{
		//ShowError("Invalid number of arguments passed to crash reporter.");
		std::cerr << "Invalid number of arguments passed to crash reporter." << std::endl;
		return __LINE__;
	}

	InitCrashDetection();
	NSApplicationLoad();
	NSAlert* alert = [[[NSAlert alloc] init] autorelease];
	[alert setMessageText: 
		[NSString stringWithUTF8String:GetCrashDetectionHeader().c_str()]];
	[alert setInformativeText:
		[NSString stringWithUTF8String:GetCrashDetectionMessage().c_str()]];
	[alert addButtonWithTitle:@"Send Report"];
	[alert addButtonWithTitle:@"Cancel"];
	int response = [alert runModal];

	if (response == NSAlertFirstButtonReturn)
	{
		map<string, string> params;
		GetCrashReportParameters(params);

		NSMutableDictionary* nsParams = [[NSMutableDictionary alloc] init];
		map<string, string>::iterator i = params.begin();
		while (i != params.end())
		{
			NSString* key = [NSString stringWithUTF8String:i->first.c_str()];
			NSString* value = [NSString stringWithUTF8String:i->second.c_str()];
#ifdef DEBUG
			NSLog(@"key = %@, value = %@",key,value);
#endif
			[nsParams setObject:value forKey:key];
			i++;
		}

		NSURL* url = [NSURL URLWithString:[NSString stringWithFormat: @"http://%s", CRASH_REPORT_URL]];
		NSLog(@"Sending crash report to %@",url);
		HTTPMultipartUpload* uploader = [[HTTPMultipartUpload alloc] initWithURL:url];
		[uploader addFileAtPath:[NSString stringWithUTF8String:dumpFilePath.c_str()] name:@"dump"];
		[uploader setParameters:nsParams];

		NSError* error;
		[uploader send:&error];
		if (error)
		{
			string msg = "An error occured while attempting sending the crash report: ";
			msg += [[error localizedDescription] UTF8String];
			//ShowError(msg);
			std::cerr << msg << std::endl;
			return __LINE__;
		}
	}

	return 0;
}

string OSXCrashHandler::GetApplicationHomePath() const
{
	NSString *bundle = [[NSBundle mainBundle] bundlePath];
	NSString *contents = [NSString stringWithFormat:@"%@/Contents", bundle];
	return std::string([contents UTF8String]);
}
#endif

int main(int argc, const char* argv[])
{
	NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
	KrollOSXBoot bootloader(argc, argv);
	int rc = 0;

#ifdef USE_BREAKPAD
	OSXCrashHandler crashHandler(argc, argv);
	if (argc > 2 && !strcmp(CRASH_REPORT_OPT, argv[1]))
	{
		crashHandler.SendCrashReport();
	}
	else
#endif
	if (!EnvironmentUtils::Has(BOOTSTRAP_ENV))
	{
		rc = bootloader.Bootstrap();
	}
	else
	{
#ifdef USE_BREAKPAD
		NSString* tempPath = NSTemporaryDirectory();
		if (tempPath == nil)
			tempPath = @"/tmp";
		string dumpPath = [tempPath UTF8String];
		crashHandler.createHandler(dumpPath);
#endif
		[[NSApplication sharedApplication] setDelegate:
			[[KrollApplicationDelegate alloc] init]];
			NSApplicationLoad(); EnvironmentUtils::Unset(BOOTSTRAP_ENV);
		// TODO: use BootStrap() method which is platform independent
		rc = bootloader.StartHost();
	}

	[pool release];
	return rc;
}
