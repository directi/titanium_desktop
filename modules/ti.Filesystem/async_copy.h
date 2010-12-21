/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _TI_ASYNC_COPY_H
#define _TI_ASYNC_COPY_H

#include <kroll/kroll.h>
#include <kroll/utils/Thread.h>

#ifdef OS_WIN32
#include <windows.h>
#elif OS_OSX
#import <Foundation/Foundation.h>
#endif

#include <string>
#include <vector>
#include <Poco/Path.h>
#include "filesystem_binding.h"


namespace ti
{
	class AsyncCopy : public StaticBoundObject,
		kroll::Runnable
	{
	public:
		AsyncCopy(const std::vector<std::string> &files,
			const std::string destination,
			KMethodRef callback);
		virtual ~AsyncCopy();
		virtual void run();

		static void Run(void*);

	private:
		const std::vector<std::string> files;
		const std::string destination;
		KMethodRef callback;
		bool stopped;

		void ToString(const ValueList& args, KValueRef result);
		void Cancel(const ValueList& args, KValueRef result);
		void Copy(const std::string& src, const std::string& dest);
	};
}

#endif

