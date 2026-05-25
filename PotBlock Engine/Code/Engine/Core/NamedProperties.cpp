#include "Engine/Core/NamedProperties.hpp"

void PBE::NamedProperties::AddProperty(HCIString const& name, Property&& property)
{
	auto itr = m_Properties.find(name);
	if (itr != m_Properties.end())
	{
		// If the property already exists, we can just replace it.
		itr->second = property;
		return;
	}
	m_Properties[name] = property;
}

void PBE::NamedProperties::AddProperty(HCIString const& name, Property const& property)
{
	auto itr = m_Properties.find(name);
	if (itr != m_Properties.end())
	{
		// If the property already exists, we can just replace it.
		itr->second = property;
		return;
	}
	m_Properties[name] = property;
}

PBE::Property& PBE::NamedProperties::operator[](HCIString const& name)
{
	return m_Properties[name];
}

bool PBE::NamedProperties::HasProperty(HCIString const& name) const
{
	return m_Properties.find(name) != m_Properties.end();
}

PBE::Property const& PBE::NamedProperties::operator[](HCIString const& name) const
{
	return m_Properties.at(name);
}

PBE::Result PBE::NamedProperties::GetProperty(HCIString const& name, Property& property) const
{
	auto itr = m_Properties.find(name);
	if (itr == m_Properties.end())
	{
		return RESULT_PROPERTY_DOES_NOT_EXIST;
	}

	property = itr->second;

	return RESULT_SUCCESS;
}

void PBE::NamedProperties::SetProperty(HCIString const& name, Property const& property)
{
	m_Properties[name] = property;
}

PBE::Result PBE::NamedProperties::GetInt32(HCIString const& name, int& out_int) const
{
	auto itr = m_Properties.find(name);
	if (itr == m_Properties.end())
	{
		return RESULT_PROPERTY_DOES_NOT_EXIST;
	}

	Property const& property = itr->second;

	if (!std::holds_alternative<int>(property))
	{
		return RESULT_PROPERTY_HAS_WRONG_TYPE;
	}

	out_int = std::get<int>(property);

	return RESULT_SUCCESS;
}

void PBE::NamedProperties::SetInt32(HCIString const& name, int value)
{
	auto itr = m_Properties.find(name);
	if (itr == m_Properties.end())
	{
		m_Properties[name] = value;
	}
	else
	{
		itr->second = value;
	}
}

PBE::Result PBE::NamedProperties::GetFloat(HCIString const& name, float& out_float) const
{
	auto itr = m_Properties.find(name);
	if (itr == m_Properties.end())
	{
		return RESULT_PROPERTY_DOES_NOT_EXIST;
	}

	Property const& property = itr->second;

	if (!std::holds_alternative<float>(property))
	{
		return RESULT_PROPERTY_HAS_WRONG_TYPE;
	}

	out_float = std::get<float>(property);

	return RESULT_SUCCESS;
}

void PBE::NamedProperties::SetFloat(HCIString const& name, float value)
{
	auto itr = m_Properties.find(name);
	if (itr == m_Properties.end())
	{
		m_Properties[name] = value;
	}
	else
	{
		itr->second = value;
	}
}

PBE::Result PBE::NamedProperties::GetString(HCIString const& name, std::string& out_string) const
{
	auto itr = m_Properties.find(name);
	if (itr == m_Properties.end())
	{
		return RESULT_PROPERTY_DOES_NOT_EXIST;
	}

	Property const& property = itr->second;

	if (!std::holds_alternative<std::string>(property))
	{
		return RESULT_PROPERTY_HAS_WRONG_TYPE;
	}

	out_string = std::get<std::string>(property);

	return RESULT_SUCCESS;
}

void PBE::NamedProperties::SetString(HCIString const& name, std::string const& value)
{
	auto itr = m_Properties.find(name);
	if (itr == m_Properties.end())
	{
		m_Properties[name] = value;
	}
	else
	{
		itr->second = value;
	}
}

PBE::Result PBE::NamedProperties::GetVec2(HCIString const& name, Vec2& out_vec2) const
{
	auto itr = m_Properties.find(name);
	if (itr == m_Properties.end())
	{
		return RESULT_PROPERTY_DOES_NOT_EXIST;
	}
	Property const& property = itr->second;
	if (!std::holds_alternative<Vec2>(property))
	{
		return RESULT_PROPERTY_HAS_WRONG_TYPE;
	}
	out_vec2 = std::get<Vec2>(property);
	return RESULT_SUCCESS;
}

void PBE::NamedProperties::SetVec2(HCIString const& name, Vec2 const& value)
{
	auto itr = m_Properties.find(name);
	if (itr == m_Properties.end())
	{
		m_Properties[name] = value;
	}
	else
	{
		itr->second = value;
	}
}

PBE::Result PBE::NamedProperties::GetVec3(HCIString const& name, Vec3& out_vec3) const
{
	auto itr = m_Properties.find(name);
	if (itr == m_Properties.end())
	{
		return RESULT_PROPERTY_DOES_NOT_EXIST;
	}
	Property const& property = itr->second;
	if (!std::holds_alternative<Vec3>(property))
	{
		return RESULT_PROPERTY_HAS_WRONG_TYPE;
	}
	out_vec3 = std::get<Vec3>(property);
	return RESULT_SUCCESS;
}

void PBE::NamedProperties::SetVec3(HCIString const& name, Vec3 const& value)
{
	auto itr = m_Properties.find(name);
	if (itr == m_Properties.end())
	{
		m_Properties[name] = value;
	}
	else
	{
		itr->second = value;
	}
}

PBE::Result PBE::NamedProperties::GetVec4(HCIString const& name, Vec4& out_vec4) const
{
	auto itr = m_Properties.find(name);
	if (itr == m_Properties.end())
	{
		return RESULT_PROPERTY_DOES_NOT_EXIST;
	}
	Property const& property = itr->second;
	if (!std::holds_alternative<Vec4>(property))
	{
		return RESULT_PROPERTY_HAS_WRONG_TYPE;
	}
	out_vec4 = std::get<Vec4>(property);
	return RESULT_SUCCESS;
}

void PBE::NamedProperties::SetVec4(HCIString const& name, Vec4 const& value)
{
	auto itr = m_Properties.find(name);
	if (itr == m_Properties.end())
	{
		m_Properties[name] = value;
	}
	else
	{
		itr->second = value;
	}
}

PBE::Result PBE::NamedProperties::GetEulerAngles(HCIString const& name, EulerAngles& out_eulerAngles) const
{
	auto itr = m_Properties.find(name);
	if (itr == m_Properties.end())
	{
		return RESULT_PROPERTY_DOES_NOT_EXIST;
	}
	Property const& property = itr->second;
	if (!std::holds_alternative<EulerAngles>(property))
	{
		return RESULT_PROPERTY_HAS_WRONG_TYPE;
	}
	out_eulerAngles = std::get<EulerAngles>(property);
	return RESULT_SUCCESS;
}

void PBE::NamedProperties::SetEulerAngles(HCIString const& name, EulerAngles const& value)
{
	auto itr = m_Properties.find(name);
	if (itr == m_Properties.end())
	{
		m_Properties[name] = value;
	}
	else
	{
		itr->second = value;
	}
}

PBE::Result PBE::NamedProperties::GetCustom(HCIString const& name, std::shared_ptr<PropertyWrapperBase>& out_custom) const
{
	auto itr = m_Properties.find(name);
	if (itr == m_Properties.end())
	{
		return RESULT_PROPERTY_DOES_NOT_EXIST;
	}
	Property const& property = itr->second;
	if (!std::holds_alternative<std::shared_ptr<PropertyWrapperBase>>(property))
	{
		return RESULT_PROPERTY_HAS_WRONG_TYPE;
	}
	out_custom = std::get<std::shared_ptr<PropertyWrapperBase>>(property);
	return RESULT_SUCCESS;
}

void PBE::NamedProperties::SetCustom(HCIString const& name, std::shared_ptr<PropertyWrapperBase> const& value)
{
	auto itr = m_Properties.find(name);
	if (itr == m_Properties.end())
	{
		m_Properties[name] = value;
	}
	else
	{
		itr->second = value;
	}
}

void PBE::NamedProperties::WriteToGrowBuffer(PBE::GrowBuffer& buffer) const
{
	buffer.Write((int)m_Properties.size());
	for (auto& propertyPair : m_Properties)
	{
		HCIString const& key = propertyPair.first;
		Property const& value = propertyPair.second;
		buffer.WriteString(key.GetCasedString());
		if (std::holds_alternative<int>(value))
		{
			int intValue = std::get<int>(value);
			buffer.Write(PropertyType::INT32);
			buffer.Write(intValue);
		}
		else if (std::holds_alternative<float>(value))
		{
			float floatValue = std::get<float>(value);
			buffer.Write(PropertyType::FLOAT);
			buffer.Write(floatValue);
		}
		else if (std::holds_alternative<std::string>(value))
		{
			std::string stringValue = std::get<std::string>(value);
			buffer.Write(PropertyType::STRING);
			buffer.WriteString(stringValue);
		}
		else if (std::holds_alternative<Vec2>(value))
		{
			Vec2 vec2 = std::get<Vec2>(value);
			buffer.Write(PropertyType::VEC2);
			buffer.Write(vec2);
		}
		else if (std::holds_alternative<Vec3>(value))
		{
			Vec3 vec3 = std::get<Vec3>(value);
			buffer.Write(PropertyType::VEC3);
			buffer.Write(vec3);
		}
		else if (std::holds_alternative<Vec4>(value))
		{
			Vec4 vec4 = std::get<Vec4>(value);
			buffer.Write(PropertyType::VEC4);
			buffer.Write(vec4);
		}
		else if (std::holds_alternative<EulerAngles>(value))
		{
			EulerAngles eulerAngles = std::get<EulerAngles>(value);
			buffer.Write(PropertyType::EULER_ANGLES);
			buffer.Write(eulerAngles);
		}
		else if (std::holds_alternative<std::shared_ptr<PropertyWrapperBase>>(value))
		{
			std::shared_ptr<PropertyWrapperBase> customValue = std::get<std::shared_ptr<PropertyWrapperBase>>(value);
			buffer.Write(PropertyType::CUSTOM);
			customValue->WriteToGrowBuffer(buffer);
		}
	}
}

std::vector<uint8_t> PBE::NamedProperties::GetBuffer() const
{
	PBE::GrowBuffer buffer;
	WriteToGrowBuffer(buffer);
	return buffer.GetBuffer();
}

PBE::Result PBE::NamedProperties::LoadFromBufferReader(BufferReader& reader)
{
	int numProperties;
	if (reader.Read(numProperties) != PBE::RESULT_SUCCESS)
	{
		return PBE::RESULT_BUFFER_TOO_SMALL;
	}

	for (int propertyIndex = 0; propertyIndex < numProperties; ++propertyIndex)
	{
		std::string key;
		if (reader.ReadString(key) != PBE::RESULT_SUCCESS)
		{
			return PBE::RESULT_BUFFER_TOO_SMALL;
		}

		PropertyType type;
		if (reader.Read(type) != PBE::RESULT_SUCCESS)
		{
			return PBE::RESULT_BUFFER_TOO_SMALL;
		}

		switch (type)
		{
			case PropertyType::INT32:
			{
				int intValue;
				if (reader.Read(intValue) != PBE::RESULT_SUCCESS)
				{
					return PBE::RESULT_BUFFER_TOO_SMALL;
				}
				AddProperty(key, intValue);
			}
			break;
			case PropertyType::FLOAT:
			{
				float floatValue;
				if (reader.Read(floatValue) != PBE::RESULT_SUCCESS)
				{
					return PBE::RESULT_BUFFER_TOO_SMALL;
				}
				AddProperty(key, floatValue);
			}
			break;
			case PropertyType::STRING:
			{
				std::string stringValue;
				if (reader.ReadString(stringValue) != PBE::RESULT_SUCCESS)
				{
					return PBE::RESULT_BUFFER_TOO_SMALL;
				}
				AddProperty(key, stringValue);
			}
			break;
			case PropertyType::VEC2:
			{
				Vec2 vec2;
				if (reader.Read(vec2) != PBE::RESULT_SUCCESS)
				{
					return PBE::RESULT_BUFFER_TOO_SMALL;
				}
				AddProperty(key, vec2);
			}
			break;
			case PropertyType::VEC3:
			{
				Vec3 vec3;
				if (reader.Read(vec3) != PBE::RESULT_SUCCESS)
				{
					return PBE::RESULT_BUFFER_TOO_SMALL;
				}
				AddProperty(key, vec3);
			}
			break;
			case PropertyType::VEC4:
			{
				Vec4 vec4;
				if (reader.Read(vec4) != PBE::RESULT_SUCCESS)
				{
					return PBE::RESULT_BUFFER_TOO_SMALL;
				}
				AddProperty(key, vec4);
			}
			break;
			case PropertyType::EULER_ANGLES:
			{
				EulerAngles eulerAngles;
				if (reader.Read(eulerAngles) != PBE::RESULT_SUCCESS)
				{
					return PBE::RESULT_BUFFER_TOO_SMALL;
				}
				AddProperty(key, eulerAngles);
			}
			break;
			return PBE::RESULT_SUCCESS;
		}
	}

	return PBE::RESULT_SUCCESS;
}

std::string PBE::NamedProperties::GetJSON() const
{
	using nlohmann::json;
	json j;

	for (auto& propertyPair : m_Properties)
	{
		HCIString const& keyHCI = propertyPair.first;
		std::string key = keyHCI.GetCasedString();
		for (char& c : key)
		{
			c = (char)std::tolower(c);
		}

		Property const& value = propertyPair.second;

		if (std::holds_alternative<int>(value))
		{
			j[key] = std::get<int>(value);
		}
		else if (std::holds_alternative<float>(value))
		{
			j[key] = std::get<float>(value);
		}
		else if (std::holds_alternative<std::string>(value))
		{
			j[key] = std::get<std::string>(value);
		}
		else if (std::holds_alternative<Vec2>(value))
		{
			Vec2 vec = std::get<Vec2>(value);
			j[key] = { {"type", "vec2"}, {"x", vec.x}, {"y", vec.y} };
		}
		else if (std::holds_alternative<Vec3>(value))
		{
			Vec3 vec = std::get<Vec3>(value);
			j[key] = { {"type", "vec3"}, {"x", vec.x}, {"y", vec.y}, {"z", vec.z} };
		}
		else if (std::holds_alternative<Vec4>(value))
		{
			Vec4 vec = std::get<Vec4>(value);
			j[key] = { {"type", "vec4"}, {"x", vec.x}, {"y", vec.y}, {"z", vec.z}, {"w", vec.w} };
		}
		else if (std::holds_alternative<EulerAngles>(value))
		{
			EulerAngles eulerAngles = std::get<EulerAngles>(value);
			j[key] = { {"type", "eulerAngles"}, {"yaw", eulerAngles.m_Yaw}, {"pitch", eulerAngles.m_Pitch}, {"roll", eulerAngles.m_Roll} };
		}

		return j;
	}
}
PBE::Result PBE::NamedProperties::LoadFromJSONString(std::string_view string)
{
	using nlohmann::json;
	json j = json::parse(string);
	for (json::iterator it = j.begin(); it != j.end(); ++it)
	{
		std::string key = it.key();
		json value = it.value();
		if (value.is_number_integer())
		{
			AddProperty(key, value.get<int>());
		}
		else if (value.is_number_float())
		{
			AddProperty(key, value.get<float>());
		}
		else if (value.is_string())
		{
			AddProperty(key, value.get<std::string>());
		}
		else if (value.is_structured())
		{
			std::string type = value["type"];
			if (type == "vec2")
			{
				AddProperty(key, Vec2(value["x"], value["y"]));
			}
			else if (type == "vec3")
			{
				AddProperty(key, Vec3(value["x"], value["y"], value["z"]));
			}
		}
	}

	return PBE::RESULT_SUCCESS;
}