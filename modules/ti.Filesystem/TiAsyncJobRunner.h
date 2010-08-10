#ifndef _TI_ASYNC_JOB_RUNNER_H_
#define _TI_ASYNC_JOB_RUNNER_H_


#include <list>
#include <string>

#include <Poco/Event.h>
#include <Poco/Mutex.h>
#include <Poco/Thread.h>

#include "TiThreadTarget.h"

namespace ti {


	class TiAsyncJobRunner
		: public Poco::Runnable
	{
	public:

		TiAsyncJobRunner();
		~TiAsyncJobRunner();

		void enqueue(TiThreadTarget * job);

	private:

		Poco::Mutex jobMutex;
		std::list<TiThreadTarget *> jobQueue;
		bool bRunning;
		Poco::Thread thread;
		Poco::Event pendingJobEvent;

		virtual void run();
		void doJobs();
	};

	class TiAsyncJobRunnerSingleton
	{
	public:
		TiAsyncJobRunner jobRunner;

		static TiAsyncJobRunnerSingleton * singleton;
		static Poco::Mutex singletonMutex;
		static TiAsyncJobRunner * Instance();

	private:

		TiAsyncJobRunnerSingleton() : jobRunner() {}
		~TiAsyncJobRunnerSingleton() {}
	};
}


#endif // _TI_ASYNC_JOB_RUNNER_H_
