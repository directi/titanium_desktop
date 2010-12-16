#ifndef _TI_ASYNC_JOB_RUNNER_H_
#define _TI_ASYNC_JOB_RUNNER_H_


#include <list>

#include <kroll/utils/Thread.h>
#include <boost/thread/condition_variable.hpp>

#include "TiThreadTarget.h"

namespace ti
{
	class TiAsyncJobRunner
		: public kroll::Runnable
	{
	public:

		TiAsyncJobRunner();
		virtual ~TiAsyncJobRunner();
		virtual void run();

		void enqueue(TiThreadTarget * job);

	private:
		boost::mutex jobMutex;
		std::list<TiThreadTarget *> jobQueue;
		bool bRunning;
		kroll::Thread thread;

		bool waiting;
		boost::condition_variable pendingJob;
		boost::mutex pendingJobLock;
		void doJobs();
		void notifyJobRunner();
		void wait();
	};

	class TiAsyncJobRunnerSingleton
	{
	public:
		TiAsyncJobRunner jobRunner;

		static TiAsyncJobRunnerSingleton * singleton;
		static boost::mutex singletonMutex;
		static TiAsyncJobRunner * Instance();

	private:

		TiAsyncJobRunnerSingleton() : jobRunner() {}
		virtual ~TiAsyncJobRunnerSingleton() {}
	};
}


#endif // _TI_ASYNC_JOB_RUNNER_H_
