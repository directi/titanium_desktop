/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */


#include <cstdio>
#include <cstring>
#include <iostream>
#include <sstream>

#include "value.h"
#include "profiled_bound_list.h"
#include "profiled_bound_method.h"

#include <kroll/host.h>
#include <kroll/Assertion.h>
#include <kroll/utils/time_utils.h>

namespace kroll
{
	std::ofstream * ProfiledBoundObject::stream = NULL;

	ProfiledBoundObject::ProfiledBoundObject(KObjectRef delegate, const std::string& parentType) :
		KObject(delegate->GetType()),
		delegate(delegate),
		parentType(parentType)
	{
	}
	ProfiledBoundObject::~ProfiledBoundObject()
	{
	}

	void ProfiledBoundObject::SetStream(std::ofstream* stream)
	{
		ProfiledBoundObject::stream = stream;
	}

	KObjectRef ProfiledBoundObject::Wrap(KObjectRef value, std::string parentType)
	{
		ProfiledBoundObject* po = dynamic_cast<ProfiledBoundObject*>(value.get());
		if(! po)
		{
			KObjectRef wrapped = new ProfiledBoundObject(value, parentType);
			return wrapped;
		}
		return value;
	}

	void ProfiledBoundObject::Set(const char *name, KValueRef value)
	{
		std::string type = this->GetSubType(name);
		KValueRef result = Value::Wrap(value);

		TimeUtils::StopWatch sw(true);
		delegate->Set(name, result);
		sw.stop();

		this->Log("set", type, sw.elapsed());
	}

	KValueRef ProfiledBoundObject::Get(const char *name)
	{
		std::string type = this->GetSubType(name);

		TimeUtils::StopWatch sw(true);
		KValueRef value = delegate->Get(name);
		sw.stop();

		this->Log("get", type, sw.elapsed());
		return Value::Wrap(value);
	}

	SharedStringList ProfiledBoundObject::GetPropertyNames()
	{
		return delegate->GetPropertyNames();
	}

	void ProfiledBoundObject::Log(
		const char* eventType,
		const std::string& name,
		double elapsedTime)
	{
		ASSERT_MAIN_THREAD
		if ((*ProfiledBoundObject::stream)) {
			*ProfiledBoundObject::stream << Host::GetInstance()->GetElapsedTime() << ",";
			*ProfiledBoundObject::stream << eventType << ",";
			*ProfiledBoundObject::stream << name << ",";
			*ProfiledBoundObject::stream << elapsedTime << "," << std::endl;
		}
	}

	SharedString ProfiledBoundObject::DisplayString(int levels)
	{
		return delegate->DisplayString(levels);
	}

	bool ProfiledBoundObject::HasProperty(const char* name)
	{
		return delegate->HasProperty(name);
	}

	bool ProfiledBoundObject::Equals(KObjectRef other)
	{
		AutoPtr<ProfiledBoundObject> pother = other.cast<ProfiledBoundObject>();
		if (!pother.isNull())
			other = pother->delegate;

		return other.get() == this->delegate.get();
	}

	std::string ProfiledBoundObject::GetSubType(std::string name)
	{
		if (!this->GetType().empty())
		{
			return this->GetType() + "." + name;
		}
		else
		{
			return name;
		}
	}
}
