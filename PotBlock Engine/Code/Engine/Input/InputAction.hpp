#pragma once
#include "Engine/Core/HCIString.hpp"
#include <functional>
#include <variant>

namespace PBE
{
	enum InputActionValueType
	{
		INPUT_ACTION_VALUE_TYPE_BOOL,
		INPUT_ACTION_VALUE_TYPE_FLOAT,
		INPUT_ACTION_VALUE_TYPE_VEC2
	};

	enum InputActionMode
	{
		INPUT_ACTION_MODE_STARTED,
		INPUT_ACTION_MODE_HELD,
		INPUT_ACTION_MODE_ENDED
	};

	using InputActionValue = std::variant<bool, float, Vec2>;

	struct InputAction
	{
		InputActionValueType m_Type;
		PBE::HCIString m_Name;

		std::function<void(InputActionValue const& value)> m_Started;
		std::function<void(InputActionValue const& value)> m_Held;
		std::function<void(InputActionValue const& value)> m_Ended;
	};
}