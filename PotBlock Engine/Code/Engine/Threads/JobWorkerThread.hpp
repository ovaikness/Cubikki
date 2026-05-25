#pragma once
#include "Engine/Threads/Job.hpp"

#include <condition_variable>
#include <atomic>
#include <thread>
#include <deque>

namespace PBE
{
	class JobSystem;

	class JobWorkerThread
	{
	public:
		JobWorkerThread(JobSystem* owner, int id);
		~JobWorkerThread();

		void Start();
		void Stop();
		void ClaimJob(Job* job);
	private:
		friend class JobSystem;

		int m_id;
		JobSystem* m_owningJobSystem;

		std::atomic<bool> m_running = false;
		std::atomic<bool> m_waitingForJob = true;

		std::condition_variable m_condition;
		std::mutex m_jobsMutex;

		std::thread m_thread;

		std::deque<Job*> m_jobs;

		void ThreadMain();
	};
}