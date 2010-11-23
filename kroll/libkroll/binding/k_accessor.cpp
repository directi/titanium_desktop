/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#include "k_accessor.h"
#include "value.h"
#include "kmethod.h"


namespace kroll
{
	KAccessor::KAccessor()
	{
	}

	void KAccessor::RecordAccessor(const std::string& name, KValueRef value)
	{
		if (name.find("set") == 0)
			DoMap(name.substr(3), value, setterMap);

		else if (name.find("get") == 0)
			DoMap(name.substr(3), value, getterMap);

		else if (name.find("is") == 0)
			DoMap(name.substr(2), value, getterMap);
	}

	bool KAccessor::HasGetterFor(std::string name)
	{
		return !FindAccessor(name, getterMap).isNull();
	}

	KValueRef KAccessor::UseGetter(std::string name, KValueRef existingValue)
	{
		if (!existingValue->IsUndefined())
			return existingValue;

		KMethodRef getter = FindAccessor(name, getterMap);
		if (getter.isNull())
			return existingValue;

		return getter->Call();
	}

	bool KAccessor::UseSetter(std::string name, KValueRef newValue, KValueRef existingValue)
	{
		RecordAccessor(name, newValue);

		// If a property already exists on this object with the given
		// name, just set the property and don't call the setter.
		if (!existingValue->IsUndefined())
			return false;

		KMethodRef setter = FindAccessor(name, setterMap);
		if (setter.isNull())
			return false;

		setter->Call(newValue);
		return true;
	}

	void KAccessor::DoMap(std::string name, KValueRef accessor, AccessorMap& map)
	{
		// Lower-case the name so that all comparisons are case-insensitive.
		std::transform(name.begin(), name.end(), name.begin(), tolower);

		// Null old mapping if it exists. This is so that if an accessor
		// is replaced with a non-accessor, we don't keep a copy of it around.
		if (map.find(name) != map.end())
			map[name] = 0;

		if (!accessor->IsMethod())
			return;

		KMethodRef m = accessor->ToMethod();
		map[name] = m;
	}

	KMethodRef KAccessor::FindAccessor(std::string& name, AccessorMap& map)
	{
		// Lower-case the name so that all comparisons are case-insensitive.
		std::transform(name.begin(), name.end(), name.begin(), tolower);

		if (map.find(name) == map.end())
			return 0;

		return map[name];
	}
}
