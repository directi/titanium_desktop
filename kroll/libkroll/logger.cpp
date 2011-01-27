/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#include "logger.h"

#include <kroll/utils/file_utils.h>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/time_facet.hpp>


namespace kroll
{
	static std::string getCurrentTimeString()
	{
		std::ostringstream msg;
		const boost::posix_time::ptime now =
			boost::posix_time::microsec_clock::local_time();
		boost::posix_time::time_facet*const f=
			new boost::posix_time::time_facet("%d_%b_%Y_%H_%M_%S");
		msg.imbue(std::locale(msg.getloc(),f));
		msg << now;
		return msg.str();
	}

	static std::string getCurrentLogTimeString()
	{
		std::ostringstream msg;
		const boost::posix_time::ptime now =
			boost::posix_time::microsec_clock::local_time();
		boost::posix_time::time_facet*const f=
			new boost::posix_time::time_facet("%d-%b-%Y %H:%M:%s");
		msg.imbue(std::locale(msg.getloc(),f));
		msg << now;
		return msg.str();
	}

	static std::string formatMsg(const std::string& name, const std::string& message, Logger::Level level)
	{
		std::string formatted("[");
		formatted += getCurrentLogTimeString() + "] [";
		formatted += name + "] [";
		formatted += Logger::getStringForLevel(level) + "] ";;
		formatted += message;
		return formatted;
	}

	std::map<std::string, Logger*> Logger::loggers;
	char Logger::buffer[LOGGER_MAX_ENTRY_SIZE];
	boost::mutex Logger::mutex;

	std::string Logger::getStringForLevel(Level level)
	{
		switch(level)
		{
		case LFATAL:
			return "Fatal";
		case LCRITICAL:
			return "Critical";
		case LERROR:
			return "Error";
		case LWARN:
			return "Warning";
		case LNOTICE:
			return "Notice";
		case LINFO:
			return "Information";
		case LDEBUG:
			return "Debug";
		case LTRACE:
			return "Trace";
		};
		return "";
	}

	Logger* Logger::Get(const std::string &name)
	{
		if (loggers.find(name) == loggers.end())
		{
			std::string logger_name = std::string(PRODUCT_NAME) + "." + name;
			loggers[name] = new Logger(logger_name);
		}
		return loggers[name];
	}

	void Logger::Initialize(bool console, const std::string &logFilePath, Level level)
	{
		std::string path(logFilePath);
		// appending timestamp for creating a new log file each time we run our application.
		path += ".";
		path += getCurrentTimeString();

		RootLoggerConfig config(console, path, level);
		RootLogger::Initialize(config);
	}

	void Logger::Shutdown()
	{
		std::map<std::string, Logger*>::iterator i = loggers.begin();
		while (i != loggers.end())
		{
			Logger* l = (i++)->second;
			delete l;
		}
		loggers.clear();
		RootLogger::UnInitialize();
	}

#ifdef LOGGER_HAVE_CALLBACKS
	void Logger::AddLoggerCallback(LoggerCallback callback)
	{
		RootLogger::Instance()->AddLoggerCallback(callback);
	}

	void Logger::RemoveLoggerCallback(LoggerCallback callback)
	{
		RootLogger::Instance()->RemoveLoggerCallback(callback);
	}
#endif

	Logger::Logger(const std::string &name)
		: name(name),
		level(RootLogger::Instance()->getLevel())
	{ }

	Logger::Logger(const std::string &name, Level level)
		: name(name),
		level(level) { }

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

	void Logger::SetLevel(Logger::Level level)
	{
		this->Log(Logger::LWARN, "Logger Level changed from %s to %s",
			Logger::getStringForLevel(this->level).c_str(),
			Logger::getStringForLevel(level).c_str());
		this->level = level;
	}

	/*static*/
	std::string Logger::Format(const char* format, va_list args)
	{
		// Protect the buffer
		boost::mutex::scoped_lock lock(mutex);

		vsnprintf(Logger::buffer, LOGGER_MAX_ENTRY_SIZE - 1, format, args);
		Logger::buffer[LOGGER_MAX_ENTRY_SIZE - 1] = '\0';
		std::string text = buffer;
		return text;
	}


	void Logger::Log(Level level, const std::string& message)
	{
		if (IsEnabled(level))
		{
			RootLogger::Instance()->LogImpl(this->name, message, level);
		}
	}

	void Logger::Log(Level level, const char* format, va_list args)
	{
		if (IsEnabled(level))
		{
			this->Log(level, Logger::Format(format, args));
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

	RootLogger* RootLogger::instance = NULL;
	
	void RootLogger::Initialize(const RootLoggerConfig& config)
	{
		if(!RootLogger::instance)
		{
			RootLogger::instance = new RootLogger(config);
		}
	}
	void RootLogger::UnInitialize()
	{
		if(RootLogger::instance)
		{
			delete RootLogger::instance;
			RootLogger::instance = NULL;
		}
	}

	RootLogger::RootLogger(const RootLoggerConfig& config)
		: config(config)
	{
		RootLogger::instance = this;

		if (config.fileLogging)
		{
			// Before opening the logfile, ensure that a parent directory exists
			FileUtils::CreateDirectory(FileUtils::Dirname(config.path));
			stream.open(config.path.c_str(), std::ofstream::app);
		}
	}

	RootLogger::~RootLogger()
	{
		if (config.fileLogging)
		{
			stream.close();
		}
	}

	void RootLogger::LogImpl(const std::string& name, const std::string& message, Logger::Level level)
	{
		if (IsEnabled(level))
		{
			const std::string line = formatMsg(name, message, level);
			if (config.consoleLogging)
			{
				printf("%s\n", line.c_str());
				fflush(stdout);
			}

			if (config.fileLogging && stream.is_open())
			{
				stream << line << std::endl;
				stream.flush();
			}

#ifdef LOGGER_HAVE_CALLBACKS
			if(config.level >= Logger::LDEBUG)
			{
				ReadLock lock(mutex);
				for (size_t i = 0; i < callbacks.size(); i++)
				{
					callbacks[i](level, line);
				}
			}
#endif
		}
	}

#ifdef LOGGER_HAVE_CALLBACKS
	void RootLogger::AddLoggerCallback(Logger::LoggerCallback callback)
	{
		WriteLock lock(mutex);
		callbacks.push_back(callback);
	}

	void RootLogger::RemoveLoggerCallback(Logger::LoggerCallback callback)
	{
		WriteLock lock(mutex);
		for(std::vector<Logger::LoggerCallback>::iterator
			oIter = callbacks.begin();
			oIter != callbacks.end();
		oIter++)
		{
			if (*oIter == callback)
			{
				callbacks.erase(oIter);
				break;
			}
		}
	}
#endif
}
