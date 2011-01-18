/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license. 
 * Copyright (c) 2008, 2009 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _KR_MAIN_THREAD_UTILS_H_
#define _KR_MAIN_THREAD_UTILS_H_

namespace kroll
{
	KROLL_API bool IsMainThread();
}

#define CRASH() do { \
    *(int *)(uintptr_t)0xbbadbeef = 0; \
    ((void(*)())0)(); /* More reliable, but doesn't say BBADBEEF */ \
} while(false);

#define ASSERT(str, a) if(!a) { fprintf(stderr, str); CRASH(); }
#define ASSERT_MAIN_THREAD ASSERT("IsMainThread", IsMainThread())

#endif
