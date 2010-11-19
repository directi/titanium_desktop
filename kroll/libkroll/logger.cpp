/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#include <Poco/ScopedLock.h>
#include <Poco/PatternFormatter.h>
#include <Poco/Path.h>
#include <Poco/File.h>

#include <cstdio>
#include <sstream>
#include <cstring>

#include "logger.h"

#include <kroll/utils/file_utils.h>

#define LOGGER_MAX_ENTRY_SIZE 2048

using Poco::PatternFormatter;
using Poco::Path;
using Poco::File;

namespace kroll
{
	std::map<std::string, Logger*> Logger::loggers;
	char Logger::buffer[LOGGER_MAX_ENTRY_SIZE];
	Poco::Mutex Logger::mutex;

	/*static*/
	Logger* Logger::Get(const std::string &name)
	{
		std::string logger_name = std::string(PRODUCT_NAME) + "." + name;
		return Logger::GetImpl(logger_name);
	}

	/*static*/
	void Logger::Initialize(bool console, const std::string &logFilePath, Level level)
	{
		Logger::loggers[PRODUCT_NAME] = 
			new RootLogger(console, logFilePath, level);
	}

	/*static*/
	void Logger::Shutdown()
	{
		std::map<std::string, Logger*>::iterator i = loggers.begin();
		while (i != loggers.end())
		{
			Logger* l = (i++)->second;
			delete l;
		}
		loggers.clear();
	}

	/*static*/
	Logger* Logger::GetRootLogger()
	{
		return RootLogger::instance;
	}

	/*static*/
	void Logger::AddLoggerCallback(LoggerCallback callback)
	{
		RootLogger* rootLogger = 
			reinterpret_cast<RootLogger*>(GetRootLogger());
		rootLogger->AddLoggerCallback(callback);
	}

	/*static*/
	Logger* Logger::GetImpl(const std::string &name)
	{
		if (loggers.find(name) == loggers.end())
		{
			loggers[name] = new Logger(name);
		}
		return loggers[name];
	}

	static std::string getCurrentTimeString()
	{
		time_t time_of_day;
		char buffer[ 80 ];
		time_of_day = time( NULL );
		strftime( buffer, 80, "%d_%B_%Y_%H_%M_%S", localtime( &time_of_day ) );
		printf( "%s\n", buffer );
		std::string str(buffer);
		return str;
	}


	Logger::Level Logger::GetLevel(const std::string& level, bool debugEnabled)
	{
		if (level == "TRACE")
			return Logger::LTRACE;
		else if (level == "DEBUG")
			return Logger::LDEBUG;
		else if (level == "INFO")
			return Logger::LINFO;
		else if (level == "NOTICE")
			return Logger::LNOTICE;
		else if (level == "WARN")
			return Logger::LWARN;
		else if (level == "ERROR")
			return Logger::LERROR;
		else if (level == "CRITICAL")
			return Logger::LCRITICAL;
		else if (level == "FATAL")
			return Logger::LFATAL;
		else if (debugEnabled)
			return Logger::LDEBUG;
		return Logger::LINFO;
	}

	Logger::Logger(const std::string &name) :
		name(name)
	{
		Logger* parent = this->GetParent();
		this->level = parent->GetLevel();
	}

	Logger::Logger(const std::string &name, Level level) :
		name(name),
		level(level)
	{ }

	void Logger::SetLevel(Logger::Level level)
	{
		this->level = level;
	}

	Logger* Logger::GetChild(const std::string &name)
	{
		std::string childName = this->name + "." + name;
		return Logger::GetImpl(childName);
	}

	Logger* Logger::GetParent()
	{
		size_t lastPeriodPos = this->name.rfind(".");
		if (lastPeriodPos == std::string::npos)
		{
			// in some cases this causes an infinite loop
			if (RootLogger::instance == NULL)
			{
				return NULL;
			}
			
			return Logger::GetRootLogger();
		}
		else
		{
			std::string parentName = this->name.substr(0, lastPeriodPos);
			return Logger::GetImpl(parentName);
		}
	}

	void Logger::Log(Poco::Message& m)
	{
		if (IsEnabled(level))
		{
			RootLogger* root = RootLogger::instance;
			root->LogImpl(m);
		}
	}
	/*static*/
	std::string Logger::Format(const char* format, va_list args)
	{
		// Protect the buffer
		Poco::Mutex::ScopedLock lock(mutex);

		vsnprintf(Logger::buffer, LOGGER_MAX_ENTRY_SIZE - 1, format, args);
		Logger::buffer[LOGGER_MAX_ENTRY_SIZE - 1] = '\0';
		std::string text = buffer;
		return text;
	}


	void Logger::Log(Level level, const std::string& message)
	{
		if (IsEnabled(level))
		{
			Poco::Message m(this->name, message, (Poco::Message::Priority) level);
			this->Log(m);
		}
	}

	void Logger::Log(Level level, const char* format, va_list args)
	{
		if (IsEnabled(level))
		{
			std::string messageText = Logger::Format(format, args);
			this->Log(level, messageText);
		}
	}

	void Logger::Log(Level level, const char* format, ...)
	{
		if (IsEnabled(level))
		{
			va_list args;
			va_start(args, format);
			this->Log(level, format, args);
			va_end(args);
		}
	}

	void Logger::Trace(const std::string &message)
	{
		if (IsTraceEnabled())
		{
			this->Log(LTRACE, message);
		}
	}

	void Logger::Trace(const char* format, ...)
	{
		if (IsTraceEnabled())
		{
			va_list args;
			va_start(args, format);
			this->Log(LTRACE, format, args);
			va_end(args);
		}
	}

	void Logger::Debug(const std::string &message)
	{
		if (IsDebugEnabled())
		{
			this->Log(LDEBUG, message);
		}
	}

	void Logger::Debug(const char* format, ...)
	{
		if (IsDebugEnabled())
		{
			va_list args;
			va_start(args, format);
			this->Log(LDEBUG, format, args);
			va_end(args);
		}
	}

	void Logger::Info(const std::string &message)
	{
		if (IsInfoEnabled())
		{
			this->Log(LINFO, message);
		}
	}

	void Logger::Info(const char* format, ...)
	{
		if (IsInfoEnabled())
		{
			va_list args;
			va_start(args, format);
			this->Log(LINFO, format, args);
			va_end(args);
		}
	}

	void Logger::Notice(const std::string &message)
	{
		if (IsNoticeEnabled())
		{
			this->Log(LNOTICE, message);
		}
	}

	void Logger::Notice(const char* format, ...)
	{
		if (IsNoticeEnabled())
		{
			va_list args;
			va_start(args, format);
			this->Log(LNOTICE, format, args);
			va_end(args);
		}
	}

	void Logger::Warn(const std::string &message)
	{
		if (IsWarningEnabled())
		{
			this->Log(LWARN, message);
		}
	}

	void Logger::Warn(const char* format, ...)
	{
		if (IsWarningEnabled())
		{
			va_list args;
			va_start(args, format);
			this->Log(LWARN, format, args);
			va_end(args);
		}
	}

	void Logger::Error(const std::string &message)
	{
		if (IsErrorEnabled())
		{
			this->Log(LERROR, message);
		}
	}

	void Logger::Error(const char* format, ...)
	{
		if (IsErrorEnabled())
		{
			va_list args;
			va_start(args, format);
			this->Log(LERROR, format, args);
			va_end(args);
		}
	}

	void Logger::Critical(const std::string &message)
	{
		if (IsCriticalEnabled())
		{
			this->Log(LCRITICAL, message);
		}
	}

	void Logger::Critical(const char* format, ...)
	{
		if (IsCriticalEnabled())
		{
			va_list args;
			va_start(args, format);
			this->Log(LCRITICAL, format, args);
			va_end(args);
		}
	}

	void Logger::Fatal(const std::string &message)
	{
		if (IsFatalEnabled())
		{
			this->Log(LFATAL, message);
		}
	}

	void Logger::Fatal(const char* format, ...)
	{
		if (IsFatalEnabled())
		{
			va_list args;
			va_start(args, format);
			this->Log(LFATAL, format, args);
			va_end(args);
		}
	}

	RootLogger* RootLogger::instance = NULL;
	RootLogger::RootLogger(bool consoleLogging, const std::string &logFilePath, Level level) :
		Logger(PRODUCT_NAME, level),
		consoleLogging(consoleLogging),
		fileLogging(!logFilePath.empty())
	{
		RootLogger::instance = this;
		this->formatter = new PatternFormatter("[%H:%M:%S:%i] [%s] [%p] %t");

		if (fileLogging)
		{
			std::string file_path(logFilePath);
			// Before opening the logfile, ensure that a parent directory exists
			std::string logDirectory = FileUtils::Dirname(file_path);
			File logDirectoryFile = File(logDirectory);
			logDirectoryFile.createDirectories();
			{
				// appending timestamp for creating a new log file each time we run our application.
				file_path += ".";
				file_path += getCurrentTimeString();
			}
			stream.open(file_path.c_str(), std::ofstream::app);
		}
	}

	RootLogger::~RootLogger()
	{
		if (fileLogging)
		{
			stream.close();
		}
	}

	void RootLogger::LogImpl(Poco::Message& m)
	{
		Poco::Mutex::ScopedLock lock(mutex);
		Level level = (Level) m.getPriority();
		std::string line;
		this->formatter->format(m, line);

		if (fileLogging)
		{
			if (stream.is_open())
			{
				stream << line << std::endl;
				stream.flush();
			}
		}

		if (consoleLogging)
		{
			printf("%s\n", line.c_str());
			fflush(stdout);
		}

		for (size_t i = 0; i < callbacks.size(); i++)
		{
			callbacks[i](level, line);
		}
	}

	void RootLogger::AddLoggerCallback(LoggerCallback callback)
	{
		Poco::Mutex::ScopedLock lock(mutex);
		callbacks.push_back(callback);
	}
}
