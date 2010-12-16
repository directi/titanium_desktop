#ifndef _THREAD_WRAPPER_H_
#define _THREAD_WRAPPER_H_

#include <base.h>

#include <boost/function.hpp>
#include <boost/thread/thread.hpp>

namespace kroll
{
	class KROLL_API Runnable
	{
	public:
		Runnable();
		virtual ~Runnable();
		virtual void run()=0;
	};

	class KROLL_API Thread
	{
		std::auto_ptr<boost::thread> t;
	public:
		Thread();
		~Thread();
		void start(Runnable * runnable);
		void start(boost::function<void()> func);
		inline void join();
	};
}


#endif // _THREAD_WRAPPER_H_
