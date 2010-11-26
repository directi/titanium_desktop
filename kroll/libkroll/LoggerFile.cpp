/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#include <Poco/Path.h>

#include "LoggerFile.h"

#ifdef OS_WIN32
#define MIN_PATH_LENGTH 3
#else
#define MIN_PATH_LENGTH 1
#endif


namespace kroll
{
	LoggerFile::LoggerFile(const std::string &filename)
		//: filename(absolutePath(filename))
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

		LoggerWriter::getInstance()->addLoggerFile(this);
		stream.open(this->filename.c_str(), std::ofstream::app);
	}

	LoggerFile::~LoggerFile()
	{
		LoggerWriter::getInstance()->removeLoggerFile(this);
		stream.close();
	}

	void LoggerFile::log(std::string& data)
	{
		boost::mutex::scoped_lock lock(loggerMutex);
		writeQueue.push_back(data);
		LoggerWriter::getInstance()->notify(this);
	}

	void LoggerFile::dumpToFile()
	{
		std::list<std::string> *tempWriteQueue = NULL;

		if(!writeQueue.empty())
		{
			boost::mutex::scoped_lock lock(loggerMutex);
			tempWriteQueue = new std::list<std::string>(writeQueue.size());
			std::copy(writeQueue.begin(), writeQueue.end(), tempWriteQueue->begin()); 
			writeQueue.clear();
		}
		if (tempWriteQueue)
		{
			try
			{
				if (stream.is_open())
				{
					while (!tempWriteQueue->empty())
					{
						stream.write(tempWriteQueue->front().c_str(), tempWriteQueue->front().size());
						tempWriteQueue->pop_front();
					}
					delete tempWriteQueue;
					tempWriteQueue = NULL;
					stream.flush();
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
}
