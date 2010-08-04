/**
* Appcelerator Titanium - licensed under the Apache Public License 2
* see LICENSE in the root folder for details on the license.
* Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
*/
#include <fstream>
#include <Poco/Path.h>

#include "NamedMutexFile.h"

#ifdef OS_WIN32
#define MIN_PATH_LENGTH 3
#else
#define MIN_PATH_LENGTH 1
#endif


namespace ti
{
	NamedMutexFile::NamedMutexFile(const std::string &filename)
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
		namedMutex = new Poco::NamedMutex(this->filename.c_str());
	}

	NamedMutexFile::~NamedMutexFile()
	{
	}

	void NamedMutexFile::lock()
	{
	}

	bool NamedMutexFile::tryLock()
	{
		return false;
	}

	void NamedMutexFile::unlock()
	{
	}
}
