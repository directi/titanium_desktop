/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _KR_DATA_UTILS_H_
#define _KR_DATA_UTILS_H_
#include <string>

#include <base.h>

namespace UTILS_NS
{
	namespace DataUtils
	{
		/**
		 * Generate a new UUID
		 * @returns a new UUID as a string
		 */
		KROLL_API std::string GenerateUUID();
	}
}
#endif
