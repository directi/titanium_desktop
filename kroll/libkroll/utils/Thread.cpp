#include "Thread.h"


namespace kroll
{
	Runnable::Runnable()
	{
	}
	
	Runnable::~Runnable()
	{
	}

	Thread::Thread()
	{
	}

	Thread::~Thread()
	{
	}

	void Thread::start(Runnable * runnable)
	{
		t.reset(new boost::thread(boost::bind(&Runnable::run, runnable)));
	}

	void Thread::start(boost::function<void()> func)
	{
		t.reset(new boost::thread(func));
	}

	void Thread::join()
	{
		if (t.get())
		{
			t->join();
		}
	}
}
