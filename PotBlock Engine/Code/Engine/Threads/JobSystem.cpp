#include "Engine/Threads/JobSystem.hpp"
#include "Engine/Threads/JobWorkerThread.hpp"

#include <stdexcept>
#include <thread>

namespace PBE
{
	JobSystem::JobSystem()
		: JobSystem(std::thread::hardware_concurrency() - 1)
	{
		if (m_numThreads < 0)
		{
			throw std::runtime_error("Cannot initialize job system with negative number of threads!");
		}

		for (int i = 0; i < m_numThreads; i++)
		{
			JobWorkerThread* worker = new JobWorkerThread(this, i);
			m_workerThreads.push_back(worker);
			worker->Start();
		}
	}

	JobSystem::JobSystem(int numThreads)
		: m_numThreads(numThreads)
	{
		if (m_numThreads < 0)
		{
			throw std::runtime_error("Cannot initialize job system with negative number of threads!");
		}

		for (int i = 0; i < m_numThreads; i++)
		{
			JobWorkerThread* worker = new JobWorkerThread(this, i);
			m_workerThreads.push_back(worker);
			worker->Start();
		}
	}

	JobSystem::~JobSystem()
	{
		JoinAndClearWorkers();
	}

	void JobSystem::SubmitJob(Job* job)
	{
		job->m_status = JobStatus::WAITING;
		if (FindWorkerThreadAndClaimJob(job))
		{
			// Job was claimed by a worker thread
			return;
		}
		// Jobs to be claimed by worker threads. The worker thread will claim the job when it is ready to process it.
		std::lock_guard lock(m_waitingJobMutex);
		m_jobsAwaitingClaim.push_back(job);
	}

	bool JobSystem::IsJobComplete(Job* job)
	{
		std::lock_guard<std::mutex> lock(m_completedMutex);

		return UnsafeIsJobComplete(job);
	}

	void JobSystem::WaitUntilJobComplete(Job* job)
	{
		std::unique_lock<std::mutex> lock(m_completedMutex);

		m_jobCompleteCondition.wait(lock, [&]
			{
				return UnsafeIsJobComplete(job);
			}
		);
	}

	void JobSystem::ClaimJob(Job* job)
	{
		SubmitCompletedClaimedJobFromWorkerThread(job);
		ClaimJobIfComplete(job);
	}

	Job* JobSystem::ClaimJobIfComplete(Job* job)
	{
		std::lock_guard<std::mutex> lock(m_completedMutex);
		return UnsafeClaimJobIfComplete(job);
	}

	Job* JobSystem::WaitUntilAndClaimJobIfComplete(Job* job)
	{
		std::unique_lock<std::mutex> lock(m_completedMutex);

		m_jobCompleteCondition.wait(lock, [&]
			{
				return UnsafeClaimJobIfComplete(job) != nullptr;
			}
		);

		return job;
	}

	void JobSystem::SetNumWorkerThreads(int numThreads)
	{
		JoinAndClearWorkers();

		for (int i = 0; i < numThreads; i++)
		{
			JobWorkerThread* worker = new JobWorkerThread(this, i);
			m_workerThreads.push_back(worker);
			worker->Start();
		}
	}

	void JobSystem::Shutdown()
	{
		JoinAndClearWorkers();
	}

	bool JobSystem::FindWorkerThreadAndClaimJob(Job* job)
	{
		for (JobWorkerThread* worker : m_workerThreads)
		{
			if (worker->m_waitingForJob)
			{
				worker->ClaimJob(job);
				return true;
			}
		}
		return false;
	}

	void JobSystem::SubmitCompletedClaimedJobFromWorkerThread(Job* job)
	{
		{
			std::lock_guard lock(m_claimMutex);

			auto jobItr = m_claimedJobs.find(job);
			if (jobItr != m_claimedJobs.end())
			{
				m_claimedJobs.erase(jobItr);
			}
		}

		{
			std::lock_guard lock(m_completedMutex);
			m_completedJobs.insert(job);
			job->m_status = JobStatus::COMPLETED;
		}

		m_jobCompleteCondition.notify_all();
	}

	Job* JobSystem::ClaimNextJob()
	{
		Job* nextJob = nullptr;

		{
			std::lock_guard lock(m_waitingJobMutex);

			if (m_jobsAwaitingClaim.empty())
			{
				return nullptr;
			}

			nextJob = m_jobsAwaitingClaim.front();
			m_jobsAwaitingClaim.pop_front();
		}

		{
			std::lock_guard lock(m_claimMutex);
			m_claimedJobs.insert(nextJob);
		}

		nextJob->m_status = JobStatus::CLAIMED;
		return nextJob;
	}

	void JobSystem::JoinAndClearWorkers()
	{
		for (JobWorkerThread* worker : m_workerThreads)
		{
			worker->Stop();
			delete worker;
		}
		m_workerThreads.clear();
	}

	bool JobSystem::UnsafeIsJobComplete(Job* job)
	{
		if (job == nullptr)
		{
			// If no job is specified, return the next completed job
			if (m_completedJobs.empty())
			{
				return false;
			}
			Job* nextJob = *m_completedJobs.begin();
			return nextJob != nullptr;
		}

		auto jobItr = m_completedJobs.find(job);
		if (jobItr != m_completedJobs.end())
		{
			return job != nullptr;
		}

		return false;
	}

	Job* JobSystem::UnsafeClaimJobIfComplete(Job* job)
	{
		if (job == nullptr)
		{
			// If no job is specified, return the next completed job
			if (m_completedJobs.empty())
			{
				return nullptr;
			}
			Job* nextJob = *m_completedJobs.begin();
			m_completedJobs.erase(m_completedJobs.begin());
			if (nextJob)
			{
				nextJob->m_status = JobStatus::WAITING;
			}
			return nextJob;
		}

		auto jobItr = m_completedJobs.find(job);
		if (jobItr != m_completedJobs.end())
		{
			m_completedJobs.erase(jobItr);
			job->m_status = JobStatus::WAITING;
			return job;
		}

		return nullptr;
	}
}