/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _KR_PROFILED_BOUND_OBJECT_H_
#define _KR_PROFILED_BOUND_OBJECT_H_
#include <Poco/FileStream.h>
#include <Poco/Mutex.h>

#include <base.h>

#include "kobject.h"

namespace kroll
{
	/**
	 * The ProfiledBoundObject is a wrapped KObject that does profiling on a 
	 * wrapped KObject
	 */
	class KROLL_API ProfiledBoundObject : public KObject
	{
		public:
			ProfiledBoundObject(KObjectRef delegate, std::string& parentType);
		virtual ~ProfiledBoundObject();
		static void SetStream(Poco::FileOutputStream*);

		public:
		// @see KObject::Set
		virtual void Set(const char *name, KValueRef value);
		// @see KObject::Get
		virtual KValueRef Get(const char *name);
		// @see KObject::GetPropertyNames
		virtual SharedStringList GetPropertyNames();
		// @see KObject::DisplayString
		virtual SharedString DisplayString(int levels=3);
		// @see KObject::Equals
		virtual bool Equals(KObjectRef other);

		bool HasProperty(const char* name);

		static KObjectRef Wrap(KObjectRef value, std::string type); 

	protected:
		KObjectRef delegate;
		std::string parentType;
		std::string GetSubType(std::string name);
		void Log(const char* eventType, std::string& name, Poco::Timestamp::TimeDiff);
		static Poco::FileOutputStream *stream;
		static Poco::Mutex logMutex;
	};
}

#endif
