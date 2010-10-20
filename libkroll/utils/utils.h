/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008-2009 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _KR_UTILS_H_
#define _KR_UTILS_H_

#include <base.h>

#include <kroll/utils/url_utils.h>
#include <kroll/utils/file_utils.h>
#include <kroll/utils/data_utils.h>
#include <kroll/utils/boot_utils.h>
#include <kroll/utils/application.h>
#include <kroll/utils/platform_utils.h>
#include <kroll/utils/environment_utils.h>

// Platform specific utilities
#ifdef OS_WIN32
#include <kroll/utils/win32/win32_utils.h>
#else
#include <kroll/utils/posix/posix_utils.h>
#endif

#ifdef OS_OSX
#include <kroll/utils/osx/osx_utils.h>
#endif

#endif
