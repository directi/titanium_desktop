/**
 * @author "Mital Vora<mital.d.vora@gmail.com>"
 * Licenced under GNU LGPL.
 * see LICENSE in the root folder for details on the license.
 */
#ifndef _KROLL_LOGGERWRITER_H_
#define _KROLL_LOGGERWRITER_H_

#include <Poco/Event.h>
#include <Poco/Thread.h>

#ifdef OS_OSX
#import <Foundation/Foundation.h>
#endif

#include <vector>

namespace kroll
{
	class LoggerFile;

	class LoggerWriter
		: public Poco::Runnable
	{
		private:
			static LoggerWriter * singleton;
			static Poco::Mutex singletonAccessMutex;

		public:
			static LoggerWriter* getInstance();

		private:
			Poco::Mutex filesMutex;
			std::vector<LoggerFile *> files;
			
			bool bRunning;
			Poco::Thread thread;
			Poco::Event pendingMsgEvent;

		protected:
			LoggerWriter();
			virtual ~LoggerWriter();
			virtual void run();
			void start();
			void stop();

		public:
			void addLoggerFile(LoggerFile * file);
			void removeLoggerFile(LoggerFile * file);
			void notify(LoggerFile * file);
	};
}

#endif
