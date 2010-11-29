/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license. 
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#include <Poco/Semaphore.h>

#include "binding/kmethod.h"

#include "logger.h"
#include "main_thread_job.h"


namespace kroll
{

	MainThreadJob::MainThreadJob(KMethodRef method, const ValueList& args) :
		method(method),
		args(args),
		returnValue(NULL),
		exception(ValueException(NULL)),
		semaphore(0, 1)
	{
		// The semaphore starts at 0, meaning that the calling
		// thread can wait for the value to become >0 using wait()
		// and the main thread can call set() after job execution
		// which meets this condition.
	}

	MainThreadJob::MainThreadJob(KMethodRef method) :
		method(method),
		args(ValueList()),
		returnValue(NULL),
		exception(ValueException(NULL)),
		semaphore(0, 1)
	{
		// The semaphore starts at 0, meaning that the calling
		// thread can wait for the value to become >0 using wait()
		// and the main thread can call set() after job execution
		// which meets this condition.
	}

	void MainThreadJob::Wait()
	{
		this->semaphore.wait();
	}

	void MainThreadJob::Execute()
	{
		try
		{
			this->returnValue = this->method->Call(this->args);
		}
		catch (ValueException& e)
		{
			this->exception = e;
		}
		catch (Poco::SystemException& e)
		{
			this->exception = ValueException::FromString(e.displayText());
		}
		catch (std::exception& e)
		{
			this->exception = ValueException::FromString(e.what());
		}
		catch (...)
		{
			this->exception = ValueException::FromString("Unknown Exception from job queue");
		}

		this->semaphore.set();
	}

	KValueRef MainThreadJob::GetResult()
	{
		return this->returnValue;
	}

	ValueException MainThreadJob::GetException()
	{
		return this->exception;
	}

	void MainThreadJob::PrintException()
	{
		static Logger* logger = Logger::Get("Host");
		if (this->returnValue.isNull())
		{
			logger->Error("Error in the job queue: %s",
				this->exception.ToString().c_str());
		}
	}

	MainThreadReadJob::MainThreadReadJob(KMethodRef method, const char * _data, size_t _size)
		: MainThreadJob(method),
		data(_data),
		size(_size)
	{
	}
	void MainThreadReadJob::Execute()
	{
		BytesRef bytes(new Bytes(this->data, this->size));
		args.push_back(Value::NewObject(bytes));
		MainThreadJob::Execute();
	}

}

