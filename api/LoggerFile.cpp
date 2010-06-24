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
	LoggerWriter::LoggerWriter()
		: bRunning(false),
		thread(),
		pendingMsgEvent(true)
	{
		thread.setName("LoggerWriter Thread");
	}

	void LoggerWriter::addLoggerFile(LoggerFile * file)
	{
		//Unsafe
		if(!singleton)
		{
			singleton = new LoggerWriter();
		}
		singleton->addFile(file);
	}

	void LoggerWriter::removeLoggerFile(LoggerFile * file)
	{
		//Unsafe
		if(singleton)
		{
			singleton->removeFile(file);
		}
	}

	void LoggerWriter::addFile(LoggerFile * file)
	{
		if(file)
		{
			this->files.push_back(file);
		}
		this->start();
	}

	void LoggerWriter::removeFile(LoggerFile * file)
	{
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
			this->thread.join();
		}
	}

	void LoggerWriter::notify(LoggerFile * file)
	{
		if(singleton)
		{
			singleton->notify();
		}
	}


	void LoggerWriter::notify()
	{
		this->pendingMsgEvent.set();
	}

	void LoggerWriter::run()
	{
		bRunning = true;
		while(bRunning)
		{
			pendingMsgEvent.wait();
			for(std::vector<LoggerFile *>::iterator
				oIter = files.begin();
				oIter != files.end();
			oIter++)
			{
				(*oIter)->dumpToFile();
			}
		}
	}

	LoggerFile::LoggerFile(const std::string &filename)
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

		LoggerWriter::addLoggerFile(this);
	}

	LoggerFile::~LoggerFile()
	{
		LoggerWriter::removeLoggerFile(this);
	}

	void LoggerFile::log(std::string& data)
	{
		Poco::Mutex::ScopedLock lock(loggerMutex);
		writeQueue.push_back(data);
		LoggerWriter::notify(this);
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
}
