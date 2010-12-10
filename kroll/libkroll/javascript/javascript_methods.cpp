/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008-2010 Appcelerator, Inc. All Rights Reserved.
 */

#include "javascript_methods.h"
#include <kroll/host.h>
#include <kroll/utils/Timer.h>
#include <boost/thread/recursive_mutex.hpp>

namespace kroll
{
	// Common javascript functions that are re-implemented
	//  in the global scope so they are available without a window
	namespace JavaScriptMethods
	{
		void Bind(KObjectRef global)
		{
			global->SetMethod("setTimeout", new KFunctionPtrMethod(&SetTimeout));
			global->SetMethod("clearTimeout", new KFunctionPtrMethod(&ClearTimeout));
			global->SetMethod("setInterval", new KFunctionPtrMethod(&SetInterval));
			global->SetMethod("clearInterval", new KFunctionPtrMethod(&ClearInterval));
			global->SetMethod("debuggerPaused", new KFunctionPtrMethod(&DebuggerPaused));
			global->SetMethod("debuggerResumed", new KFunctionPtrMethod(&DebuggerResumed));
		}
		
		//class MainThreadCaller
		//{
		//public:
		//	KMethodRef method;
		//	ValueList args;
		//	
		//	//void OnTimer(Poco::Timer& timer)
		//	//{
		//	//	Host::GetInstance()->RunOnMainThread(method, args);
		//	//}
		//};
		
		//static int currentTimerId = 0;
		static std::map<int, Timer*> timers;
		//static std::map<int, MainThreadCaller*> callers;
		static boost::recursive_mutex timersMutex;
		
		static KValueRef CreateTimer(const ValueList& args, bool recursive)
		{
			KMethodRef method = 0;
			if (args.at(0)->IsMethod())
			{
				method = args.GetMethod(0);
			}
			else if (args.at(0)->IsString())
			{
				// TODO -- create a function from evaluatable code
				throw ValueException::FromString("SetTimeout is not implemented for strings");
			}
			
			long duration = (long)args.at(1)->ToDouble();
			ValueList methodArgs;
			for (size_t i = 2; i < args.size(); i++)
			{
				methodArgs.push_back(args.at(i));
			}
			
			if (!method.isNull())
			{
#ifdef WIN32
				boost::recursive_mutex::scoped_lock lock(timersMutex);
				Timer * timer = new Win32Timer(duration, recursive, method, methodArgs);
				timers[timer->getID()] = timer;
				timer->start();
				return Value::NewInt(timer->getID());
#else
				//boost::recursive_mutex::scoped_lock lock(timersMutex);
				//timers[id] = new Poco::Timer(duration, interval ? duration : 0);
				//callers[id] = new MainThreadCaller();
				//callers[id]->method = method;
				//callers[id]->args = methodArgs;
				//
				//Poco::TimerCallback<MainThreadCaller> callback(*callers[id], &MainThreadCaller::OnTimer);
				//timers[id]->start(callback);
				//
				//currentTimerId++;
				//return Value::NewInt(id);
				throw ValueException::FromString("JavaScriptMethods::CreateTimer Not Implemented");
#endif
			}
			else
			{
				throw ValueException::FromString("Unable to get method for executing on timeout");
			}
		}
		
		// this gets called on the main thread to avoid deadlock during the thread callback
		static KValueRef StopTimer(const ValueList& args)
		{
			bool bRet = false;
#ifdef OS_WIN32
			int id = args.GetInt(0);
			boost::recursive_mutex::scoped_lock lock(timersMutex);
			std::map<int, Timer*>::iterator timerIter = timers.find(id);
			if (timerIter != timers.end())
			{
				Timer * timer = timerIter->second;
				bRet = timer->stop();
				timers.erase(timerIter);
				delete timer;
			}
#else
			//boost::recursive_mutex::scoped_lock lock(timersMutex);
			//std::map<int, Poco::Timer*>::iterator timerIter = timers.find(id);
			//std::map<int, MainThreadCaller*>::iterator callerIter = callers.find(id);
			//if (timerIter != timers.end() && callerIter != callers.end())
			//{
			//	MainThreadCaller* caller = callerIter->second;
			//	Poco::Timer* timer = timerIter->second;
			//	
			//	// same as stop() but safe to be called from within the Timer callback
			//	timer->restart(0);
			//	callers.erase(callerIter);
			//	timers.erase(timerIter);
			//	delete caller;
			//	delete timer;
			//	
			//	bRet = true;
			//}
#endif
			return Value::NewBool(bRet);
		}
		
		KValueRef SetTimeout(const ValueList& args)
		{
			args.VerifyException("setTimeout", "m|s i");
			return CreateTimer(args, false);
		}
		
		KValueRef SetInterval(const ValueList& args)
		{
			args.VerifyException("setInterval", "m|s i");
			return CreateTimer(args, true);
		}
		
		KValueRef ClearTimeout(const ValueList& args)
		{
			args.VerifyException("clearTimeout", "i");
			return Host::GetInstance()->RunOnMainThread(new KFunctionPtrMethod(&StopTimer), args);
		}
		
		KValueRef ClearInterval(const ValueList& args)
		{
			args.VerifyException("clearInterval", "i");
			return Host::GetInstance()->RunOnMainThread(new KFunctionPtrMethod(&StopTimer), args);
		}

		KValueRef DebuggerPaused(const ValueList& args)
		{
			Host::GetInstance()->SuspendMainThreadJobs();
			return Value::Undefined;
		}

		KValueRef DebuggerResumed(const ValueList& args)
		{
			Host::GetInstance()->ResumeMainThreadJobs();
			return Value::Undefined;
		}
	}
}
