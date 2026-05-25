#pragma once

#include <map>
#include <variant>
#include <string>
#include <string_view>
#include <algorithm>

#include "Engine/Core/HCIString.hpp"
#include "Engine/Core/BufferReader.hpp"
#include "Engine/Core/GrowBuffer.hpp"
#include "Engine/Core/EventDispatcher.hpp"

#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Vec4.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Math/Mat4.hpp"

#include "Engine/Core/Result.hpp"
#include "nlohmann/json.hpp"

namespace PBE
{
	class PropertyWrapperBase
	{
	public:
		virtual ~PropertyWrapperBase() = default;
		virtual void WriteToGrowBuffer(GrowBuffer& buffer) const = 0;
		virtual void ReadFromBufferReader(BufferReader& reader) = 0;
	};

	template <typename T_Value>
	class PropertyWrapper
	{
	public:
		T_Value m_Value;
		PropertyWrapper(T_Value const& value)
			: m_Value(value)
		{
		}
		PropertyWrapper(T_Value&& value)
			: m_Value(std::move(value))
		{
		}
		virtual ~PropertyWrapper() = default;
		T_Value& GetValue()
		{
			return m_Value;
		};
	};


	enum class PropertyType : uint8_t
	{
		INT32,
		FLOAT,
		STRING,
		VEC2,
		VEC3,
		VEC4,
		EULER_ANGLES,
		CUSTOM
	};

	// Properties are 16 bytes in size.
	using Property = std::variant<
		int,
		float,
		std::string,
		Vec2,
		Vec3,
		Vec4,
		EulerAngles,
		std::shared_ptr<PropertyWrapperBase>
	>;

	class NamedProperties;

	extern PBE::EventDispatcher<std::shared_ptr<PropertyWrapperBase>> g_NamedPropertyWriteCallbacks;
	extern PBE::EventDispatcher<NamedProperties*,std::shared_ptr<PropertyWrapperBase>> g_NamedPropertyReadCallbacks;

	class NamedProperties
	{
	public:
		std::map<HCIString, Property> m_Properties;
	public:
		void AddProperty(HCIString const& name, Property&& property);

		void AddProperty(HCIString const& name, Property const& property);

		PBE::Property& operator[](HCIString const& name);

		PBE::Property const& operator[](HCIString const& name) const;

		bool HasProperty(HCIString const& name) const;

		PBE::Result GetProperty(HCIString const& name, Property& property) const;
		void SetProperty(HCIString const& name, Property const& property);
		PBE::Result GetInt32(HCIString const& name, int& out_int) const;
		void SetInt32(HCIString const& name, int value);
		PBE::Result GetFloat(HCIString const& name, float& out_float) const;
		void SetFloat(HCIString const& name, float value);
		PBE::Result GetString(HCIString const& name, std::string& out_string) const;
		void SetString(HCIString const& name, std::string const& value);
		PBE::Result GetVec2(HCIString const& name, Vec2& out_vec2) const;
		void SetVec2(HCIString const& name, Vec2 const& value);
		PBE::Result GetVec3(HCIString const& name, Vec3& out_vec3) const;
		void SetVec3(HCIString const& name, Vec3 const& value);
		PBE::Result GetVec4(HCIString const& name, Vec4& out_vec4) const;
		void SetVec4(HCIString const& name, Vec4 const& value);
		PBE::Result GetEulerAngles(HCIString const& name, EulerAngles& out_eulerAngles) const;
		void SetEulerAngles(HCIString const& name, EulerAngles const& value);
		PBE::Result GetCustom(HCIString const& name, std::shared_ptr<PropertyWrapperBase>& out_custom) const;
		void SetCustom(HCIString const& name, std::shared_ptr<PropertyWrapperBase> const& value);
		void WriteToGrowBuffer(GrowBuffer& buffer) const;
		std::vector<uint8_t> GetBuffer() const;
		PBE::Result LoadFromBufferReader(BufferReader& reader);

		std::string GetJSON() const;
		PBE::Result LoadFromJSONString(std::string_view string);
	};
}