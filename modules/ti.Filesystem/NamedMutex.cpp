/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#include "NamedMutex.h"
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
	std::map<std::string, ReferenceCountedNamedMutex *> NamedMutex::files;
	Poco::Mutex NamedMutex::filesMutex;

	NamedMutex::NamedMutex(const std::string &filename)
		: StaticBoundObject("Filesystem.NamedMutex")
	{
		Poco::Path pocoPath(Poco::Path::expand(filename));
		fileName = pocoPath.absolute().toString();
		{
			Poco::Mutex::ScopedLock lock(filesMutex);
			std::map<std::string, ReferenceCountedNamedMutex *>::iterator oIter = files.find(fileName);
			if(oIter == files.end())
			{
				files[fileName] = new ReferenceCountedNamedMutex(new NamedMutexFile(fileName));
			}
		}
		files[fileName]->addRef();

		/**
		 * @tiapi(method=True,name=Filesystem.NamedMutex.log,since=0.7) Writes data to the logfile
		 * @tiarg(for=Filesystem.NamedMutex.log,type=String|Blob,name=data) data to log
		 */
		this->SetMethod("log",&NamedMutex::Log);
	}

	NamedMutex::~NamedMutex()
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

	void NamedMutex::Log(const ValueList& args, KValueRef result)
	{
		// TODO: create map of NamedMutexFile, select one of the required and call Log on that
		std::string data = (char*)args.at(0)->ToString();
//		files[fileName]->file->log(data);
	}
}
