/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _KJS_KMETHOD_H_
#define _KJS_KMETHOD_H_

#include "javascript_module.h"
#if BARK_UP_WEAKREF_TREE
#include <JavaScriptCore/JSWeakObjectMapRefPrivate.h>
#endif

#include <vector>
#include <string>
#include <map>

namespace kroll
{
#if BARK_UP_WEAKREF_TREE
	typedef std::map<JSContextRef, JSWeakObjectMapRef> ContextRefs;
#endif

	class KROLL_API KKJSMethod : public KMethod, public KKJSObject
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
		virtual void release();
		virtual void duplicate();

		virtual bool SameContextGroup(JSContextRef c);
		JSObjectRef GetJSObject();
		JSContextRef Context() { return context; }

		protected:
		JSContextRef context;
		JSObjectRef jsobject;
		JSObjectRef thisObject;

		private:
#if BARK_UP_WEAKREF_TREE
			static ContextRefs contextRefs;
			static Poco::Mutex contextRefsMutex;
			static void MapDestroyed(JSWeakObjectMapRef, void *);
			static void RegisterMethod(KKJSMethod *);
			static void UnregisterMethod(KKJSMethod *);
#endif
			DISALLOW_EVIL_CONSTRUCTORS(KKJSMethod);
	};
}

#endif
