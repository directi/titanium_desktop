/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _TI_LOGGER_H_
#define _TI_LOGGER_H_

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
			NamedMutexFile * file;

			ReferenceCountedNamedMutex(NamedMutexFile *_file = NULL)
				: references(0), file(_file) { }

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
			std::string fileName;

			static std::map<std::string, ReferenceCountedNamedMutex *> files;
			static Poco::Mutex filesMutex;

		public:
			NamedMutex(const std::string &filename);
			virtual ~NamedMutex();
			
			void Log(const ValueList& args, KValueRef result);
	};
}

#endif
