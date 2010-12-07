/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#include <kroll/host.h>
#include <kroll/Assertion.h>
			
namespace kroll
{
	bool IsMainThread()
	{
		return Host::GetInstance()->IsMainThread();
	}
}
