/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "../zip_utils.h"
#include <kroll/utils/file_utils.h>

#ifdef OS_OSX
#include <Cocoa/Cocoa.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/network/IOEthernetInterface.h>
#include <IOKit/network/IONetworkInterface.h>
#include <IOKit/network/IOEthernetController.h>
#include <sys/utsname.h>
#include <libgen.h>
#elif defined(OS_LINUX)
#include <cstdarg>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/utsname.h>
#include <libgen.h>
#endif

#include <iostream>
#include <sstream>
#include <cstring>

namespace UTILS_NS
{
	bool FileUtils::Unzip(const std::string& source, const std::string& destination, 
		UnzipCallback callback, void* data)
	{
#ifdef OS_OSX
		//
		// we don't include gzip since we're on OSX
		// we just let the built-in OS handle extraction for us
		//
		std::vector<std::string> args;
		args.push_back("--noqtn");
		args.push_back("-x");
		args.push_back("-k");
		args.push_back("--rsrc");
		args.push_back(source);
		args.push_back(destination);
		std::string cmdline = "/usr/bin/ditto";
		RunAndWait(cmdline, args);
		return true;
#elif OS_LINUX
		std::vector<std::string> args;
		args.push_back("-qq");
		args.push_back("-o");
		args.push_back(source);
		args.push_back("-d");
		args.push_back(destination);
		std::string cmdline("/usr/bin/unzip");
		RunAndWait(cmdline, args);
		return true;
#endif
	}
}

