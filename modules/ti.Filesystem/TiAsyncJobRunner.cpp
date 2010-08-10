#include "TiAsyncJobRunner.h"


namespace ti
{
	TiAsyncJobRunner::TiAsyncJobRunner()
		: jobMutex(),
		bRunning(false),
		thread(),
		pendingJobEvent(true)
	{
		thread.setName("TiAsyncJobRunner Thread");
		thread.start(*this);
	}

	TiAsyncJobRunner::~TiAsyncJobRunner()
	{
		if(thread.isRunning())
		{
			this->bRunning = false;
			this->pendingJobEvent.set();
		}
	}

	void TiAsyncJobRunner::enqueue(TiThreadTarget * job)
	{
		Poco::Mutex::ScopedLock lock(jobMutex);
		jobQueue.push_back(job);
		pendingJobEvent.set();
	}

	void TiAsyncJobRunner::doJobs()
	{
		std::list<TiThreadTarget *> *tempJobQueue = NULL;

		if(!jobQueue.empty())
		{
			Poco::Mutex::ScopedLock lock(jobMutex);
			tempJobQueue = new std::list<TiThreadTarget *>(jobQueue.size());
			std::copy(jobQueue.begin(), jobQueue.end(), tempJobQueue->begin()); 
			jobQueue.clear();
		}
		if (tempJobQueue)
		{
			while (!tempJobQueue->empty())
			{
				tempJobQueue->front()->run();
				delete tempJobQueue->front();
				tempJobQueue->pop_front();
			}
		}
	}

	void TiAsyncJobRunner::run()
	{
		bRunning = true;
		while(bRunning)
		{
			doJobs();
			pendingJobEvent.wait();
		}

	}

	TiAsyncJobRunnerSingleton * TiAsyncJobRunnerSingleton::singleton = NULL;
	Poco::Mutex TiAsyncJobRunnerSingleton::singletonMutex;
	TiAsyncJobRunner * TiAsyncJobRunnerSingleton::Instance()
	{
		Poco::Mutex::ScopedLock lock(singletonMutex);
		if(!singleton)
		{
			singleton = new TiAsyncJobRunnerSingleton();
		}
		return &(singleton->jobRunner);
	}

}
