/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008, 2009 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _MAIN_THREAD_JOB_H
#define _MAIN_THREAD_JOB_H

#include <Poco/Semaphore.h>

#include "base.h"
#include "binding/arg_list.h"
#include "binding/value_exception.h"
#include "binding/binding_declaration.h"


namespace kroll
{
	class KROLL_API MainThreadJob
	{
	public:
		MainThreadJob(KMethodRef method, const ValueList& args, bool waitForCompletion);
		void Lock();
		void Wait();
		void Execute();
		KValueRef GetResult();
		ValueException GetException();
		bool ShouldWaitForCompletion();
		void PrintException();

	private:
		KMethodRef method;
		const ValueList args;
		bool waitForCompletion;
		KValueRef returnValue;
		ValueException exception;
		Poco::Semaphore semaphore;
	};
}

#endif
