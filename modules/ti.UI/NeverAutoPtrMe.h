#ifndef NEVER_AUTO_PTR_ME_INCLUDED
#define NEVER_AUTO_PTR_ME_INCLUDED

#include "reference_counted.h"

namespace kroll
{
	// Use this for stuff like UI components which share the same lifecycle as the js context.
	class NeverAutoPtrMe 
	{
	public:
		NeverAutoPtrMe(ReferenceCounted* ref)
		{
			ref->setprotect();
		}
	};
}
#endif
