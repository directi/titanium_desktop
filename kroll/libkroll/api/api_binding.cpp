/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "api_binding.h"
#include <algorithm>

#include <kroll/thread_manager.h>

#include <kroll/binding/bytes.h>
#include <kroll/binding/static_bound_list.h>

#include <kroll/utils/file_utils.h>

using std::string;
using std::vector;
using std::pair;
using std::map;

namespace kroll
{
	APIBinding::APIBinding(Host* host) :
		KAccessorObject("API"),
		host(host),
		global(host->GetGlobalObject()),
		logger(Logger::Get("API"))
	{
		/**
		 * @tiapi(method=True,name=API.set,since=0.2)
		 * @tiapi Set an attribute in the global object
		 * @tiarg[String, key] Key of the attribute to set
		 * @tiarg[String, value] New value of the attribute
		 */
		this->SetMethod("set", &APIBinding::_Set);

		/**
		 * @tiapi(method=True,name=API.get,since=0.2)
		 * @tiapi Get an attribute in the global object
		 * @tiarg[String, key] Key of the attribute to get
		 * @tiresult[any] Attribute at that key
		 */
		this->SetMethod("get", &APIBinding::_Get);

		/**
		 * @tiapi(method=True,name=API.addEventListener,since=0.5)
		 * @tiapi Register a root event listener
		 * @tiarg[String, eventName] The event name to listen for
		 * @tiarg[Function, callback] The callback to invoke when this message occurs
		 * @tiresult[Number] An id which represents this event listener
		 */
		this->SetMethod("addEventListener", &APIBinding::_AddEventListener);

		/**
		 * @tiapi(method=True,name=API.removeEventListener,since=0.5)
		 * @tiapi Remove a root event listener
		 * @tiarg[Number|Function, id] The id or callback of the event listener to remove
		 */
		this->SetMethod("removeEventListener", &APIBinding::_RemoveEventListener);

		/**
		 * @tiapi(method=True,name=API.fireEvent,since=0.5)
		 * @tiapi Fire the event with a given name
		 * @tiarg[String|Object, event] The name of the event to fire or the event itself
		 */
		this->SetMethod("fireEvent", &APIBinding::_FireEvent);

		/**
		 * @tiapi(method=True,name=API.createKObject,since=0.5) Create a Kroll object.
		 * @tiarg[Object, toWrap, optional=true] An object to wrap in a new KObject.
		 * @tiresult[Object] A new KObject.
		 */
		this->SetMethod("createKObject", &APIBinding::_CreateKObject);

		/**
		 * @tiapi(method=True,name=API.createKMethod,since=0.5) create a Kroll method.
		 * @tiarg[Function, toWrap, optional=true] A function to wrap in a new KMethod
		 * @tiresult[Function] A new KMethod.
		 */
		this->SetMethod("createKMethod", &APIBinding::_CreateKMethod);
		
		/**
		 * @tiapi(method=True,name=API.createKList,since=0.5) create a Kroll list.
		 * @tiarg[Array, toWrap, optional=true] A function to wrap in a new KMethod
		 * @tiresult[Array] A new KList.
		 */
		this->SetMethod("createKList", &APIBinding::_CreateKList);

		/**
		 * @tiapi(method=True,name=API.createBytes,since=0.9) Create a Kroll Bytes object.
		 * @tiarg[String, contents, optional=true] The contents of the new Bytes.
		 * @tiarg The blob will be empty if none are given.
		 * @tiresult[Bytes] A new Bytes.
		 */
		this->SetMethod("createBytes", &APIBinding::_CreateBytes);
		this->SetMethod("createBlob", &APIBinding::_CreateBytes);

		/**
		 * @tiapi(method=True,name=API.log,since=0.2)
		 * @tiapi Log a statement with a given severity
		 * @tiarg[Number, type] the severity of this log statement
		 * @tiarg[String, statement] the statement to log
		 */
		this->SetMethod("log", &APIBinding::_Log);

		/**
		 * @tiapi(method=True,name=API.trace,since=0.4)
		 * @tiapi Log a statement with TRACE severity
		 * @tiarg[String, statement] the statement to log
		 */
		this->SetMethod("trace", &APIBinding::_LogTrace);

		/**
		 * @tiapi(method=True,name=API.debug,since=0.4)
		 * @tiapi Log a statement with DEBUG severity
		 * @tiarg[String, statement] the statement to log
		 */
		this->SetMethod("debug", &APIBinding::_LogDebug);

		/**
		 * @tiapi(method=True,name=API.info,since=0.4)
		 * @tiapi Log a statement with INFO severity
		 * @tiarg[String, statement] the statement to log
		 */
		this->SetMethod("info", &APIBinding::_LogInfo);

		/**
		 * @tiapi(method=True,name=API.notice,since=0.4)
		 * @tiapi Log a statement with NOTICE severity
		 * @tiarg[String, statement] the statement to log
		 */
		this->SetMethod("notice", &APIBinding::_LogNotice);

		/**
		 * @tiapi(method=True,name=API.warn,since=0.4)
		 * @tiapi Log a statement with WARN severity
		 * @tiarg[String, statement] the statement to log
		 */
		this->SetMethod("warn", &APIBinding::_LogWarn);

		/**
		 * @tiapi(method=True,name=API.error,since=0.4)
		 * @tiapi Log a statement with ERROR severity
		 * @tiarg[String, statement] the statement to log
		 */
		this->SetMethod("error", &APIBinding::_LogError);

		/**
		 * @tiapi(method=True,name=API.critical,since=0.4)
		 * @tiapi Log a statement with CRITICAL severity
		 * @tiarg[String, statement] the statement to log
		 */
		this->SetMethod("critical", &APIBinding::_LogCritical);

		/**
		 * @tiapi(method=True,name=API.fatal,since=0.4)
		 * @tiapi Log a statement with FATAL severity
		 * @tiarg[String, statement] the statement to log
		 */
		this->SetMethod("fatal", &APIBinding::_LogFatal);

		/**
		 * @tiapi(method=True,name=API.setLogLevel,since=0.5)
		 * @tiapi Set the log level of the root logger
		 * @tiarg[Number, level] the threshold of severity to log
		 */
		this->SetMethod("setLogLevel", &APIBinding::_SetLogLevel);

		/**
		 * @tiapi(method=True,name=API.getLogLevel,since=0.5)
		 * @tiapi Get the log level of the root logger
		 * @tiresult[Number] the threshold of severity to log
		 */
		this->SetMethod("getLogLevel", &APIBinding::_GetLogLevel);
	
		/**
		 * @tiapi(method=True,name=API.print,since=0.6)
		 * @tiapi print a raw string to stdout (no newlines are appended)
		 * @tiarg[Any, data] data to print
		 */
		this->SetMethod("print", &APIBinding::_Print);
	
		// These are properties for log severity levels

		/**
		 * @tiapi(property=True,name=API.TRACE,since=0.4)
		 * @tiapi a constant representing TRACE severity
		 */
		this->Set("TRACE", Value::NewInt(Logger::LTRACE));

		/**
		 * @tiapi(property=True,name=API.DEBUG,since=0.4)
		 * @tiapi a constant representing DEBUG severity
		 */
		this->Set("DEBUG", Value::NewInt(Logger::LDEBUG));

		/**
		 * @tiapi(property=True,name=API.INFO,since=0.4)
		 * @tiapi a constant representing INFO severity
		 */
		this->Set("INFO", Value::NewInt(Logger::LINFO));

		/**
		 * @tiapi(property=True,name=API.NOTICE,since=0.4)
		 * @tiapi a constant representing NOTICE severity
		 */
		this->Set("NOTICE", Value::NewInt(Logger::LNOTICE));

		/**
		 * @tiapi(property=True,name=API.WARN,since=0.4)
		 * @tiapi a constant representing WARN severity
		 */
		this->Set("WARN", Value::NewInt(Logger::LWARN));

		/**
		 * @tiapi(property=True,name=API.ERROR,since=0.4)
		 * @tiapi a constant representing ERROR severity
		 */
		this->Set("ERROR", Value::NewInt(Logger::LERROR));

		/**
		 * @tiapi(property=True,name=API.CRITICAL,since=0.4)
		 * @tiapi a constant representing CRITICAL severity
		 */
		this->Set("CRITICAL", Value::NewInt(Logger::LCRITICAL));

		/**
		 * @tiapi(property=True,name=API.FATAL,since=0.4)
		 * @tiapi a constant representing FATAL severity
		 */
		this->Set("FATAL", Value::NewInt(Logger::LFATAL));

		// These are properties for dependencies

		/**
		 * @tiapi(property=True,name=API.EQ,since=0.4)
		 * @tiapi a constant representing an equality dependency
		 */
		this->Set("EQ", Value::NewInt(Dependency::EQ));

		/**
		 * @tiapi(property=True,name=API.LT,since=0.4)
		 * @tiapi a constant representing a less-than dependency
		 */
		this->Set("LT", Value::NewInt(Dependency::LT));

		/**
		 * @tiapi(property=True,name=API.LTE,since=0.4)
		 * @tiapi a constant representing a less-than-or-equal dependency
		 */
		this->Set("LTE", Value::NewInt(Dependency::LTE));

		/**
		 * @tiapi(property=True,name=API.GT,since=0.4)
		 * @tiapi a constant representing a greater-than dependency
		 */
		this->Set("GT", Value::NewInt(Dependency::GT));

		/**
		 * @tiapi(property=True,name=API.GTE,since=0.4)
		 * @tiapi a constant representing a greather-than-or-equal dependency
		 */
		this->Set("GTE", Value::NewInt(Dependency::GTE));

		// Component types

		/**
		 * @tiapi(property=True,name=API.MODULE,since=0.4)
		 * @tiapi a constant representing a module component type
		 */
		this->Set("MODULE", Value::NewInt(MODULE));

		/**
		 * @tiapi(property=True,name=API.RUNTIME,since=0.4)
		 * @tiapi a constant representing a runtime component type
		 */
		this->Set("RUNTIME", Value::NewInt(RUNTIME));

		/**
		 * @tiapi(property=True,name=API.UNKNOWN,since=0.4)
		 * @tiapi a constant representing an UNKNOWN component type
		 */
		this->Set("UNKNOWN", Value::NewInt(UNKNOWN));
	}

	APIBinding::~APIBinding()
	{
	}

	void APIBinding::_Set(const ValueList& args, KValueRef result)
	{
		string key = args.at(0)->ToString();
		KValueRef value = args.at(1);
		string::size_type pos = key.find_first_of(".");

		if (pos==string::npos)
		{
			this->Set(key.c_str(), value);
		}
		else
		{
			// if we have a period, make it relative to the
			// global scope such that <module>.<key> would resolve
			// to the 'module' with key named 'key'
			global->SetNS(key.c_str(), value);
		}
	}

	Logger::Level APIBinding::ValueToLevel(KValueRef v)
	{
		if (v->IsString())
		{
			string levelString = v->ToString();
			return Logger::GetLevel(levelString);
		}
		else if (v->IsObject())
		{
			SharedString ss = v->ToObject()->DisplayString();
			return Logger::GetLevel(*ss);
		}
		else if (v->IsNumber())
		{
			return (Logger::Level) v->ToInt();
		}
		else // return the appropriate default
		{
			string levelString = "";
			return Logger::GetLevel(levelString);
		}
	}

	void APIBinding::_Get(const ValueList& args, KValueRef result)
	{
		string s = args.at(0)->ToString();
		const char *key = s.c_str();
		KValueRef r = 0;
		string::size_type pos = s.find_first_of(".");

		if (pos==string::npos)
		{
			r = this->Get(key);
		}
		else
		{
			// if we have a period, make it relative to the
			// global scope such that <module>.<key> would resolve
			// to the 'module' with key named 'key'
			r = global->GetNS(key);
		}
		result->SetValue(r);
	}

	void APIBinding::_SetLogLevel(const ValueList& args, KValueRef result)
	{
		args.VerifyException("setLogLevel", "s|n");
		RootLogger::Instance()->setLevel(ValueToLevel(args.at(0)));
	}

	void APIBinding::_GetLogLevel(const ValueList& args, KValueRef result)
	{
		result->SetInt(RootLogger::Instance()->getLevel());
	}

	void APIBinding::_Print(const ValueList& args, KValueRef result)
	{
		for (size_t c=0; c < args.size(); c++)
		{
			KValueRef arg = args.at(c);
			if (arg->IsString())
			{
				std::cout << arg->ToString();
			}
			else
			{
				std::cout << arg->DisplayString();
			}
		}
		std::cout.flush();
	}
	
	void APIBinding::_LogTrace(const ValueList& args, KValueRef result)
	{
		KValueRef arg1 = args.at(0);
		this->Log(Logger::LTRACE, arg1);
	}
	void APIBinding::_LogDebug(const ValueList& args, KValueRef result)
	{
		KValueRef arg1 = args.at(0);
		this->Log(Logger::LDEBUG, arg1);
	}
	void APIBinding::_LogInfo(const ValueList& args, KValueRef result)
	{
		KValueRef arg1 = args.at(0);
		this->Log(Logger::LINFO, arg1);
	}
	void APIBinding::_LogNotice(const ValueList& args, KValueRef result)
	{
		KValueRef arg1 = args.at(0);
		this->Log(Logger::LNOTICE, arg1);
	}
	void APIBinding::_LogWarn(const ValueList& args, KValueRef result)
	{
		KValueRef arg1 = args.at(0);
		this->Log(Logger::LWARN, arg1);
	}
	void APIBinding::_LogError(const ValueList& args, KValueRef result)
	{
		KValueRef arg1 = args.at(0);
		this->Log(Logger::LERROR, arg1);
	}
	void APIBinding::_LogCritical(const ValueList& args, KValueRef result)
	{
		KValueRef arg1 = args.at(0);
		this->Log(Logger::LCRITICAL, arg1);
	}
	void APIBinding::_LogFatal(const ValueList& args, KValueRef result)
	{
		KValueRef arg1 = args.at(0);
		this->Log(Logger::LFATAL, arg1);
	}

	void APIBinding::_Log(const ValueList& args, KValueRef result)
	{
		if (args.size() == 1)
		{
			KValueRef v = args.at(0);
			this->Log(Logger::LINFO, v);
		}
		else if (args.size() == 2)
		{
			KValueRef arg1 = args.at(0);
			
			KValueRef v = args.at(1);
			this->Log(ValueToLevel(arg1), v);
		}
	}

	void APIBinding::_AddEventListener(const ValueList& args, KValueRef result)
	{
		GlobalObject::GetInstance()->_AddEventListener(args, result);
	}

	void APIBinding::_RemoveEventListener(const ValueList& args, KValueRef result)
	{
		GlobalObject::GetInstance()->_RemoveEventListener(args, result);
	}

	void APIBinding::_FireEvent(const ValueList& args, KValueRef result)
	{
		args.VerifyException("fireEvent", "s|o");
		if (args.at(0)->IsString())
		{
			std::string eventName = args.GetString(0);
			GlobalObject::GetInstance()->FireEvent(eventName);
		}
		else if (args.at(0)->IsObject())
		{
			AutoPtr<Event> event = args.GetObject(0).cast<Event>();
			if (!event.isNull())
				GlobalObject::GetInstance()->FireEvent(event);
		}
	}

	//---------------- IMPLEMENTATION METHODS
	void APIBinding::Log(int severity, KValueRef value)
	{
		if (value->IsString())
		{
			string message = value->ToString();
			logger->Log((Logger::Level) severity, message);
		}
		else
		{
			const std::string message = value->DisplayString();
			logger->Log((Logger::Level) severity, message);
		}
	}

	KListRef APIBinding::ManifestToKList(const map<string, string>& manifest)
	{
		KListRef list = new StaticBoundList();

		for(map<string, string>::const_iterator
			oIter = manifest.begin();
			oIter != manifest.end();
		oIter++)
		{
			KListRef entry = new StaticBoundList();
			entry->Append(Value::NewString(oIter->first));
			entry->Append(Value::NewString(oIter->second));
			list->Append(Value::NewList(entry));
		}
		return list;
	}

	void APIBinding::_CreateKObject(const ValueList& args, KValueRef result)
	{
		args.VerifyException("createKObject", "?o");
		if (args.size() <= 0)
		{
			result->SetObject(new StaticBoundObject());
		}
		else
		{
			KObjectRef wrapped = args.GetObject(0);
			result->SetObject(new KObjectWrapper(wrapped));
		}
	}

	void APIBinding::_CreateKMethod(const ValueList& args, KValueRef result)
	{
		args.VerifyException("createKMethod", "m");
		KMethodRef wrapped = args.GetMethod(0);
		result->SetMethod(new KMethodWrapper(args.GetMethod(0)));
	}

	void APIBinding::_CreateKList(const ValueList& args, KValueRef result)
	{
		args.VerifyException("createKList", "?l");
		if (args.size() <= 0)
		{
			result->SetList(new StaticBoundList());
		}
		else
		{
			KListRef wrapped = args.GetList(0);
			result->SetList(new KListWrapper(wrapped));
		}
	}

	static void GetBytes(KValueRef value, std::vector<BytesRef>& blobs)
	{
		if (value->IsObject())
		{
			blobs.push_back(value->ToObject().cast<Bytes>());
		}
		else if (value->IsString())
		{
			blobs.push_back(new Bytes(value->ToString()));
		}
		else if (value->IsList())
		{
			KListRef list = value->ToList();
			for (size_t j = 0; j < list->Size(); j++)
			{
				GetBytes(list->At(j), blobs);
			}
		}
		else if (value->IsNumber())
		{
			blobs.push_back(new Bytes(value->ToInt()));
		}
	}
	
	void APIBinding::_CreateBytes(const ValueList& args, KValueRef result)
	{
		args.VerifyException("createBytes", "?s|o|l|i");
		std::vector<BytesRef> blobs;
		for (size_t i = 0; i < args.size(); i++)
		{
			GetBytes(args.at(i), blobs);
		}
		result->SetObject(Bytes::GlobBytes(blobs));
	}

	KObjectWrapper::KObjectWrapper(KObjectRef object) :
		object(object)
	{
	}

	void KObjectWrapper::Set(const char *name, KValueRef value)
	{
		object->Set(name, value);
	}

	KValueRef KObjectWrapper::Get(const char *name)
	{
		return object->Get(name);
	}

	bool KObjectWrapper::HasProperty(const char *name)
	{
		return object->HasProperty(name);	
	}
	
	SharedStringList KObjectWrapper::GetPropertyNames()
	{
		return object->GetPropertyNames();
	}

	SharedString KObjectWrapper::DisplayString(int levels)
	{
		return object->DisplayString(levels);
	}

	bool KObjectWrapper::Equals(KObjectRef other)
	{
		return object->Equals(other);	
	}
	
	KMethodWrapper::KMethodWrapper(KMethodRef method) :
		method(method)
	{
	}

	KValueRef KMethodWrapper::Call(const ValueList& args)
	{
		return method->Call(args);
	}

	void KMethodWrapper::Set(const char *name, KValueRef value)
	{
		method->Set(name, value);
	}

	KValueRef KMethodWrapper::Get(const char *name)
	{
		return method->Get(name);
	}

	bool KMethodWrapper::HasProperty(const char *name)
	{
		return method->HasProperty(name);
	}
	
	SharedStringList KMethodWrapper::GetPropertyNames()
	{
		return method->GetPropertyNames();
	}

	SharedString KMethodWrapper::DisplayString(int levels)
	{
		return method->DisplayString(levels);
	}
	
	bool KMethodWrapper::Equals(KObjectRef other)
	{
		return method->Equals(other);	
	}

	KListWrapper::KListWrapper(KListRef list) :
		list(list)
	{
	}

	void KListWrapper::Append(KValueRef value)
	{
		list->Append(value);
	}

	unsigned int KListWrapper::Size()
	{
		return list->Size();
	}

	KValueRef KListWrapper::At(unsigned int index)
	{
		return list->At(index);
	}

	void KListWrapper::SetAt(unsigned int index, KValueRef value)
	{
		list->SetAt(index, value);
	}

	bool KListWrapper::Remove(unsigned int index)
	{
		return list->Remove(index);
	}

	void KListWrapper::Set(const char *name, KValueRef value)
	{
		list->Set(name, value);
	}

	KValueRef KListWrapper::Get(const char *name)
	{
		return list->Get(name);
	}

	bool KListWrapper::HasProperty(const char *name)
	{
		return list->HasProperty(name);
	}
	
	SharedStringList KListWrapper::GetPropertyNames()
	{
		return list->GetPropertyNames();
	}

	SharedString KListWrapper::DisplayString(int levels)
	{
		return list->DisplayString(levels);
	}
	
	bool KListWrapper::Equals(KObjectRef other)
	{
		return list->Equals(other);	
	}

}
