/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */


#include <cstdio>
#include <cstring>
#include <Poco/Stopwatch.h>

#include "value.h"
#include "profiled_bound_method.h"

namespace kroll
{
	ProfiledBoundMethod::ProfiledBoundMethod(KMethodRef delegate, std::string& parentType) :
		ProfiledBoundObject(delegate, parentType),
		method(delegate)
	{
	}

	ProfiledBoundMethod::~ProfiledBoundMethod()
	{
	}

	KValueRef ProfiledBoundMethod::Call(const ValueList& args)
	{
		std::string type = this->GetType();

		KValueRef value;
		Poco::Stopwatch sw;
		sw.start();
		try {
			value = method->Call(args);
		} catch (...) {
			sw.stop();
			this->Log("call", type, sw.elapsed());
			throw;
		}

		sw.stop();
		this->Log("call", type, sw.elapsed());
		return Value::Wrap(value);
	}

	KMethodRef ProfiledBoundMethod::Wrap(KMethodRef value, std::string parentType)
	{
		ProfiledBoundMethod* po = dynamic_cast<ProfiledBoundMethod*>(value.get());
		if(! po)
		{
			return new ProfiledBoundMethod(value, parentType);
		}
		return value;
	}
}
