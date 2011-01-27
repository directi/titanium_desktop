/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license. 
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#include "binding/kmethod.h"

#include "logger.h"
#include "main_thread_job.h"


namespace kroll
{

	MainThreadJob::MainThreadJob(KMethodRef method, const ValueList& args) :
		method(method),
		returnValue(NULL),
		exception(NULL),
		executed(false),
		waiting(false),
		args(args)
	{
		
	}

	MainThreadJob::MainThreadJob(KMethodRef method) :
		method(method),
		returnValue(NULL),
		exception(NULL),
		executed(false),
		waiting(false),
		args(ValueList())
	{
		
	}

	void MainThreadJob::Execute()
	{
		static bool isExec = false;
		if(isExec)
			fprintf(stderr, "Screwed, we just execed one MainThreadJob on top of another!\n");
		isExec = true;
		try
		{
			this->returnValue = this->method->Call(this->args);
		}
		catch (ValueException& e)
		{
			this->exception = e;
		}
		catch (std::exception& e)
		{
			this->exception = ValueException::FromString(e.what());
		}
		catch (...)
		{
			this->exception = ValueException::FromString("Unknown Exception from job queue");
		}
	
		{
			boost::lock_guard<boost::mutex> lock(completionLock);
			executed = true;
			if(waiting)
				completion.notify_one();
		}
		isExec = false;
	}

	void MainThreadJob::Wait()
	{
		boost::unique_lock<boost::mutex> lock(completionLock);
		waiting = true;
		while(! executed)
		{
			completion.wait(lock);
		}
		waiting = false;
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

