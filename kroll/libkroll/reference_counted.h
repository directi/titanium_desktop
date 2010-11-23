/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _KR_REFERENCE_COUNTED_H_
#define _KR_REFERENCE_COUNTED_H_

#include <Poco/AtomicCounter.h>
#include "base.h"

namespace kroll
{
	class KROLL_API ReferenceCounted
	{
		private:
		Poco::AtomicCounter count;

		public:
		ReferenceCounted() : count(1) { }
		virtual ~ReferenceCounted() { }

		void duplicate()
		{
			++count;
		}

		void release()
		{
			int value = --count;
			if (value <= 0) 
				delete this;
		}

		int referenceCount() const
		{
			return count.value();
		}
	};
}
#endif
