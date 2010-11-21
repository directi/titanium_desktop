/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "javascript_module.h"

namespace kroll
{
	KKJSList::KKJSList(JSContextRef context, JSObjectRef jsobject) :
		KList("JavaScript.KKJSList"),
		KKJSObject(context, jsobject)
	{
	}

	KKJSList::~KKJSList()
	{
		
	}

	unsigned int KKJSList::Size()
	{
		KValueRef length_val = this->Get("length");
		if (length_val->IsInt())
			return (unsigned int) length_val->ToInt();
		else
			return 0;
	}

	KValueRef KKJSList::At(unsigned int index)
	{
		std::string name = KList::IntToChars(index);
		KValueRef value = this->Get(name.c_str());
		return value;
	}

	void KKJSList::SetAt(unsigned int index, KValueRef value)
	{
		std::string name = KList::IntToChars(index);
		this->Set(name.c_str(), value);
	}

	void KKJSList::Append(KValueRef value)
	{
		KValueRef push_method = this->Get("push");

		if (push_method->IsMethod())
		{
			ValueList list;
			list.push_back(value);
			push_method->ToMethod()->Call(list);
		}
		else
		{
			throw ValueException::FromString("Could not find push method on KJS array.");
		}
	}

	bool KKJSList::Remove(unsigned int index)
	{
		if (index >= 0 && index < this->Size())
		{
			KValueRef spliceMethod = this->Get("splice");
			spliceMethod->ToMethod()->Call(
				Value::NewInt(index),
				Value::NewInt(1));
			return true;
		}
		return false;
	}
}
