#pragma once

#include <atomic>
#include <deque>
#include <mutex>
#include <condition_variable>

namespace PBE
{
	template<typename T_ValueType>
	class ThreadSafeQueue
	{
	private:
		std::deque<T_ValueType> m_Queue;
		std::mutex m_Mutex;
		std::condition_variable m_Condition;

	public:
		ThreadSafeQueue() = default;
		ThreadSafeQueue(const ThreadSafeQueue<T_ValueType>&) = delete;
		~ThreadSafeQueue() {
			Clear();
		}

	public:

		const T_ValueType& Front()
		{
			std::scoped_lock lock(m_Mutex);
			return m_Queue.front();
		}
		const T_ValueType& Back()
		{
			std::scoped_lock lock(m_Mutex);
			return m_Queue.back();
		}
		T_ValueType PopFront()
		{
			std::scoped_lock lock(m_Mutex);
			auto t = std::move(m_Queue.front());
			m_Queue.pop_front();
			return t;
		}
		T_ValueType PopBack()
		{
			std::scoped_lock lock(m_Mutex);
			auto t = std::move(m_Queue.back());
			m_Queue.pop_back();
			return t;
		}
		void PushFront(const T_ValueType& item)
		{
			std::scoped_lock lock(m_Mutex);
			m_Queue.emplace_front(std::move(item));
			m_Condition.notify_one();
		}
		void PushBack(const T_ValueType& item)
		{
			std::scoped_lock lock(m_Mutex);
			m_Queue.emplace_back(std::move(item));
			m_Condition.notify_one();
		}
		bool Empty()
		{
			std::scoped_lock lock(m_Mutex);
			return m_Queue.empty();
		}
		size_t Count()
		{
			std::scoped_lock lock(m_Mutex);
			return m_Queue.size();
		}
		void Clear()
		{
			std::scoped_lock lock(m_Mutex);
			m_Queue.clear();
		}

		void Wait()
		{
			std::unique_lock<std::mutex> lock(m_Mutex);
			m_Condition.wait(lock, [this]() { return !m_Queue.empty(); });
		}
	};
};