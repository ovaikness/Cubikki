#include "Engine/Threads/JobExecutionGraph.hpp"

PBE::JobExecutionGraph::JobExecutionGraph(JobSystem* jobSystem)
	: m_JobSystem(jobSystem)
{

}

PBE::JobExecutionGraph::~JobExecutionGraph()
{

}

void PBE::JobExecutionGraph::AddNode(int id, Job* job, std::vector<size_t> const& dependencies)
{
	m_Nodes.push_back(SystemExecutionGraphNode{ id, job, dependencies });
}

void PBE::JobExecutionGraph::Execute()
{
	for (auto& node : m_Nodes)
	{
		node.m_State = SystemExecutionGraphNodeState::WAITING;
	}

	// Keep track of the number of nodes that have been completed
	// Don't use the set because some nodes can share an ID.
	int numCompleted{ 0 };
	std::set<int> nodesCompleted{};

	while (numCompleted != (int)m_Nodes.size())
	{
		for (auto& node : m_Nodes)
		{
			if (node.m_State != SystemExecutionGraphNodeState::WAITING)
			{
				continue;
			}
			for (auto& dependency : node.m_Dependencies)
			{
				// If the dependency hasn't been completed yet, skip this node
				if (nodesCompleted.find(dependency) == nodesCompleted.end())
				{
					continue;
				}

				m_JobSystem->SubmitJob(node.m_Job);
				node.m_State = SystemExecutionGraphNodeState::RUNNING;
			}
		}

		for (auto& node : m_Nodes)
		{
			if (node.m_State == SystemExecutionGraphNodeState::RUNNING)
			{
				m_JobSystem->WaitUntilAndClaimJobIfComplete(node.m_Job);
				node.m_State = SystemExecutionGraphNodeState::COMPLETE;
				nodesCompleted.insert(node.m_DependencyID);
			}
		}
	}
}