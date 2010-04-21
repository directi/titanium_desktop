/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _KROLL_LOGGERFILE_H_
#define _KROLL_LOGGERFILE_H_

#include <Poco/Thread.h>
#include <Poco/Event.h>

#ifdef OS_OSX
#import <Foundation/Foundation.h>
#endif

#include <list>
#include <string>


namespace kroll
{
	class KROLL_API LoggerFile
		: public Poco::Runnable
	{
		public:
			LoggerFile(const std::string &filename);
			virtual ~LoggerFile();

			void log(std::string& data);

			virtual void run();
			virtual void dumpToFile();

		private:
			Poco::Thread thread;
			bool bRunning;
			Poco::Event pendingMsgEvent;
		protected:
			std::string filename;
			std::list<std::string> writeQueue;
			Poco::Mutex loggerMutex;
	};
}

#endif