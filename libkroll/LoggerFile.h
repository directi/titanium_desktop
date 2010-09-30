/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _KROLL_LOGGERFILE_H_
#define _KROLL_LOGGERFILE_H_


#ifdef OS_OSX
#import <Foundation/Foundation.h>
#endif

#include <list>
#include <string>

#include "LoggerWriter.h"

namespace kroll
{
	class KROLL_API LoggerFile
	{
		public:
			LoggerFile(const std::string &filename);
			virtual ~LoggerFile();
			void log(std::string& data);
			virtual void dumpToFile();

		protected:
			std::string filename;
			Poco::Mutex loggerMutex;
			std::list<std::string> writeQueue;
	};
}

#endif