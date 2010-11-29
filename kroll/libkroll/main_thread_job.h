/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008, 2009 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _MAIN_THREAD_JOB_H
#define _MAIN_THREAD_JOB_H

#include <Poco/Semaphore.h>

#include "base.h"
#include "binding/bytes.h"
#include "binding/arg_list.h"
#include "binding/value_exception.h"
#include "binding/binding_declaration.h"


namespace kroll
{
	class KROLL_API MainThreadJob
	{
	public:
		MainThreadJob(KMethodRef method);
		MainThreadJob(KMethodRef method, const ValueList& args);
		virtual ~MainThreadJob() {}
		virtual void Execute();

		void Lock();
		void Wait();
		KValueRef GetResult();
		ValueException GetException();
		bool ShouldWaitForCompletion();
		void PrintException();

	private:
		KMethodRef method;
		KValueRef returnValue;
		ValueException exception;
		Poco::Semaphore semaphore;

	protected:
		ValueList args;
	};

	class KROLL_API MainThreadReadJob
		: public MainThreadJob
	{
	private:
		const char * data;
		size_t size;
	public:
		MainThreadReadJob(KMethodRef method, const char * _data, size_t _size);
		virtual ~MainThreadReadJob() {}
		virtual void Execute();
	};
}

#endif
