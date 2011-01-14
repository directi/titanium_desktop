/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#include "NamedMutex.h"


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
	std::map<std::string, ReferenceCountedNamedMutex *> NamedMutex::namedMutexes;
	boost::mutex NamedMutex::filesMutex;

	NamedMutex::NamedMutex(const std::string &mutexname)
		: StaticBoundObject("Filesystem.NamedMutex"),
		mutexname(mutexname)
	{
		boost::mutex::scoped_lock lock(filesMutex);
		std::map<std::string, ReferenceCountedNamedMutex *>::iterator oIter = namedMutexes.find(mutexname);
		if(oIter == namedMutexes.end())
		{
			namedMutexes[mutexname] = new ReferenceCountedNamedMutex(new NamedMutexFile(mutexname));
		}
		namedMutexes[mutexname]->addRef();

		/**
		 * @tiapi(method=True,name=Filesystem.NamedMutex.Lock,since=1.1.0) locks the NamedMutex
		 */
		this->SetMethod("lock",&NamedMutex::Lock);
		/**
		 * @tiapi(method=True,name=Filesystem.NamedMutex.Lock,since=1.1.0) tries locking the NamedMutex
		 */
		this->SetMethod("tryLock",&NamedMutex::TryLock);
		/**
		 * @tiapi(method=True,name=Filesystem.NamedMutex.Lock,since=1.1.0) unlocks the NamedMutex
		 */
		this->SetMethod("unlock",&NamedMutex::Unlock);
	}

	NamedMutex::~NamedMutex()
	{
		boost::mutex::scoped_lock lock(filesMutex);
		namedMutexes[mutexname]->release();
		if(namedMutexes[mutexname]->getReferencesCount() == 0)
		{
			delete namedMutexes[mutexname]->mutexFile;
			namedMutexes[mutexname]->mutexFile = NULL;
			namedMutexes.erase(mutexname);
		}
	}

	void NamedMutex::Lock(const ValueList& args, KValueRef result)
	{
		namedMutexes[mutexname]->mutexFile->lock();
	}
	void NamedMutex::TryLock(const ValueList& args, KValueRef result)
	{
		bool ret = namedMutexes[mutexname]->mutexFile->tryLock();
		result->SetBool(ret);
	}
	void NamedMutex::Unlock(const ValueList& args, KValueRef result)
	{
		namedMutexes[mutexname]->mutexFile->unlock();
	}
}
