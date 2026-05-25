#pragma once

#include "Engine/Threads/JobSystem.hpp"

#include <vector>

namespace PBE
{
	enum SystemExecutionGraphNodeState
	{
		WAITING,
		RUNNING,
		COMPLETE
	};

	struct SystemExecutionGraphNode
	{
	public:
		SystemExecutionGraphNodeState m_State;
		int m_DependencyID;
		std::vector<size_t> m_Dependencies;
		Job* m_Job;
	public:
		SystemExecutionGraphNode(int id, Job* job, std::vector<size_t> const& dependencies)
			: m_State(WAITING), m_DependencyID(id), m_Job(job), m_Dependencies(dependencies)
		{
		}
	};

	class JobExecutionGraph
	{
	public:
		JobSystem* m_JobSystem;
		std::vector<SystemExecutionGraphNode> m_Nodes;
	public:
		JobExecutionGraph(JobSystem* jobSystem);
		~JobExecutionGraph();
		void AddNode(int id, Job* job, std::vector<size_t> const& dependencies);
		void Execute();
	};
}