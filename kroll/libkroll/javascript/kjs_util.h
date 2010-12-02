/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _KJS_UTIL_H_
#define _KJS_UTIL_H_

#include <JavaScriptCore/JSObjectRef.h>
#include <JavaScriptCore/JSStringRef.h>
#include <JavaScriptCore/JSContextRef.h>

#include <base.h>
#include "../binding/binding.h"

namespace kroll
{
namespace KJSUtil
{

KROLL_API KValueRef ToKrollValue(JSValueRef, JSContextRef, JSObjectRef);
KROLL_API JSValueRef ToJSValue(KValueRef, JSContextRef);
KROLL_API JSValueRef KObjectToJSValue(KValueRef, JSContextRef);
KROLL_API JSValueRef KMethodToJSValue(KValueRef, JSContextRef);
KROLL_API JSValueRef KListToJSValue(KValueRef, JSContextRef);
KROLL_API std::string ToChars(JSStringRef);
KROLL_API bool IsArrayLike(JSObjectRef, JSContextRef);
KROLL_API JSGlobalContextRef CreateGlobalContext();
KROLL_API JSGlobalContextRef GetGlobalContext(JSContextRef);
KROLL_API void RegisterContext(JSGlobalContextRef);
KROLL_API void UnregisterContext(JSGlobalContextRef);

KROLL_API void ProtectJSObject(JSGlobalContextRef, JSObjectRef);
KROLL_API void UnprotectJSObject(JSGlobalContextRef, JSObjectRef);

KROLL_API KValueRef Evaluate(JSContextRef context, const char* script,
	 const char* url = "string");
KROLL_API KValueRef EvaluateFile(JSContextRef context,
	const std::string& fullPath);

};
}

#endif
