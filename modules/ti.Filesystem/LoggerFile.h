/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _TI_LOGGERFILE_H_
#define _TI_LOGGERFILE_H_

#include <kroll/kroll.h>
#include <Poco/Thread.h>
#include <Poco/Event.h>

#ifdef OS_OSX
#import <Foundation/Foundation.h>
#endif

#include <list>
#include <string>


namespace ti
{
	class LoggerFile
		: public Poco::Runnable
	{
		public:
			LoggerFile(const std::string &filename);
			virtual ~LoggerFile();

			void log(std::string& data);

			virtual void run();
			void dumpToFile();

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

		private:
			int references;
			Poco::Mutex referencesMutex;
			std::string filename;
			std::list<std::string> writeQueue;
			Poco::Thread thread;
			bool bRunning;
			Poco::Mutex loggerMutex;
			Poco::Event pendingMsgEvent;
	};
}

#endif