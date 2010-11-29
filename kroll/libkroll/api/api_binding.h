/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _API_BINDING_H_
#define _API_BINDING_H_

#include <map>
#include <string>

#include <kroll/binding/value.h>
#include <kroll/binding/k_accessor_object.h>

#include <kroll/host.h>

namespace kroll
{
	class APIBinding : public KAccessorObject
	{
		public:
		APIBinding(Host* host);
		virtual ~APIBinding();

		void Log(int severity, KValueRef);
		static KListRef ManifestToKList(const map<string, string>& manifest);

		private:
		Host* host;
		KObjectRef global;
		Logger* logger;

		void _Set(const ValueList& args, KValueRef result);
		void _Get(const ValueList& args, KValueRef result);
		void _AddEventListener(const ValueList& args, KValueRef result);
		void _RemoveEventListener(const ValueList& args, KValueRef result);
		void _FireEvent(const ValueList& args, KValueRef result);

		Logger::Level ValueToLevel(KValueRef v);
		void _SetLogLevel(const ValueList& args, KValueRef result);
		void _GetLogLevel(const ValueList& args, KValueRef result);

		void _Print(const ValueList& args, KValueRef result);
		void _Log(const ValueList& args, KValueRef result);
		void _LogTrace(const ValueList& args, KValueRef result);
		void _LogDebug(const ValueList& args, KValueRef result);
		void _LogInfo(const ValueList& args, KValueRef result);
		void _LogNotice(const ValueList& args, KValueRef result);
		void _LogWarn(const ValueList& args, KValueRef result);
		void _LogError(const ValueList& args, KValueRef result);
		void _LogCritical(const ValueList& args, KValueRef result);
		void _LogFatal(const ValueList& args, KValueRef result);

		void _CreateKObject(const ValueList& args, KValueRef result);
		void _CreateKMethod(const ValueList& args, KValueRef result);
		void _CreateKList(const ValueList& args, KValueRef result);
		void _CreateBytes(const ValueList& args, KValueRef result);
	};

	/**
	 * An wrapper for a KObject which encapsulates another one for testing
	 */
	class KObjectWrapper : public KObject
	{
	public:
		KObjectWrapper(KObjectRef object);
		void Set(const char *name, KValueRef value);
		KValueRef Get(const char *name);
		bool HasProperty(const char *name);
		SharedStringList GetPropertyNames();
		SharedString DisplayString(int levels);
		bool Equals(KObjectRef other);

	private:
		KObjectRef object;
	};

	/**
	 * An wrapper for a KMethod which encapsulates another one for testing
	 */
	class KMethodWrapper : public KMethod
	{
	public:
		KMethodWrapper(KMethodRef method);
		KValueRef Call(const ValueList& args);
		void Set(const char *name, KValueRef value);
		KValueRef Get(const char *name);
		bool HasProperty(const char *name);
		SharedStringList GetPropertyNames();
		SharedString DisplayString(int levels);
		bool Equals(KObjectRef other);

	private:
		KMethodRef method;
	};

	/**
	 * An wrapper for a KList which encapsulates another one for testing
	 */
	class KListWrapper : public KList
	{
	public:
		KListWrapper(KListRef list);
		void Append(KValueRef value);
		unsigned int Size();
		KValueRef At(unsigned int index);
		void SetAt(unsigned int index, KValueRef value);
		bool Remove(unsigned int index);
		void Set(const char *name, KValueRef value);
		KValueRef Get(const char *name);
		bool HasProperty(const char *name);
		SharedStringList GetPropertyNames();
		SharedString DisplayString(int levels=3);
		bool Equals(KObjectRef other);
	private:
		KListRef list;
	};
}

#endif
