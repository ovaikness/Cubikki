#pragma once

#include <atomic>

namespace PBE
{
	enum class JobStatus : unsigned char
	{
		WAITING,
		CLAIMED,
		COMPLETED
	};


	class Job
	{
	public:
		virtual void Execute() = 0;
		inline JobStatus GetStatus() const {
			return m_status;
		};
		inline void SetStatus(JobStatus status) {
			m_status = status;
		};
	private:
		friend class JobSystem;
		friend class JobWorkerThread;
		std::atomic<JobStatus> m_status = JobStatus::WAITING;
	};
}