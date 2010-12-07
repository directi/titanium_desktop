/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008, 2009 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _MAIN_THREAD_JOB_H
#define _MAIN_THREAD_JOB_H

#include "base.h"
#include "binding/bytes.h"
#include "binding/arg_list.h"
#include "binding/value_exception.h"
#include "binding/binding_declaration.h"

#include <boost/thread/condition_variable.hpp>
#include <boost/thread/mutex.hpp>

namespace kroll
{
	class KROLL_API MainThreadJob
	{
	public:
		MainThreadJob(KMethodRef method);
		MainThreadJob(KMethodRef method, const ValueList& args);
		virtual ~MainThreadJob() {}
		virtual void Execute();

		KValueRef GetResult();
		ValueException GetException();
		void PrintException();

		void Wait();

	private:
		KMethodRef method;
		KValueRef returnValue;
		ValueException exception;
		bool executed;
		bool waiting;
		boost::condition_variable completion;
		boost::mutex completionLock;


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
