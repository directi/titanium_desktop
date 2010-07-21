/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#include "kroll.h"
#include <fstream>
#include <Poco/Path.h>

#ifdef OS_WIN32
#define MIN_PATH_LENGTH 3
#else
#define MIN_PATH_LENGTH 1
#endif


namespace kroll
{
	LoggerWriter * LoggerWriter::singleton = NULL;
	Poco::Mutex LoggerWriter::singletonAccessMutex;

	LoggerWriter* LoggerWriter::getInstance()
	{
		Poco::Mutex::ScopedLock lock(singletonAccessMutex);
		if(!singleton)
		{
			singleton = new LoggerWriter();
		}
		return singleton;
	}

	LoggerWriter::LoggerWriter()
		: filesMutex(),
		bRunning(false),
		thread(),
		pendingMsgEvent(true)
	{
		thread.setName("LoggerWriter Thread");
	}

	LoggerWriter::~LoggerWriter()
	{
		this->stop();
	}

	void LoggerWriter::addLoggerFile(LoggerFile * file)
	{
		Poco::Mutex::ScopedLock lock(filesMutex);
		if (file)
		{
			this->files.push_back(file);
		}

		if (this->files.size() > 0)
		{
			this->start();
		}
	}

	void LoggerWriter::removeLoggerFile(LoggerFile * file)
	{
		Poco::Mutex::ScopedLock lock(filesMutex);
		for(std::vector<LoggerFile *>::iterator
			oIter = files.begin();
			oIter != files.end();
		oIter++)
		{
			if((*oIter) == file)
			{
				files.erase(oIter);
				break;
			}
		}

		if (this->files.size() == 0)
		{
			this->stop();
		}
	}

	void LoggerWriter::start()
	{
		if(!thread.isRunning())
		{
			thread.start(*this);
		}
	}

	void LoggerWriter::stop()
	{
		if(thread.isRunning())
		{
			this->bRunning = false;
			this->pendingMsgEvent.set();
		}
	}

	void LoggerWriter::notify(LoggerFile * file)
	{
		this->pendingMsgEvent.set();
	}

	void LoggerWriter::run()
	{
		bRunning = true;
		while(bRunning)
		{
			{
				Poco::Mutex::ScopedLock lock(filesMutex);
				for(std::vector<LoggerFile *>::iterator	
					oIter = files.begin();
					oIter != files.end();
				oIter++)
				{
					(*oIter)->dumpToFile();
				}
			}
			pendingMsgEvent.wait();
		}
	}
}
