/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _KJS_KMETHOD_H_
#define _KJS_KMETHOD_H_

#include "javascript_module.h"
#include <JavaScriptCore/JSWeakObjectMapRefPrivate.h>

#include <vector>
#include <string>
#include <map>

namespace kroll
{
	typedef std::map<JSContextRef, JSWeakObjectMapRef> ContextRefs;

	class KROLL_API KKJSMethod : public KMethod
	{
		public:
		KKJSMethod(JSContextRef, JSObjectRef, JSObjectRef);
		~KKJSMethod();

		virtual void Set(const char *name, KValueRef value);
		virtual KValueRef Get(const char *name);
		virtual KValueRef Call(const ValueList& args);
		virtual SharedStringList GetPropertyNames();
		virtual bool HasProperty(const char* name);
		virtual bool Equals(KObjectRef);

		virtual bool SameContextGroup(JSContextRef c);
		JSObjectRef GetJSObject();

		protected:
		JSContextRef context;
		JSObjectRef jsobject;
		JSObjectRef thisObject;
		AutoPtr<KKJSObject> kobject;

		private:
			JSObjectRef jsRef;
			static ContextRefs contextRefs;
			static Poco::Mutex contextRefsMutex;
			static void MapDestroyed(JSWeakObjectMapRef, void *);
			static void RegisterMethod(KKJSMethod *);
			static void UnregisterMethod(KKJSMethod *);
		
			DISALLOW_EVIL_CONSTRUCTORS(KKJSMethod);
	};
}

#endif
