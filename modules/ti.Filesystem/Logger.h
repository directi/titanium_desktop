/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _TI_LOGGER_H_
#define _TI_LOGGER_H_
#include <kroll/kroll.h>
#include <Poco/Thread.h>

#ifdef OS_WIN32
#include <windows.h>
#include <commdlg.h>
#include <shellapi.h>
#include <shlobj.h>
#elif OS_OSX
#import <Foundation/Foundation.h>
#endif

#include <string>
#include <list>

namespace ti
{
	class Logger
	  : public StaticBoundObject,
	    public Poco::Runnable
	{
		public:
		Logger(const std::string &filename);
		virtual ~Logger();

		std::string& GetFilename() { return filename; }
		virtual SharedString DisplayString(int levels=3)
		{
			return new string(GetFilename());
		}

		virtual void run();

		void Log(const ValueList& args, KValueRef result);
		void Log();

		private:
		std::string filename;
		std::list<std::string> writeQueue;
		Poco::Thread thread;
		bool bRunning;
		Poco::Mutex loggerMutex; 
	};
}

#endif