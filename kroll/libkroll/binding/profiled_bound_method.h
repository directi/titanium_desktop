/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _KR_PROFILED_BOUND_METHOD_H_
#define _KR_PROFILED_BOUND_METHOD_H_

#include "kmethod.h"
#include "profiled_bound_object.h"

namespace kroll
{
	/**
	 * The ProfiledBoundMethod is a wrapped KMethod that does profiling
	 */
	class ProfiledBoundMethod : public ProfiledBoundObject, public KMethod
	{
	public:
		ProfiledBoundMethod(KMethodRef delegate, std::string& parentType);
		virtual ~ProfiledBoundMethod();

		// @see KMethod::Call
		virtual KValueRef Call(const ValueList& args);
		// @see KMethod::Set
		virtual void Set(const char *name, KValueRef value) { ProfiledBoundObject::Set(name, value); }
		// @see KMethod::Get
		virtual KValueRef Get(const char *name) { return ProfiledBoundObject::Get(name); }
		// @see KMethod::GetPropertyNames
		virtual SharedStringList GetPropertyNames() { return ProfiledBoundObject::GetPropertyNames(); }
		// @see KObject::GetType
		virtual std::string& GetType() { return ProfiledBoundObject::GetType(); }

		static KMethodRef Wrap(KMethodRef value, std::string type); 
	private:
		KMethodRef method;
	};
}

#endif
