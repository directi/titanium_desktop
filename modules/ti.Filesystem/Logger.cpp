/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#include "Logger.h"
#include "filesystem_utils.h"

#include <Poco/File.h>
#include <Poco/Path.h>
#include <Poco/FileStream.h>
#include <Poco/Exception.h>


#ifndef OS_WIN32
#include <sys/types.h>
#include <sys/stat.h>
#endif

#ifdef OS_LINUX
#include <sys/statvfs.h>
#endif

#ifdef OS_WIN32
#define MIN_PATH_LENGTH 3
#else
#define MIN_PATH_LENGTH 1
#endif


namespace ti
{
	Logger::Logger(const std::string &filename) :
		StaticBoundObject("Filesystem.Logger"),
		bRunning(false)
	{
		Poco::Path pocoPath(Poco::Path::expand(filename));
		this->filename = pocoPath.absolute().toString();

		// If the filename we were given contains a trailing slash, just remove it
		// so that users can count on reproducible results from toString.
		size_t length = this->filename.length();
		if (length > MIN_PATH_LENGTH && this->filename[length - 1] == Poco::Path::separator())
		{
			this->filename.resize(length - 1);
		}

		/**
		 * @tiapi(method=True,name=Filesystem.Logger.log,since=0.7) Writes data to the logfile
		 * @tiarg(for=Filesystem.Logger.log,type=String|Blob,name=data) data to log
		 */
		this->SetMethod("log",&Logger::Log);

		thread.start(*this);
	}

	Logger::~Logger()
	{
		bRunning = false;
		thread.join();
	}

	void Logger::Log(const ValueList& args, KValueRef result)
	{
		Poco::Mutex::ScopedLock lock(loggerMutex);
		std::string data = (char*)args.at(0)->ToString();
		writeQueue.push_back(data);
	}

	void Logger::Log()
	{
		Poco::Mutex::ScopedLock lock(loggerMutex);
		if (!writeQueue.empty())
		{
			std::ofstream stream;
			stream.open(filename.c_str(), std::ofstream::app);

			if (stream.is_open())
			{
				while (!writeQueue.empty())
				{
					stream.write(writeQueue.front().c_str(), writeQueue.front().size());
					writeQueue.pop_front();
				}
				stream.close();
			}
		}
	}

	void Logger::run()
	{
		bRunning = true;
		while(bRunning)
		{
			Log();
			Poco::Thread::sleep(1000);
		}
	}
}
