/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008-2009 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _KR_UTILS_H_
#define _KR_UTILS_H_
#include "../base.h"

// If this is a version of the utils which doesn't
// include libkroll, we should use our simple version
// of SharedPtr -- if not use Poco's which is thread-safe.


#include "application.h"
#include "file_utils.h"
#include "data_utils.h"
#include "platform_utils.h"
#include "environment_utils.h"
#include "boot_utils.h"
#include "url_utils.h"

// Platform specific utilities
#ifdef OS_WIN32
#include "win32/win32_utils.h"
#endif

#ifdef OS_OSX
#include "osx/osx_utils.h"
#endif

#ifndef OS_WIN32
#include "posix/posix_utils.h"
#endif

#endif
