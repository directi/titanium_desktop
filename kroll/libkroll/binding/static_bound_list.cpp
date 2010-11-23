/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */


#include <cmath>
#include <cstdio>
#include <cstring>

#include "value.h"
#include "static_bound_list.h"

namespace kroll
{
	StaticBoundList::StaticBoundList(const char *type) :
		KList(type),
		StaticBoundObject(type),
		length(0)
	{
	}

	StaticBoundList::~StaticBoundList()
	{
	}

	void StaticBoundList::Append(KValueRef value)
	{
		std::string name = KList::IntToChars(this->length);
		StaticBoundObject::Set(name.c_str(), value);
		this->length++;
	}

	void StaticBoundList::SetAt(unsigned int index, KValueRef value)
	{
		std::string name = KList::IntToChars(index);
		StaticBoundObject::Set(name.c_str(), value);
		if (index >= this->length)
			this->length = index + 1;
	}

	bool StaticBoundList::Remove(unsigned int index)
	{
		if (index >= this->length)
			return false;

		std::string name = KList::IntToChars(index);
		StaticBoundObject::Unset(name.c_str());
		for (unsigned int i = index; i + 1 < this->length; i++)
			this->SetAt(i, this->At(i + 1));

		this->length--;
		return true;
	}

	unsigned int StaticBoundList::Size()
	{
		return this->length;
	}

	KValueRef StaticBoundList::At(unsigned int index)
	{
		std::string name = KList::IntToChars(index);
		KValueRef value = StaticBoundObject::Get(name.c_str());
		return value;
	}

	void StaticBoundList::Set(const char *name, KValueRef value)
	{
		int index = -1;
		if (KList::IsInt(name) && (index = atoi(name)) >= 0)
		{
			this->SetAt(index, value);
		}
		else
		{
			if(StaticBoundObject::Get(name).isNull()) length++;
			StaticBoundObject::Set(name, value);
		}
	}

	KValueRef StaticBoundList::Get(const char *name)
	{
		int index = -1;
		if (KList::IsInt(name) && (index = atoi(name)) >= 0)
		{
			return this->At(index);
		}
		else
		{
			return StaticBoundObject::Get(name);
		}
	}

	KListRef StaticBoundList::FromStringVector(std::vector<std::string>& values)
	{
		KListRef l = new StaticBoundList();
		std::vector<std::string>::iterator i = values.begin();
		while (i != values.end())
		{
			l->Append(Value::NewString(*i));
			i++;
		}
		return l;
	}

	SharedStringList StaticBoundList::GetPropertyNames()
	{
		return StaticBoundObject::GetPropertyNames();
	}
}

