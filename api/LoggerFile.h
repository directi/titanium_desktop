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
#include <vector>
#include <string>


namespace kroll
{
	class LoggerFile;

	class LoggerWriter
		: public Poco::Runnable
	{
		private:
			static LoggerWriter * singleton;
			std::vector<LoggerFile *> files;
			bool bRunning;
			Poco::Thread thread;
			Poco::Event pendingMsgEvent;

		protected:
			LoggerWriter();

		public:
			static void addLoggerFile(LoggerFile * file);
			static void removeLoggerFile(LoggerFile * file);
			static void notify(LoggerFile * file);
			void addFile(LoggerFile * file);
			void removeFile(LoggerFile * file);
			void start();
			void stop();
			void notify();
			virtual void run();
	};

	class KROLL_API LoggerFile
	{
		public:
			LoggerFile(const std::string &filename);
			virtual ~LoggerFile();

			void log(std::string& data);

			virtual void dumpToFile();

		protected:
			std::string filename;
			std::list<std::string> writeQueue;
			Poco::Mutex loggerMutex;
	};
}

#endif