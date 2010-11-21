/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _KR_PROFILED_GLOBAL_OBJECT_H_
#define _KR_PROFILED_GLOBAL_OBJECT_H_

#include "global_object.h"
#include "profiled_bound_object.h"

namespace kroll
{
	class KROLL_API ProfiledGlobalObject : public GlobalObject, public ProfiledBoundObject
	{
	public:
		ProfiledGlobalObject(KObjectRef delegate);
		virtual ~ProfiledGlobalObject();
	};
}

#endif
