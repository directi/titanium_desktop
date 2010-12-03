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

KROLL_API KValueRef ToKrollValue(JSValueRef, JSGlobalContextRef, JSObjectRef);
KROLL_API JSValueRef ToJSValue(KValueRef, JSGlobalContextRef);
KROLL_API JSValueRef KObjectToJSValue(KValueRef, JSGlobalContextRef);
KROLL_API JSValueRef KMethodToJSValue(KValueRef, JSGlobalContextRef);
KROLL_API JSValueRef KListToJSValue(KValueRef, JSGlobalContextRef);
KROLL_API std::string ToChars(JSStringRef);
KROLL_API bool IsArrayLike(JSObjectRef, JSGlobalContextRef);
KROLL_API JSGlobalContextRef CreateGlobalContext();
KROLL_API JSGlobalContextRef GetGlobalContext(JSContextRef);
KROLL_API void RegisterContext(JSGlobalContextRef);
KROLL_API void UnregisterContext(JSGlobalContextRef);

KROLL_API void ProtectJSObject(JSGlobalContextRef, JSObjectRef);
KROLL_API void UnprotectJSObject(JSGlobalContextRef, JSObjectRef);

KROLL_API KValueRef Evaluate(JSGlobalContextRef context, const char* script,
	 const char* url = "string");
KROLL_API KValueRef EvaluateFile(JSGlobalContextRef context,
	const std::string& fullPath);

bool HasContext(JSGlobalContextRef ctx);
};
}

#endif
