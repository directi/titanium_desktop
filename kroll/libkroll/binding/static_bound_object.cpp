/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */


#include <cstring>

#include "value.h"
#include "static_bound_list.h"
#include <kroll/Assertion.h>

namespace kroll
{
	StaticBoundObject::StaticBoundObject(const char* type)
		: KObject(type)
	{
	}

	StaticBoundObject::~StaticBoundObject()
	{
	}

	bool StaticBoundObject::HasProperty(const char* name)
	{
		ASSERT_MAIN_THREAD
		return properties.find(name) != properties.end();
	}
	
	KValueRef StaticBoundObject::Get(const char* name)
	{
		ASSERT_MAIN_THREAD
		std::map<std::string, KValueRef>::iterator iter = 
			properties.find(std::string(name));

		if (iter == properties.end())
			return Value::Undefined;
		return iter->second;
	}

	void StaticBoundObject::Set(const char* name, KValueRef value)
	{
		ASSERT_MAIN_THREAD
		this->properties[std::string(name)] = value;
	}

	void StaticBoundObject::Unset(const char* name)
	{
		ASSERT_MAIN_THREAD
		std::map<std::string, KValueRef>::iterator iter = 
			properties.find(std::string(name));

		if (this->properties.end() == iter)
			return;
		this->properties.erase(iter);
	}

	SharedStringList StaticBoundObject::GetPropertyNames()
	{
		ASSERT_MAIN_THREAD
		SharedStringList list(new StringList());
		std::map<std::string, KValueRef>::iterator iter = properties.begin();

		while (iter != properties.end())
			list->push_back(new std::string((iter++)->first));

		return list;
	}

}
