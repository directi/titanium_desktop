/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008, 2009 Appcelerator, Inc. All Rights Reserved.
 */


#include <cstdio>
#include <cstring>

#include "value.h"
#include "k_accessor_list.h"


namespace kroll
{
	KAccessorList::KAccessorList(const char* type)
		: StaticBoundList(type), KAccessorObject(type)
	{
	}
}

