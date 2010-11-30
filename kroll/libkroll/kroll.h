/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _KROLL_H_
#define _KROLL_H_

#include "base.h"

#ifndef OS_WIN32
	// this is important which essentially marks all of
	// these classes below and the typedef/templates to be
	// visible outside of the library.  if you don't do this
	// you won't be able to catch exceptions of KValueRef for
	// example
	#pragma GCC visibility push(default)
#endif

#ifndef OS_WIN32
	#pragma GCC visibility pop
#endif

#include "net/net.h"
#include "binding/binding.h"
#include "utils/utils.h"
#include "reference_counted.h"
#include "logger.h"

#include "module_provider.h"
#include "module.h"
#include "main_thread_job.h"
#include "script.h"

#ifdef OS_OSX
#include "osx/osx.h"
#elif defined(OS_WIN32)
#include "win32/win32.h"
#endif

#include "host.h"
#include "MainThreadUtils.h"

#endif