/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#include "LoggerFile.h"

#include <Poco/Path.h>

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
	LoggerFile::LoggerFile(const std::string &filename) :
		references(0),
		bRunning(false),
		pendingMsgEvent(true)
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

		thread.start(*this);
	}

	LoggerFile::~LoggerFile()
	{
		bRunning = false;
		pendingMsgEvent.set();
		thread.join();
	}

	void LoggerFile::log(std::string& data)
	{
		Poco::Mutex::ScopedLock lock(loggerMutex);
		writeQueue.push_back(data);
		pendingMsgEvent.set();
	}

	void LoggerFile::dumpToFile()
	{
		std::list<std::string> *tempWriteQueue = NULL;

		if(!writeQueue.empty())
		{
			Poco::Mutex::ScopedLock lock(loggerMutex);
			tempWriteQueue = new std::list<std::string>(writeQueue.size());
			std::copy(writeQueue.begin(), writeQueue.end(), tempWriteQueue->begin()); 
			writeQueue.clear();
		}
		if (tempWriteQueue)
		{
			try
			{
				std::ofstream stream;
				stream.open(filename.c_str(), std::ofstream::app);

				if (stream.is_open())
				{
					while (!tempWriteQueue->empty())
					{
						stream.write(tempWriteQueue->front().c_str(), tempWriteQueue->front().size());
						tempWriteQueue->pop_front();
					}
					stream.close();
					delete tempWriteQueue;
					tempWriteQueue = NULL;
				}
			}
			catch (...)
			{
				if(tempWriteQueue != NULL)
					delete tempWriteQueue;
				throw;
			}
		}
	}

	void LoggerFile::run()
	{
		bRunning=true;
		while(bRunning)
		{
			pendingMsgEvent.wait();
			dumpToFile();
		}
	}
}
