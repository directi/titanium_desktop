#include "TiAsyncJobRunner.h"


namespace ti
{
	TiAsyncJobRunner::TiAsyncJobRunner()
		: jobMutex(),
		bRunning(false),
		thread(),
		waiting(false)
	{
		//thread.setName("TiAsyncJobRunner Thread");
		thread.start(this);
	}

	TiAsyncJobRunner::~TiAsyncJobRunner()
	{
		if (this->bRunning)
		{
			this->bRunning = false;
			this->notifyJobRunner();
		}
	}

	void TiAsyncJobRunner::enqueue(TiThreadTarget * job)
	{
		boost::mutex::scoped_lock lock(jobMutex);
		jobQueue.push_back(job);
		notifyJobRunner();
	}

	void TiAsyncJobRunner::doJobs()
	{
		std::list<TiThreadTarget *> *tempJobQueue = NULL;

		if(!jobQueue.empty())
		{
			boost::mutex::scoped_lock lock(jobMutex);
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
			this->wait();
		}
	}

	void TiAsyncJobRunner::notifyJobRunner()
	{
		boost::lock_guard<boost::mutex> lock(pendingJobLock);
		pendingJob.notify_one();
	}

	void TiAsyncJobRunner::wait()
	{
		boost::unique_lock<boost::mutex> lock(pendingJobLock);
		pendingJob.wait(lock);
	}

	TiAsyncJobRunnerSingleton * TiAsyncJobRunnerSingleton::singleton = NULL;
	boost::mutex TiAsyncJobRunnerSingleton::singletonMutex;
	TiAsyncJobRunner * TiAsyncJobRunnerSingleton::Instance()
	{
		boost::mutex::scoped_lock lock(singletonMutex);
		if(!singleton)
		{
			singleton = new TiAsyncJobRunnerSingleton();
		}
		return &(singleton->jobRunner);
	}

}
