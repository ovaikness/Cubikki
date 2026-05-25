#pragma once

#include "Engine/Threads/Job.hpp"

#include <future>
#include <set>
#include <mutex>
#include <vector>
#include <deque>

namespace PBE
{
	class JobWorkerThread;
	class JobSystem
	{
	public:
		JobSystem();
		JobSystem(int numThreads);
		~JobSystem();

		void SubmitJob(Job* job);

		bool IsJobComplete(Job* job);
		void WaitUntilJobComplete(Job* job);

		void ClaimJob(Job* job);
		Job* ClaimJobIfComplete(Job* job = nullptr);
		Job* WaitUntilAndClaimJobIfComplete(Job* job);

		void SetNumWorkerThreads(int numThreads);

		void Shutdown();
	private:
		int m_numThreads = 0;

		std::condition_variable m_jobCompleteCondition;

		std::mutex m_waitingJobMutex;
		std::deque<Job*> m_jobsAwaitingClaim;

		std::mutex m_claimMutex;
		std::set<Job*> m_claimedJobs;

		std::mutex m_completedMutex;
		std::set<Job*> m_completedJobs;

		std::vector<Job*> m_jobsToFree;

		int m_nextWorkerIndex = 0;
		std::vector<JobWorkerThread*> m_workerThreads;
	private:
		friend class JobWorkerThread;
		bool FindWorkerThreadAndClaimJob(Job* job);
		void SubmitCompletedClaimedJobFromWorkerThread(Job* job);
		Job* ClaimNextJob();
		void JoinAndClearWorkers();
		bool UnsafeIsJobComplete(Job* job);
		Job* UnsafeClaimJobIfComplete(Job* job);
	};
}