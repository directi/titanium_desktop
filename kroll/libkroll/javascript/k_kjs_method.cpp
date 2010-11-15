/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "javascript_module.h"

namespace kroll
{
	KKJSMethod::KKJSMethod(JSContextRef context, JSObjectRef jsobject, JSObjectRef thisObject) :
		KMethod("JavaScript.KKJSMethod"),
		KKJSObject(context, jsobject),
		context(context),
		jsobject(jsobject),
		thisObject(thisObject)
	{

		/* KJS methods run in the global context that they originated from
		 * this seems to prevent nasty crashes from trying to access invalid
		 * contexts later. Global contexts need to be registered by all modules
		 * that use a KJS context. */
		JSObjectRef globalObject = JSContextGetGlobalObject(context);
		JSContextRef globalContext = KJSUtil::GetGlobalContext(globalObject, context);

		// This context hasn't been registered. Something has gone pretty
		// terribly wrong and Kroll will likely crash soon. Nonetheless, keep
		// the user up-to-date to keep their hopes up.
		if (globalContext == NULL)
			std::cerr << "Could not locate global context for a KJS method."  <<
				" One of the modules is misbehaving." << std::endl;

		this->context = globalContext;

#if BARK_UP_WEAKREF_TREE
		KKJSMethod::RegisterMethod(this);
#else
		KJSUtil::ProtectContextAndValue(this->context, this->jsobject);
		if (thisObject != NULL) {
			KJSUtil::ProtectContextAndValue(this->context, thisObject);
		}
#endif
	}

	KKJSMethod::~KKJSMethod()
	{
#if BARK_UP_WEAKREF_TREE
		KKJSMethod::UnregisterMethod(this);
#else
		if (this->thisObject != NULL) 
		{
			KJSUtil::UnprotectContextAndValue(this->context, this->thisObject);
		}
		KJSUtil::UnprotectContextAndValue(this->context, this->jsobject);
#endif
	}

	void KKJSMethod::release()
	{
		KObject::release();
	}

	void KKJSMethod::duplicate()
	{
		KObject::duplicate();
	}

	KValueRef KKJSMethod::Get(const char *name)
	{
		return KKJSObject::Get(name);
	}

	void KKJSMethod::Set(const char *name, KValueRef value)
	{
		return KKJSObject::Set(name, value);
	}

	bool KKJSMethod::Equals(KObjectRef other)
	{
		return this->KKJSObject::Equals(other);
	}

	SharedStringList KKJSMethod::GetPropertyNames()
	{
		return KKJSObject::GetPropertyNames();
	}

	bool KKJSMethod::HasProperty(const char* name)
	{
		return KKJSObject::HasProperty(name);
	}

	bool KKJSMethod::SameContextGroup(JSContextRef c)
	{
		return KKJSObject::SameContextGroup(c);
	}

	JSObjectRef KKJSMethod::GetJSObject()
	{
		return this->jsobject;
	}

	KValueRef KKJSMethod::Call(const ValueList& args)
	{
		JSValueRef* jsArgs = new JSValueRef[args.size()];
		for (int i = 0; i < (int) args.size(); i++)
		{
			KValueRef arg = args.at(i);
			jsArgs[i] = KJSUtil::ToJSValue(arg, this->context);
		}

		JSValueRef exception = NULL;
		JSValueRef jsValue = JSObjectCallAsFunction(this->context, jsobject,
			this->thisObject, args.size(), jsArgs, &exception);

		delete [] jsArgs; // clean up args

		if (jsValue == NULL && exception != NULL) //exception thrown
		{
			KValueRef exceptionValue = KJSUtil::ToKrollValue(exception, this->context, NULL);
			throw ValueException(exceptionValue);
		}
		return KJSUtil::ToKrollValue(jsValue, this->context, NULL);
	}

#if BARK_UP_WEAKREF_TREE
	ContextRefs KKJSMethod::contextRefs;
	Poco::Mutex KKJSMethod::contextRefsMutex;

	void KKJSMethod::MapDestroyed(JSWeakObjectMapRef map, void *data)
	{
		Poco::Mutex::ScopedLock t(contextRefsMutex);
		contextRefs.erase((JSContextRef) data);
	}

	void KKJSMethod::RegisterMethod(KKJSMethod* method)
	{
		Poco::Mutex::ScopedLock t(contextRefsMutex);
		ContextRefs::iterator pos = contextRefs.find(method->context);
		if(pos == contextRefs.end())
		{
			contextRefs[method->context] = JSWeakObjectMapCreate(method->context, &(method->context), &KKJSMethod::MapDestroyed);
			pos = contextRefs.find(method->context);
		}
		JSValueRef kvalue = KJSUtil::KMethodToJSValue(Value::NewMethod(method), method->context);
		method->jsRef = JSValueToObject(method->context, kvalue, NULL);
		if(method->jsRef != 0) {
			JSWeakObjectMapSet(method->context, pos->second, &method, method->jsRef);
			// Prevent AutoPtr's from cleaning us up - we do get cast as an AutoPtr somewhere!!...
			method->duplicate();
		}
		else
			fprintf(stderr, "Error casting KKJSMethod to a JSObjectRef\n");
	}

	void KKJSMethod::UnregisterMethod(KKJSMethod* method)
	{
		Poco::Mutex::ScopedLock t(contextRefsMutex);
		ContextRefs::iterator pos = contextRefs.find(method->context);
		if(pos != contextRefs.end())
		{
			if(method->jsRef != 0)
				JSWeakObjectMapClear(method->context, pos->second, &method, method->jsRef);
			else
				fprintf(stderr, "Error casting KKJSMethod to a JSObjectRef\n");
		}
#if DEBUG
		else
			fprintf(stderr, "Can't find a weak object map for this context\n");
#endif
	}
#endif
}

