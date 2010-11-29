/**
* Appcelerator Titanium - licensed under the Apache Public License 2
* see LICENSE in the root folder for details on the license.
* Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
*/
#include <fstream>
#include <string>
#include <iostream>

#include <kroll/kroll.h>

#include "NamedMutexFile.h"

#ifdef OS_WIN32
#define MIN_PATH_LENGTH 3
#else
#define MIN_PATH_LENGTH 1
#endif


namespace ti
{
	NamedMutexFile::NamedMutexFile(const std::string &mutex_name)
		: namedMutex(mutex_name)
	{
	}

	NamedMutexFile::~NamedMutexFile()
	{
		unlock();
	}

	void NamedMutexFile::lock()
	{
		namedMutex.lock();
	}

	bool NamedMutexFile::tryLock()
	{
		return namedMutex.tryLock();;
	}

	void NamedMutexFile::unlock()
	{
		namedMutex.unlock();
	}
}
