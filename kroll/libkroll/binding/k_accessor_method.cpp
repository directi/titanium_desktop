/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008, 2009 Appcelerator, Inc. All Rights Reserved.
 */


#include <cstdio>
#include <cstring>

#include "value.h"
#include "k_accessor_method.h"

namespace kroll
{
	KAccessorMethod::KAccessorMethod(MethodCallback* callback, const char *type)
		: StaticBoundMethod(callback, type)
	{
	}

	bool KAccessorMethod::HasProperty(const char* name)
	{
		return StaticBoundMethod::HasProperty(name) || this->HasGetterFor(name);
	}

	void KAccessorMethod::Set(const char* name, KValueRef value)
	{
		if (!this->UseSetter(name, value, StaticBoundMethod::Get(name)))
			StaticBoundMethod::Set(name, value);
	}

	KValueRef KAccessorMethod::Get(const char* name)
	{
		return this->UseGetter(name, StaticBoundMethod::Get(name));
	}
}

