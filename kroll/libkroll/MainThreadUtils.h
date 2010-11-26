/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license. 
 * Copyright (c) 2008, 2009 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _KR_MAIN_THREAD_UTILS_H_
#define _KR_MAIN_THREAD_UTILS_H_

#include <base.h>
#include <assert.h>

namespace kroll
{
	KROLL_API bool IsMainThread();
}

#define ASSERT_MAIN_THREAD assert(IsMainThread());

#endif
