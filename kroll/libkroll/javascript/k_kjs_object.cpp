/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "k_kjs_object.h"

namespace kroll
{
	KKJSObject::KKJSObject(JSContextRef context, JSObjectRef jsobject) :
		KObject("JavaScript.KKJSObject"),
		context(NULL),
		jsobject(jsobject)
	{
		this->context = KJSUtil::GetGlobalContext(context);

		KJSUtil::ProtectContextAndValue(this->context, this->jsobject);
	}

	KKJSObject::~KKJSObject()
	{
		KJSUtil::UnprotectContextAndValue(this->context, this->jsobject);
	}

	JSObjectRef KKJSObject::GetJSObject()
	{
		return this->jsobject;
	}

	JSContextRef KKJSObject::GetContext()
	{
		return this->context;
	}

	KValueRef KKJSObject::Get(const char *name)
	{
		JSStringRef jsName = JSStringCreateWithUTF8CString(name);
		JSValueRef exception = NULL;
		JSValueRef jsValue = JSObjectGetProperty(this->context, this->jsobject, jsName, NULL);
		JSStringRelease(jsName);

		if (exception != NULL) //exception thrown
		{
			KValueRef tv_exp = KJSUtil::ToKrollValue(exception, this->context, NULL);
			throw ValueException(tv_exp);
		}

		KValueRef kvalue = KJSUtil::ToKrollValue(jsValue, this->context, this->jsobject);
		return kvalue;
	}

	void KKJSObject::Set(const char *name, KValueRef value)
	{
		JSValueRef jsValue = KJSUtil::ToJSValue(value, this->context);
		JSStringRef jsName = JSStringCreateWithUTF8CString(name);

		JSValueRef exception = NULL;
		JSObjectSetProperty(this->context, this->jsobject, jsName, jsValue,
			NULL, &exception);
		JSStringRelease(jsName);

		if (exception != NULL) // An exception was thrown.
		{
			KValueRef exceptionValue = KJSUtil::ToKrollValue(exception, this->context, NULL);
			throw ValueException(exceptionValue);
		}
	}

	bool KKJSObject::Equals(KObjectRef other)
	{
		AutoPtr<KKJSObject> kjsOther = other.cast<KKJSObject>();
		if (kjsOther.isNull())
			return false;

		if (!kjsOther->SameContextGroup(this->context))
			return false;

		return JSValueIsStrictEqual(
			this->context, this->jsobject, kjsOther->GetJSObject());
	}

	SharedStringList KKJSObject::GetPropertyNames()
	{
		SharedStringList list(new StringList());

		JSPropertyNameArrayRef names =
			JSObjectCopyPropertyNames(this->context, this->jsobject);
		JSPropertyNameArrayRetain(names);

		size_t count = JSPropertyNameArrayGetCount(names);
		for (size_t i = 0; i < count; i++)
		{
			JSStringRef jsName = JSPropertyNameArrayGetNameAtIndex(names, i);
			list->push_back(new std::string(KJSUtil::ToChars(jsName)));
		}

		JSPropertyNameArrayRelease(names);
		return list;
	}

	bool KKJSObject::HasProperty(const char* name)
	{
		JSStringRef jsName = JSStringCreateWithUTF8CString(name);
		bool hasProperty = JSObjectHasProperty(context, jsobject, jsName);
		JSStringRelease(jsName);
		return hasProperty;
	}

	bool KKJSObject::SameContextGroup(JSContextRef contextIn)
	{
		return JSContextGetGroup(this->context) == JSContextGetGroup(contextIn);
	}
}
