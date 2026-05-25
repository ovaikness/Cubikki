#pragma once

#include <shared_mutex>

namespace PBE
{
	template<typename T_ValueType>
	class atomic_vector
	{
	public:
		atomic_vector() = default;
		atomic_vector(const atomic_vector<T_ValueType>& other)
		{
			std::unique_lock<std::shared_mutex> lock(other.m_mutex);
			m_vector = other.m_vector;
		}

		void push_back(const T_ValueType& value)
		{
			std::unique_lock<std::shared_mutex> lock(m_mutex);
			m_vector.push_back(value);
		}

		void clear()
		{
			std::unique_lock<std::shared_mutex> lock(m_mutex);
			m_vector.clear();
		}

		std::vector<T_ValueType> get()
		{
			std::unique_lock<std::shared_mutex> lock(m_mutex);
			return m_vector;
		}

		void set(const std::vector<T_ValueType>& vector)
		{
			std::unique_lock<std::shared_mutex> lock(m_mutex);
			m_vector = vector;
		}

		void set(std::vector<T_ValueType>&& vector)
		{
			std::unique_lock<std::shared_mutex> lock(m_mutex);
			m_vector = std::move(vector);
		}

		size_t size() const
		{
			std::shared_lock<std::shared_mutex> lock(m_mutex);
			return m_vector.size();
		}

		T_ValueType& operator[](size_t index)
		{
			std::unique_lock<std::shared_mutex> lock(m_mutex);
			return m_vector[index];
		}

		const T_ValueType& operator[](size_t index) const
		{
			std::shared_lock<std::shared_mutex> lock(m_mutex);
			return m_vector[index];
		}

		typename std::vector<T_ValueType>::iterator begin()
		{
			std::unique_lock<std::shared_mutex> lock(m_mutex);
			return m_vector.begin();
		}

		typename std::vector<T_ValueType>::iterator end()
		{
			std::unique_lock<std::shared_mutex> lock(m_mutex);
			return m_vector.end();
		}

		typename std::vector<T_ValueType>::const_iterator begin() const
		{
			std::shared_lock<std::shared_mutex> lock(m_mutex);
			return m_vector.begin();
		}

		typename std::vector<T_ValueType>::const_iterator end() const
		{
			std::shared_lock<std::shared_mutex> lock(m_mutex);
			return m_vector.end();
		}

		typename std::vector<T_ValueType>::const_iterator cbegin() const
		{
			std::shared_lock<std::shared_mutex> lock(m_mutex);
			return m_vector.cbegin();
		}

		typename std::vector<T_ValueType>::const_iterator cend() const
		{
			std::shared_lock<std::shared_mutex> lock(m_mutex);
			return m_vector.cend();
		}

		typename std::vector<T_ValueType>::reverse_iterator rbegin()
		{
			std::unique_lock<std::shared_mutex> lock(m_mutex);
			return m_vector.rbegin();
		}

		typename std::vector<T_ValueType>::reverse_iterator rend()
		{
			std::unique_lock<std::shared_mutex> lock(m_mutex);
			return m_vector.rend();
		}

		typename std::vector<T_ValueType>::const_reverse_iterator rbegin() const
		{
			std::shared_lock<std::shared_mutex> lock(m_mutex);
			return m_vector.rbegin();
		}

		typename std::vector<T_ValueType>::const_reverse_iterator rend() const
		{
			std::shared_lock<std::shared_mutex> lock(m_mutex);
			return m_vector.rend();
		}

		typename std::vector<T_ValueType>::const_reverse_iterator crbegin() const
		{
			std::shared_lock<std::shared_mutex> lock(m_mutex);
			return m_vector.crbegin();
		}

		typename std::vector<T_ValueType>::const_reverse_iterator crend() const
		{
			std::shared_lock<std::shared_mutex> lock(m_mutex);
			return m_vector.crend();
		}

		bool empty() const
		{
			std::unique_lock<std::shared_mutex> lock(m_mutex);
			return m_vector.empty();
		}

		void erase(typename std::vector<T_ValueType>::iterator it)
		{
			std::unique_lock<std::shared_mutex> lock(m_mutex);
			m_vector.erase(it);
		}

		void erase(typename std::vector<T_ValueType>::iterator first, typename std::vector<T_ValueType>::iterator last)
		{
			std::unique_lock<std::shared_mutex> lock(m_mutex);
			m_vector.erase(first, last);
		}

		void insert(typename std::vector<T_ValueType>::iterator it, const T_ValueType& value)
		{
			std::unique_lock<std::shared_mutex> lock(m_mutex);
			m_vector.insert(it, value);
		}

		void insert(typename std::vector<T_ValueType>::iterator it, T_ValueType&& value)
		{
			std::unique_lock<std::shared_mutex> lock(m_mutex);
			m_vector.insert(it, std::move(value));
		}

		void insert(typename std::vector<T_ValueType>::iterator it, size_t count, const T_ValueType& value)
		{
			std::unique_lock<std::shared_mutex> lock(m_mutex);
			m_vector.insert(it, count, value);
		}

		void insert(typename std::vector<T_ValueType>::iterator it, typename std::vector<T_ValueType>::iterator first, typename std::vector<T_ValueType>::iterator last)
		{
			std::unique_lock<std::shared_mutex> lock(m_mutex);
			m_vector.insert(it, first, last);
		}

		void pop_back()
		{
			std::unique_lock<std::shared_mutex> lock(m_mutex);
			m_vector.pop_back();
		}

		void resize(size_t count)
		{
			std::unique_lock<std::shared_mutex> lock(m_mutex);
			m_vector.resize(count);
		}

		void resize(size_t count, const T_ValueType& value)
		{
			std::unique_lock<std::shared_mutex> lock(m_mutex);
			m_vector.resize(count, value);
		}

		void reserve(size_t count)
		{
			std::unique_lock<std::shared_mutex> lock(m_mutex);
			m_vector.reserve(count);
		}

		void shrink_to_fit()
		{
			std::unique_lock<std::shared_mutex> lock(m_mutex);
			m_vector.shrink_to_fit();
		}

		void swap(std::vector<T_ValueType>& other)
		{
			std::unique_lock<std::shared_mutex> lock(m_mutex);
			m_vector.swap(other);
		}

		void swap(atomic_vector<T_ValueType>& other)
		{
			std::lock(m_mutex, other.m_mutex);
			std::unique_lock<std::shared_mutex> lock1(m_mutex, std::adopt_lock);
			std::unique_lock<std::shared_mutex> lock2(other.m_mutex, std::adopt_lock);
			m_vector.swap(other.m_vector);
		}

		atomic_vector<T_ValueType>& operator=(const atomic_vector<T_ValueType>& other)
		{
			std::lock(m_mutex, other.m_mutex);
			std::unique_lock<std::shared_mutex> lock1(m_mutex, std::adopt_lock);
			std::unique_lock<std::shared_mutex> lock2(other.m_mutex, std::adopt_lock);
			m_vector = other.m_vector;
			return *this;
		}

	protected:
		std::vector<T_ValueType> m_vector;
		std::shared_mutex m_mutex;
	};
}