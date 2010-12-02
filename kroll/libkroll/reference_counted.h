/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _KR_REFERENCE_COUNTED_H_
#define _KR_REFERENCE_COUNTED_H_

#include <base.h>
#include <boost/thread/mutex.hpp>

namespace kroll
{
	class KROLL_API ReferenceCounted
	{
	private:
		int count;
		boost::mutex count_mutex;

	public:
		ReferenceCounted() : count(1) { }
		virtual ~ReferenceCounted() { }

		void duplicate()
		{
			boost::mutex::scoped_lock lock(count_mutex);
			++count;
		}

		void release()
		{
			{
				boost::mutex::scoped_lock lock(count_mutex);
				--count;
			}
			if (count == 0)
			{
				delete this;
			}
		}

		int referenceCount() const
		{
			return count;
		}
	};
}
#endif
