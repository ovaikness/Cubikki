#include "Engine/Threads/JobWorkerThread.hpp"
#include "Engine/Threads/JobSystem.hpp"

namespace PBE
{
	JobWorkerThread::JobWorkerThread(JobSystem* owner, int id)
		: m_owningJobSystem(owner)
		, m_id(id)
	{

	}

	JobWorkerThread::~JobWorkerThread()
	{

	}

	void JobWorkerThread::Start()
	{
		m_running = true;
		m_thread = std::thread(&JobWorkerThread::ThreadMain, this);
	}

	void JobWorkerThread::Stop()
	{
		m_running = false;
		// Wake up the thread so it can exit
		m_condition.notify_one();
		m_thread.join();
	}

	void JobWorkerThread::ClaimJob(Job* job)
	{
		std::lock_guard<std::mutex> lock(m_jobsMutex);
		m_jobs.push_back(job);
		m_waitingForJob = false;
		// Wake up the thread to process the next job
		m_condition.notify_one();
	}

	void JobWorkerThread::ThreadMain()
	{
		while (m_running)
		{
			std::unique_lock<std::mutex> lock(m_jobsMutex);
			m_condition.wait(lock, [this] { return !m_jobs.empty() || !m_running; });

			if (!m_jobs.empty())
			{
				Job* job = m_jobs.front();
				m_jobs.pop_front();
				lock.unlock();
				job->Execute();
				m_owningJobSystem->SubmitCompletedClaimedJobFromWorkerThread(job);
			}

			while (Job* job = m_owningJobSystem->ClaimNextJob())
			{
				job->Execute();
				m_owningJobSystem->SubmitCompletedClaimedJobFromWorkerThread(job);
			}

			m_waitingForJob = true;
		}
	}
}