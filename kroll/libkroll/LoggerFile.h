/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _KROLL_LOGGERFILE_H_
#define _KROLL_LOGGERFILE_H_

#include <list>
#include <string>
#include <fstream>

#include "base.h"
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
		boost::mutex loggerMutex;
		std::list<std::string> writeQueue;
	private:
		std::ofstream stream;
	};
}

#endif