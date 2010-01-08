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
	class ReferenceCountedLogger
	{
		public:
			Poco::Mutex referencesMutex;
			int references;
			LoggerFile * file;

			ReferenceCountedLogger(LoggerFile *_file = NULL)
				: file(_file), references(0) { }

			void addRef()
			{
				Poco::Mutex::ScopedLock lock(referencesMutex);
				references++;
			}

			void release()
			{
				Poco::Mutex::ScopedLock lock(referencesMutex);
				--references;
			}

			int getReferencesCount()
			{
				Poco::Mutex::ScopedLock lock(referencesMutex);
				return references;
			}
	};

	class Logger
		: public StaticBoundObject
	{
		private:
			std::string fileName;

			static std::map<std::string, ReferenceCountedLogger *> files;
			static Poco::Mutex filesMutex;

		public:
			Logger(const std::string &filename);
			Logger(const std::string &filename, const std::string &rootXMLText, const std::string &xsltFile);
			virtual ~Logger();
			
			void Log(const ValueList& args, KValueRef result);
	};
}

#endif