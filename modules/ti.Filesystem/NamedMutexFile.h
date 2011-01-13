/**
* Appcelerator Titanium - licensed under the Apache Public License 2
* see LICENSE in the root folder for details on the license.
* Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
*/
#ifndef _NAMEDMUTEXFILE_H_
#define _NAMEDMUTEXFILE_H_

#include <kroll/kroll.h>
#include <kroll/utils/NamedMutex.h>


namespace ti
{
	class NamedMutexFile
	{
	public:
		NamedMutexFile(const std::string &filename);
		virtual ~NamedMutexFile();
		void lock();
		bool tryLock();
		void unlock();

	protected:
		kroll::NamedMutex namedMutex;
	};
}

#endif