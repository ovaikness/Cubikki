#pragma once

#include <functional>
#include <vector>
#include <algorithm>

namespace PBE
{
	extern unsigned int g_EventDispatcherIDCounter;

	//PBE::SubscriberHandle<> is a template struct that represents a subscriber to an event dispatcher.
	template <typename... Args>
	struct SubscriberHandle
	{
		using Delegate = std::function<bool(Args...)>;

		unsigned int m_ID;
		Delegate m_Delegate;

		SubscriberHandle()
			: m_ID(static_cast<unsigned int>(~0))
		{
		}

		SubscriberHandle(Delegate delegate)
			: m_ID(++g_EventDispatcherIDCounter), m_Delegate(delegate)
		{
		}

		bool operator==(const SubscriberHandle<Args...>& other) const
		{
			return m_ID == other.m_ID;
		}
	};

	//PBE::EventDispatcher<> is a template class that manages a list of subscribers, 
	// allowing them to register callback functions and invoke them with specified arguments, 
	// facilitating an event-driven programming model.
	template <typename... Args>
	class EventDispatcher
	{
	public:
		using Delegate = std::function<bool(Args...)>;

		template <typename T_ValueType>
		SubscriberHandle<Args...> AddSubscriber(T_ValueType* instance, bool (T_ValueType::* function)(Args...))
		{
			auto lambda = [instance, function](Args... args) {
				return (instance->*function)(args...);
				};

			SubscriberHandle<Args...> handle(lambda);
			m_Listeners.push_back(handle);
			return handle;
		}

		SubscriberHandle<Args...> AddSubscriber(Delegate listener) {
			SubscriberHandle<Args...> handle(listener);
			m_Listeners.push_back(handle);
			return handle;
		}

		void RemoveSubscriber(SubscriberHandle<Args...> listenerHandle) {
			m_Listeners.erase(
				std::remove_if(m_Listeners.begin(), m_Listeners.end(),
					[&listenerHandle](const SubscriberHandle<Args...>& handle) {
						return handle.m_ID == listenerHandle.m_ID;
					}),
				m_Listeners.end()
			);
		}

		void operator()(Args... args) {
			Invoke(args...);
		}

		void Invoke(Args... args) {
			for (const auto& listener : m_Listeners) {
				if (listener.m_Delegate(args...))
				{
					return;
				}
			}
		}
	private:
		std::vector<SubscriberHandle<Args...>> m_Listeners;
	};
}
