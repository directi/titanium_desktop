/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */


#include <cstdio>
#include <cstring>
#include <iostream>
#include <sstream>
#include <Poco/Stopwatch.h>
#include <Poco/ScopedLock.h>

#include "value.h"
#include "profiled_global_object.h"

namespace kroll
{
	ProfiledGlobalObject::ProfiledGlobalObject(KObjectRef global) :
ProfiledBoundObject(global, std::string("GlobalObject"))
	{
	}

	ProfiledGlobalObject::~ProfiledGlobalObject()
	{
	}
}
