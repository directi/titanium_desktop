/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */


#include <cstdio>
#include <cstring>

#include "value.h"
#include "profiled_bound_list.h"

namespace kroll
{
	ProfiledBoundList::ProfiledBoundList(KListRef delegate, std::string& parentType) :
		ProfiledBoundObject(delegate, parentType),
		list(delegate)
	{
	}

	ProfiledBoundList::~ProfiledBoundList()
	{
	}

	void ProfiledBoundList::Append(KValueRef value)
	{
		list->Append(value);
	}

	unsigned int ProfiledBoundList::Size()
	{
		return list->Size();
	}

	KValueRef ProfiledBoundList::At(unsigned int index)
	{
		return list->At(index);
	}

	void ProfiledBoundList::SetAt(unsigned int index, KValueRef value)
	{
		list->SetAt(index,value);
	}

	bool ProfiledBoundList::Remove(unsigned int index)
	{
		return list->Remove(index);
	}

	void ProfiledBoundList::Set(const char *name, KValueRef value)
	{
		ProfiledBoundObject::Set(name, value);
	}

	KValueRef ProfiledBoundList::Get(const char *name)
	{
		return ProfiledBoundObject::Get(name);
	}

	SharedStringList ProfiledBoundList::GetPropertyNames()
	{
		return ProfiledBoundObject::GetPropertyNames();
	}

	bool ProfiledBoundList::HasProperty(const char* name)
	{
		return ProfiledBoundObject::HasProperty(name);
	}

	KListRef ProfiledBoundList::Wrap(KListRef value, std::string parentType)
	{
		ProfiledBoundList* po = dynamic_cast<ProfiledBoundList*>(value.get());
		if(! po)
		{
			return new ProfiledBoundList(value, parentType);
		}
		return value;
	}
}
