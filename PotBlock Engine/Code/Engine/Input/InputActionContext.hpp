#pragma once
#include "Engine/Math/Vec2.hpp"

#include "Engine/Input/InputAction.hpp"

#include <map>

namespace PBE
{

	class InputActionContext
	{
	public:
		InputActionContext() = default;
		~InputActionContext() = default;

		void BindAction(std::string const& actionName, InputAction const& action)
		{
			m_Actions[actionName] = action;
		}
		void ExecuteAction(std::string const& actionName, InputActionValue const& value, InputActionMode mode)
		{
			auto itr = m_Actions.find(actionName);
			if (itr == m_Actions.end())
			{
				return;
			}

			// Check if the value is the correct type. If not, return.
			// #TODO Add a way to log this.

			if (itr->second.m_Type == INPUT_ACTION_VALUE_TYPE_BOOL && !std::holds_alternative<bool>(value))
			{
				return;
			}
			else if (itr->second.m_Type == INPUT_ACTION_VALUE_TYPE_FLOAT && !std::holds_alternative<float>(value))
			{
				return;
			}
			else if (itr->second.m_Type == INPUT_ACTION_VALUE_TYPE_VEC2 && !std::holds_alternative<Vec2>(value))
			{
				return;
			}

			switch (mode)
			{
			case INPUT_ACTION_MODE_STARTED:
			{
				itr->second.m_Started(value);
				break;
			}
			case INPUT_ACTION_MODE_HELD:
			{
				itr->second.m_Held(value);
				break;
			}
			case INPUT_ACTION_MODE_ENDED:
			{
				itr->second.m_Ended(value);
				break;
			}
			}
		}
	private:
		std::map<std::string, InputAction> m_Actions;
	};
}