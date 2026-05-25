#pragma once

#include <functional>
#include <tuple>
#include <sstream>
#include <vector>
#include <string>
#include <stdexcept>

namespace PBE
{
	class DevConsole;

	struct CommandBase
	{
		virtual ~CommandBase() = default;
		virtual bool Execute(std::vector<std::string> const& args) = 0;
	};

	template<typename... Args>
	struct Command : public CommandBase
	{
		std::function<bool(Args...)> m_Function;
		std::string m_Description = "Default description.";

		Command(std::function<bool(Args...)> func, std::string_view description)
			: m_Function(std::move(func)), m_Description(description) {}

		bool Execute(std::vector<std::string> const& args) override
		{
			if (args.size() != sizeof...(Args)) {
				return false;
			}

			return CallWithArgs(std::index_sequence_for<Args...>{}, args);
		}

	private:
		template<std::size_t... Is>
		bool CallWithArgs(std::index_sequence<Is...>, std::vector<std::string> const& args)
		{
			return m_Function(ParseArg<Args>(args[Is])...);
		}

		template<typename T_ValueType>
		T_ValueType ParseArg(std::string_view arg)
		{
			std::istringstream iss(std::string(arg));
			T_ValueType value{};

			if constexpr (std::is_arithmetic_v<T_ValueType>)
			{
				iss >> value;
				if (iss.fail()) {
					throw std::runtime_error("Failed to parse argument.");
				}
			}
			else if constexpr (std::is_same_v<T_ValueType, std::string>)
			{
				value = std::string(arg);
			}
			else if constexpr (std::is_same_v<T_ValueType, std::string_view>)
			{
				value = arg;
			}
			else if constexpr (std::is_same_v<T_ValueType, const char*>)
			{
				value = arg.data();
			}
			else if constexpr (std::is_same_v<T_ValueType, std::wstring>)
			{
				value = std::wstring(arg.begin(), arg.end());
			}
			else if constexpr (std::is_same_v<T_ValueType, std::wstring_view>)
			{
				value = std::wstring_view(arg.begin(), arg.end());
			}
			else 
			{
				static_assert(true && "Unsupported type for ParseArg.");
			}

			return value;
		}
	};
}