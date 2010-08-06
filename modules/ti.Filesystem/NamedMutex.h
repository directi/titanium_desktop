/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _TI_NAMEDMUTEX_H_
#define _TI_NAMEDMUTEX_H_

#include <map>
#include <kroll/kroll.h>

#include "NamedMutexFile.h"

namespace ti
{
	class ReferenceCountedNamedMutex
	{
		public:
			Poco::Mutex referencesMutex;
			int references;
			NamedMutexFile * mutexFile;

			ReferenceCountedNamedMutex(NamedMutexFile *_mutexFile = NULL)
				: references(0), mutexFile(_mutexFile) { }

			void addRef()
			{
				Poco::Mutex::ScopedLock lock(referencesMutex);
				references++;
			}

			void release()
			{
				Poco::Mutex::ScopedLock lock(referencesMutex);
				--references;
			}

			int getReferencesCount()
			{
				Poco::Mutex::ScopedLock lock(referencesMutex);
				return references;
			}
	};

	class NamedMutex
		: public StaticBoundObject
	{
		private:
			std::string mutexname;

			static std::map<std::string, ReferenceCountedNamedMutex *> namedMutexes;
			static Poco::Mutex filesMutex;

		public:
			NamedMutex(const std::string &mutexname);
			virtual ~NamedMutex();
			
			void Lock(const ValueList& args, KValueRef result);
			void TryLock(const ValueList& args, KValueRef result);
			void Unlock(const ValueList& args, KValueRef result);
	};
}

#endif
