/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */


#include <sstream>
#include <cmath>

#include "value.h"
#include "arg_list.h"
#include "kmethod.h"
#include "profiled_bound_method.h"

namespace kroll
{

	SharedString KMethod::DisplayString(int levels)
	{
		std::ostringstream oss;
		oss << "<" << this->GetType() << " at " << this << ">";
		return new std::string(oss.str());
	}

	KValueRef KMethod::Call(KObjectRef thisObject, const ValueList& args)
	{
		return this->Call(args);
	}

	KValueRef KMethod::Call()
	{
		return this->Call(ValueList());
	}

	KValueRef KMethod::Call(const char* one, KValueRef two, KValueRef three,
		KValueRef four)
	{
		ValueList args;
		args.push_back(Value::NewString(one));
		args.push_back(two);
		args.push_back(three);
		args.push_back(four);
		return this->Call(args);
	}

	KValueRef KMethod::Call(KValueRef one)
	{
		ValueList args;
		args.push_back(one);
		return this->Call(args);
	}

	KValueRef KMethod::Call(KValueRef one, KValueRef two)
	{
		ValueList args;
		args.push_back(one);
		args.push_back(two);
		return this->Call(args);
	}

	KValueRef KMethod::Call(KValueRef one, KValueRef two, KValueRef three)
	{
		ValueList args;
		args.push_back(one);
		args.push_back(two);
		args.push_back(three);
		return this->Call(args);
	}

	KValueRef KMethod::Call(const char* one)
	{
		ValueList args;
		args.push_back(Value::NewString(one));
		return this->Call(args);
	}

	KValueRef KMethod::Call(const char* one, KValueRef two)
	{
		ValueList args;
		args.push_back(Value::NewString(one));
		args.push_back(two);
		return this->Call(args);
	}

	KValueRef KMethod::Call(const char* one, KValueRef two, KValueRef three)
	{
		ValueList args;
		args.push_back(Value::NewString(one));
		args.push_back(two);
		args.push_back(three);
		return this->Call(args);
	}
}

