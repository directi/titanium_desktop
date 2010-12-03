/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _LOGGER_H_
#define _LOGGER_H_

#include <base.h>
#include <cstdarg>
#include <fstream>

#include <boost/thread/mutex.hpp>

#define LOGGER_MAX_ENTRY_SIZE 2048

#define LOG_METHOD(METHOD,LEVEL) \
	void METHOD(const std::string &message) \
	{ \
		if (IsEnabled(LEVEL)) \
		{ \
			this->Log(LEVEL, message); \
		} \
	} \
	void METHOD(const char* format, ...) \
	{ \
		if (IsEnabled(LEVEL)) \
		{ \
			va_list args; \
			va_start(args, format); \
			this->Log(LEVEL, format, args); \
			va_end(args); \
		} \
	} \

namespace kroll
{
	class RootLogger;
	class KROLL_API Logger
	{
	public:
		typedef enum
		{
			LFATAL = 1,
			LCRITICAL,
			LERROR,
			LWARN,
			LNOTICE,
			LINFO,
			LDEBUG,
			LTRACE
		} Level;

		static std::string getStringForLevel(Level level);
		typedef void (*LoggerCallback)(Level, const std::string&);

	protected:
		Logger(const std::string &name);
		Logger(const std::string &name, Level level);
		~Logger() {}

		const std::string name;
		Level level;

		static boost::mutex mutex;
		static char buffer[LOGGER_MAX_ENTRY_SIZE];
		static std::map<std::string, Logger*> loggers;

	private:
		bool IsEnabled(Level) const	{ return level <= this->level; }

	public:
		static Logger* Get(const std::string &name);
		static void Initialize(bool console, const std::string &logFilePath, Level level);
		static void Shutdown();
		static void AddLoggerCallback(LoggerCallback callback);
		static void RemoveLoggerCallback(LoggerCallback callback);
		static Logger::Level GetLevel(const std::string& level, bool debugEnabled = false);
		static std::string Format(const char*, va_list);

		Level GetLevel() const { return this->level; }
		void SetLevel(Logger::Level level);
		std::string GetName() const { return this->name; }
		Logger* GetChild(const std::string &name);
		Logger* GetParent();
		void Log(Level, const std::string &);
		void Log(Level, const char*, va_list);
		void Log(Level, const char*, ...);

		LOG_METHOD(Trace, Logger::LTRACE)
		LOG_METHOD(Debug, Logger::LDEBUG)
		LOG_METHOD(Info, Logger::LINFO)
		LOG_METHOD(Notice, Logger::LNOTICE)
		LOG_METHOD(Warn, Logger::LWARN)
		LOG_METHOD(Error, Logger::LERROR)
		LOG_METHOD(Critical, Logger::LCRITICAL)
		LOG_METHOD(Fatal, Logger::LFATAL)
	};

	struct RootLoggerConfig
	{
		const bool consoleLogging;
		const bool fileLogging;
		const std::string& path;
		Logger::Level level;

		RootLoggerConfig(bool consoleLogging,
			const std::string& path = std::string(""),
			const Logger::Level level = Logger::LINFO)
			: consoleLogging(consoleLogging),
			fileLogging(!path.empty()),
			path(path),
			level(level)
		{
		}
	};

	class KROLL_API RootLogger
	{
	public:
		static void Initialize(const RootLoggerConfig& config);
		static void UnInitialize();
		static RootLogger * Instance() {return RootLogger::instance; }

		Logger::Level getLevel() const { return config.level; }
		void setLevel(Logger::Level level) { config.level = level; }
		void LogImpl(const std::string& name, const std::string& message, Logger::Level level);
		void AddLoggerCallback(Logger::LoggerCallback callback);
		void RemoveLoggerCallback(Logger::LoggerCallback callback);

	private:
		static RootLogger* instance;

		RootLogger(const RootLoggerConfig& config);
		~RootLogger();
		bool IsEnabled(Logger::Level level) const { return level <= config.level; }

		RootLoggerConfig config;
		std::ofstream stream;

		boost::mutex mutex;
		std::vector<Logger::LoggerCallback> callbacks;
	};
}

#endif // _LOGGER_H_