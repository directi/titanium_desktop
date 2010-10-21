/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _KR_ZIP_FILE_UTILS_H_
#define _KR_ZIP_FILE_UTILS_H_
	
#include <base.h>

namespace UTILS_NS
{
	namespace FileUtils
	{
		typedef bool (*UnzipCallback)(char* message, int current,
			int total, void* data);
		KROLL_API bool Unzip(const std::string& source, const std::string& destination, 
			UnzipCallback callback=0, void* data=0);
	}
}

#endif
