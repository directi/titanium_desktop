/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#include "Logger.h"
#include "XMLLoggerFile.h"

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
	std::map<std::string, ReferenceCountedLogger *> Logger::files;
	Poco::Mutex Logger::filesMutex;

	Logger::Logger(const std::string &filename)
		: StaticBoundObject("Filesystem.Logger")
	{
		Poco::Path pocoPath(Poco::Path::expand(filename));
		fileName = pocoPath.absolute().toString();
		{
			Poco::Mutex::ScopedLock lock(filesMutex);
			std::map<std::string, ReferenceCountedLogger *>::iterator oIter = files.find(fileName);
			if(oIter == files.end())
			{
				files[fileName] = new ReferenceCountedLogger(new kroll::LoggerFile(fileName));
			}
		}
		files[fileName]->addRef();

		/**
		 * @tiapi(method=True,name=Filesystem.Logger.log,since=0.7) Writes data to the logfile
		 * @tiarg(for=Filesystem.Logger.log,type=String|Blob,name=data) data to log
		 */
		this->SetMethod("log",&Logger::Log);
	}

	Logger::Logger(const std::string &filename, const std::string &rootXMLText, const std::string &xsltFile)
		: StaticBoundObject("Filesystem.Logger")
	{
		Poco::Path pocoPath(Poco::Path::expand(filename));
		fileName = pocoPath.absolute().toString();
		{
			Poco::Mutex::ScopedLock lock(filesMutex);
			std::map<std::string, ReferenceCountedLogger *>::iterator oIter = files.find(fileName);
			if(oIter == files.end())
			{
				files[fileName] = new ReferenceCountedLogger(new XMLLoggerFile(fileName, rootXMLText, xsltFile));
			}
		}
		files[fileName]->addRef();

		/**
		 * @tiapi(method=True,name=Filesystem.Logger.log,since=0.7) Writes data to the logfile
		 * @tiarg(for=Filesystem.Logger.log,type=String|Blob,name=data) data to log
		 */
		this->SetMethod("log",&Logger::Log);
	}

	Logger::~Logger()
	{
		Poco::Mutex::ScopedLock lock(filesMutex);
		files[fileName]->release();
		if(files[fileName]->getReferencesCount() == 0)
		{
			delete files[fileName]->file;
			files[fileName]->file = NULL;
			files.erase(fileName);
		}
	}

	void Logger::Log(const ValueList& args, KValueRef result)
	{
		// TODO: create map of LoggerFile, select one of the required and call Log on that
		std::string data = (char*)args.at(0)->ToString();
		files[fileName]->file->log(data);
	}
}
