/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#include <Poco/Logger.h>
#include <Poco/Message.h>
#include <Poco/Mutex.h>
#include <Poco/PatternFormatter.h>
#include <cstdarg>
#include <iostream>
#include <fstream>

#include "LoggerFile.h"

namespace kroll
{
	class RootLogger;
	class KROLL_API Logger
	{
	public:
		typedef enum
		{
			LFATAL = Poco::Message::PRIO_FATAL,
			LCRITICAL = Poco::Message::PRIO_CRITICAL,
			LERROR = Poco::Message::PRIO_ERROR,
			LWARN = Poco::Message::PRIO_WARNING,
			LNOTICE = Poco::Message::PRIO_NOTICE,
			LINFO = Poco::Message::PRIO_INFORMATION,
			LDEBUG = Poco::Message::PRIO_DEBUG,
			LTRACE = Poco::Message::PRIO_TRACE
		} Level;
		typedef void (*LoggerCallback)(Level, std::string&);

	protected:
		Logger() {}
		Logger(const std::string &name);
		Logger(const std::string &name, Level level);
		virtual ~Logger() {}

		std::string name;
		Level level;
		static Poco::Mutex mutex;
		static char buffer[];

		static Logger* GetImpl(const std::string &name);
		static std::map<std::string, Logger*> loggers;

	private:
		bool IsEnabled(Level) const	{ return level <= this->level; }
		bool IsTraceEnabled() const { return this->IsEnabled(LTRACE); }
		bool IsDebugEnabled() const { return this->IsEnabled(LDEBUG); }
		bool IsInfoEnabled() const { return this->IsEnabled(LINFO); }
		bool IsNoticeEnabled() const { return this->IsEnabled(LNOTICE); }
		bool IsWarningEnabled() const { return this->IsEnabled(LWARN); }
		bool IsErrorEnabled() const { return this->IsEnabled(LERROR); }
		bool IsCriticalEnabled() const { return this->IsEnabled(LCRITICAL); }
		bool IsFatalEnabled() const { return this->IsEnabled(LFATAL); }

		virtual void Log(Poco::Message& m);

	public:
		static Logger* Get(const std::string &name);
		static Logger* GetRootLogger();
		static void Initialize(bool console, const std::string &logFilePath, Level level);
		static void Shutdown();
		static Level GetLevel(const std::string& level, bool debugEnabled = false);
		static void AddLoggerCallback(LoggerCallback callback);
		static std::string Format(const char*, va_list);

		Level GetLevel() const { return this->level; }
		void SetLevel(Logger::Level level);
		std::string GetName() const { return this->name; }
		Logger* GetChild(const std::string &name);
		Logger* GetParent();
		void Log(Level, const std::string &);
		void Log(Level, const char*, va_list);
		void Log(Level, const char*, ...);

		void Trace(const std::string&);
		void Trace(const char*, ...);

		void Debug(const std::string& );
		void Debug(const char*, ...);

		void Info(const std::string&);
		void Info(const char*, ...);

		void Notice(const std::string&);
		void Notice(const char*, ...);

		void Warn(const std::string&);
		void Warn(const char*, ...);

		void Error(const std::string&);
		void Error(const char*, ...);

		void Critical(const std::string&);
		void Critical(const char*, ...);

		void Fatal(const std::string&);
		void Fatal(const char*, ...);
	};

	class KROLL_API RootLogger : public Logger
	{
	public:
		RootLogger(bool consoleLogging, const std::string &logFilePath, Level level);
		~RootLogger();
		static RootLogger* instance;
		virtual void LogImpl(Poco::Message& m);
		void AddLoggerCallback(LoggerCallback callback);

	protected:
		const bool consoleLogging;
		const bool fileLogging;

		Poco::PatternFormatter* formatter;
		Poco::Mutex mutex;
		std::vector<LoggerCallback> callbacks;
		kroll::LoggerFile * logFile;
	};

}

