#pragma once

namespace PBE
{
	template<typename T_ValueType>
	class LuaWrapper
	{
	public:
		T_ValueType* m_value = nullptr;

		LuaWrapper() = default;
		LuaWrapper(T_ValueType* value)
			: m_value(value)
		{
		}

		LuaWrapper& operator=(T_ValueType value)
		{
			*m_value = value;
			return *this;
		}

		operator T_ValueType() const
		{
			return *m_value;
		}

		T_ValueType get()
		{
			return *m_value;
		}

		void set(T_ValueType value)
		{
			*m_value = value;
		}
	};
}