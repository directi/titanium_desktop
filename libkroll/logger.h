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

		static Logger* Get(const std::string &name);
		static Logger* GetRootLogger();
		static void Initialize(bool console, const std::string &logFilePath, Level level);
		static void Shutdown();
		static Level GetLevel(const std::string& level, bool debugEnabled = false);
		static void AddLoggerCallback(LoggerCallback callback);
		static std::string Format(const char*, va_list);

		Logger() {}
		Logger(const std::string &name);
		Logger(const std::string &name, Level level);
		virtual ~Logger() {}

		Level GetLevel() const { return this->level; }
		void SetLevel(Logger::Level level);
		std::string GetName() const { return this->name; }
		Logger* GetChild(const std::string &name);
		Logger* GetParent();
		
		bool IsEnabled(Level) const;
		bool IsTraceEnabled() const;
		bool IsDebugEnabled() const;
		bool IsInfoEnabled() const;
		bool IsNoticeEnabled() const;
		bool IsWarningEnabled() const;
		bool IsErrorEnabled() const;
		bool IsCriticalEnabled() const;
		bool IsFatalEnabled() const;

		virtual void Log(Poco::Message& m);
		void Log(Level, const std::string &);
		void Log(Level, const char*, va_list);
		void Log(Level, const char*, ...);

		void Trace(std::string);
		void Trace(const char*, ...);

		void Debug(std::string);
		void Debug(const char*, ...);

		void Info(std::string);
		void Info(const char*, ...);

		void Notice(std::string);
		void Notice(const char*, ...);

		void Warn(std::string);
		void Warn(const char*, ...);

		void Error(std::string);
		void Error(const char*, ...);

		void Critical(std::string);
		void Critical(const char*, ...);

		void Fatal(std::string);
		void Fatal(const char*, ...);

		protected:
		std::string name;
		Level level;
		static Poco::Mutex mutex;
		static char buffer[];

		static Logger* GetImpl(const std::string &name);
		static std::map<std::string, Logger*> loggers;
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

