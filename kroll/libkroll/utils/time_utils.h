/**
 * @author: Mital Vora <mital.d.vora@gmail.com>
 */
#ifndef _TIME_UTILS_H_
#define _TIME_UTILS_H_

#include <base.h>

namespace UTILS_NS
{
	namespace TimeUtils
	{
		KROLL_API double getCurrentTimeInMiliSeconds();
		KROLL_API double getCurrentTimeInSeconds();

		class KROLL_API StopWatch
		{
			double start_time;
			double end_time;
		public:
			StopWatch(bool start = false);
			~StopWatch();
			void start();
			void stop();
			double elapsed() const;
		};
	}
}
#endif // _TIME_UTILS_H_