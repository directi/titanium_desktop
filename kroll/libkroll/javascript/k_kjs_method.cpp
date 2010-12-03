/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "javascript_module.h"

namespace kroll
{
	KKJSMethod::KKJSMethod(JSGlobalContextRef context, JSObjectRef jsobject, JSObjectRef thisObject) :
		KMethod("JavaScript.KKJSMethod"),
		KKJSObject(context, jsobject),
		thisObject(thisObject)
	{
		KJSUtil::ProtectJSObject(this->context, thisObject);
	}

	KKJSMethod::~KKJSMethod()
	{
		if (this->thisObject != NULL) 
		{
			KJSUtil::UnprotectJSObject(this->context, this->thisObject);
		}
	}

	KValueRef KKJSMethod::Call(const ValueList& args)
	{
		if(! KJSUtil::HasContext(this->context))
		{
			Logger::Get("KKSJMethod")->Warn("Can't call this method with no context");
			return Value::Undefined;
		}

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
}
