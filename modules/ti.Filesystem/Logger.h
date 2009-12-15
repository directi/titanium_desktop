/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _TI_LOGGER_H_
#define _TI_LOGGER_H_

#include <map>
#include "LoggerFile.h"


namespace ti
{
	class Logger
		: public StaticBoundObject
	{
		private:
			std::string fileName;
			LoggerFile *currentFile;

			static std::map<std::string, LoggerFile *> files;
			static Poco::Mutex filesMutex;

		public:
			Logger(const std::string &filename);
			virtual ~Logger();
			
			void Log(const ValueList& args, KValueRef result);
	};
}

#endif