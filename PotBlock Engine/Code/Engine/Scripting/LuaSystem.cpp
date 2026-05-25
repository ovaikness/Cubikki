#include "Engine/Scripting/LuaSystem.hpp"

#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/OBB3.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/IntVec3.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Vec4.hpp"
#include "Engine/Math/Mat4.hpp"
#include "Engine/Math/Rotor3D.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Math/Rotor3D.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Scripting/LuaWrapper.hpp"
#include "Engine/Input/InputSystem.hpp"

#include "Engine/Graphics/Renderer.hpp"
#include "Engine/Graphics/Camera.hpp"
#include "Engine/Graphics/UI/Canvas.hpp"
#include "Engine/Graphics/UI/Widget.hpp"
#include "Engine/Graphics/UI/Panel.hpp"
#include "Engine/Graphics/UI/TextEntryPanel.hpp"

#include <sol/sol.hpp>
#include <nlohmann/json.hpp>
#include <iostream>
#include <fstream>
#include <unordered_map>

PBE::LuaSystem::LuaSystem(PBE::LuaSystemCreateInfo const& info)
	: m_Info( info )
	, m_ScriptPath( m_Info.m_ScriptPath )
{
	m_Lua.open_libraries(sol::lib::base, sol::lib::package, sol::lib::string, sol::lib::table, sol::lib::math, sol::lib::os, sol::lib::io);
	RegisterUserTypes();
	ReloadAllScripts();
}

sol::protected_function const& PBE::LuaSystem::GetScript(ScriptID id)
{
	return m_LoadedScripts[id];
}

PBE::EventDispatcher<>& PBE::LuaSystem::GetScriptReloadDispatcher(ScriptID id)
{
	return m_ScriptReloadDispatchers[id];
}

PBE::ScriptID PBE::LuaSystem::GetScriptID(std::string_view scriptName)
{
	auto itr = m_ScriptIDs.find(scriptName.data());
	if (itr != m_ScriptIDs.end())
	{
		return itr->second;
	}

	return ~0ui32;
}

void PBE::LuaSystem::ReloadScript(std::string_view scriptName)
{
	auto itr = m_ScriptIDs.find(scriptName.data());
	if (itr != m_ScriptIDs.end())
	{
		ScriptID id = itr->second;
		auto result = m_Lua.script_file(scriptName.data());
		if (!result.valid())
		{
			sol::error err = result;
			std::string what = err.what();
			std::cout << what << std::endl;
			return;
		}

		m_LoadedScripts[id] = result;
	}
	else
	{
		std::filesystem::path scriptPath = m_ScriptPath / scriptName.data();

		if (!std::filesystem::exists(scriptPath))
		{
			std::cout << "Script does not exist" << std::endl;
			return;
		}

		if (scriptPath.extension() != ".lua")
		{
			std::cout << "Script is not a lua file" << std::endl;
			return;
		}

		auto result = m_Lua.script_file(scriptPath.string());
		if (!result.valid())
		{
			sol::error err = result;
			std::string what = err.what();
			std::cout << what << std::endl;
			return;
		}
		m_ScriptIDs[scriptPath.filename().string()] = m_ScriptIDCounter;
		m_LoadedScripts.push_back(result);
		m_ScriptReloadDispatchers.push_back(EventDispatcher<>());
		m_ScriptIDCounter++;
	}
}

void PBE::LuaSystem::ReloadAllScripts()
{
	m_ScriptIDCounter = 0;
	m_ScriptIDs.clear();
	m_LoadedScripts.clear();
	std::vector<std::filesystem::path> luaPaths;

	for (const auto& entry : std::filesystem::directory_iterator(m_ScriptPath))
	{
		std::filesystem::path path = entry.path();
		luaPaths.push_back(path);
	}

	for (std::filesystem::path scriptPath : luaPaths)
	{
		if (scriptPath.extension() != ".lua")
		{
			continue;
		}

		sol::protected_function func = m_Lua.load_file(scriptPath.string());
		if (!func.valid())
		{
			std::cout << "Failed to load script: " << scriptPath << std::endl;
		}

		m_ScriptIDs[scriptPath.filename().string()] = m_ScriptIDCounter;
		m_LoadedScripts.push_back(func);
		m_ScriptReloadDispatchers.push_back(EventDispatcher<>());
		m_ScriptIDCounter++;
	}
}

void PBE::LuaSystem::RegisterUserTypes()
{
	m_Lua.new_usertype<IntVec2>(
		"IntVec2", sol::constructors<IntVec2(), IntVec2(int, int), IntVec2(IntVec2 const&)>(),
		"x", &IntVec2::x,
		"y", &IntVec2::y,
		sol::meta_function::addition, [](IntVec2& a, IntVec2& b) -> IntVec2 { return a + b; },
		sol::meta_function::subtraction, [](IntVec2& a, IntVec2& b) -> IntVec2 { return a - b; },
		//sol::meta_function::multiplication, [](IntVec2& a, int& b) -> IntVec2 { return a * b; },
		//sol::meta_function::division, [](IntVec2& a, int& b) -> IntVec2 { return a / b; },
		"GetLengthSquared", &IntVec2::GetLengthSquared,
		"GetRotated90Degrees", &IntVec2::GetRotated90Degrees,
		"GetRotatedMinus90Degrees", &IntVec2::GetRotatedMinus90Degrees,
		"GetFlipped", &IntVec2::GetFlipped
	);

	m_Lua.new_usertype<IntVec3>(
		"IntVec3", sol::constructors<IntVec3(), IntVec3(int, int, int), IntVec3(IntVec3 const&)>(),
		"x", &IntVec3::x,
		"y", &IntVec3::y,
		"z", &IntVec3::z,
		sol::meta_function::addition, [](IntVec3& a, IntVec3& b) -> IntVec3 { return a + b; },
		sol::meta_function::subtraction, [](IntVec3& a, IntVec3& b) -> IntVec3 { return a - b; },
		//sol::meta_function::multiplication, [](IntVec3& a, int& b) -> IntVec3 { return a * b; },
		//sol::meta_function::division, [](IntVec3& a, int& b) -> IntVec3 { return a / b; },
		"GetLengthSquared", &IntVec3::GetLengthSquared
	);

	m_Lua.new_usertype<Vec2>(
		"Vec2", sol::constructors<Vec2(), Vec2(float, float), Vec2(Vec2 const&), Vec2(IntVec2 const&)>(),
		"x", &Vec2::x,
		"y", &Vec2::y,
		sol::meta_function::addition, [](Vec2& a, Vec2& b) -> Vec2 { return a + b; },
		sol::meta_function::subtraction, [](Vec2& a, Vec2& b) -> Vec2 { return a - b; },
		//sol::meta_function::multiplication, [](Vec2& a, float& b) -> Vec2 { return a * b; },
		//sol::meta_function::division, [](Vec2& a, float& b) -> Vec2 { return a / b; },
		"GetLength", &Vec2::GetLength,
		"GetLengthSquared", &Vec2::GetLengthSquared,
		"GetAngle", &Vec2::GetAngle,
		"GetAngleDegrees", &Vec2::GetAngleDegrees,
		"Normalize", &Vec2::Normalize,
		"Rotate", &Vec2::Rotate,
		"RotateAroundPivot", &Vec2::RotateAroundPivot,
		"Reflect", &Vec2::Reflect,
		"GetNormalized", &Vec2::GetNormalized,
		"GetRotated", &Vec2::GetRotated,
		"GetDistance", &Vec2::GetDistance,
		"GetDistanceSquared", &Vec2::GetDistanceSquared,
		"Dot", &Vec2::Dot,
		"Cross", &Vec2::Cross,
		"GetAngleBetweenDegrees", &Vec2::GetAngleBetweenDegrees,
		"GetRotated90Degrees", &Vec2::GetRotated90Degrees,
		"GetRotatedMinus90Degrees", &Vec2::GetRotatedMinus90Degrees,
		"GetFlipped", &Vec2::GetFlipped,
		"Rotate90Degrees", &Vec2::Rotate90Degrees,
		"RotateMinus90Degrees", &Vec2::RotateMinus90Degrees,
		"Flip", &Vec2::Flip,
		"Negate", &Vec2::Negate
	);

	m_Lua.new_usertype<Vec3>(
		"Vec3", sol::constructors<Vec3(), Vec3(float, float, float), Vec3(Vec2 const&, float), Vec3(float, Vec2 const&), Vec3(IntVec3 const&), Vec3(Vec3 const&)>(),
		"x", &Vec3::x,
		"y", &Vec3::y,
		"z", &Vec3::z,
		sol::meta_function::addition, [](Vec3& a, Vec3& b) -> Vec3 { return a + b; },
		sol::meta_function::subtraction, [](Vec3& a, Vec3& b) -> Vec3 { return a - b; },
		//sol::meta_function::multiplication, [](Vec3& a, float& b) -> Vec3 { return a * b; },
		//sol::meta_function::division, [](Vec3& a, float& b) -> Vec3 { return a / b; },
		"GetLength", &Vec3::GetLength,
		"GetLengthSquared", &Vec3::GetLengthSquared,
		"GetAngle", &Vec3::GetAngle,
		"Normalize", &Vec3::Normalize,
		"Reflect", &Vec3::Reflect,
		"Project", &Vec3::Project,
		"Negate", &Vec3::Negate,
		"GetNormalized", &Vec3::GetNormalized,
		"GetSign", &Vec3::GetSign,
		"GetReciprocal", &Vec3::GetReciprocal,
		"GetDistance", &Vec3::GetDistance,
		"GetDistanceSquared", &Vec3::GetDistanceSquared,
		"Dot", &Vec3::Dot,
		"Cross", &Vec3::Cross,
		"GetAngleBetweenDegrees", &Vec3::GetAngleBetweenDegrees
	);

	m_Lua.new_usertype<Vec4>(
		"Vec4", sol::constructors<Vec4(), Vec4(float, float, float, float), Vec4(Vec3 const&, float), Vec4(float, Vec3 const&), Vec4(Vec2 const&, Vec2 const&), Vec4(Vec4 const&)>(),
		"x", &Vec4::x,
		"y", &Vec4::y,
		"z", &Vec4::z,
		"w", &Vec4::w,
		sol::meta_function::addition, [](Vec4& a, Vec4& b) -> Vec4 { return a + b; },
		sol::meta_function::subtraction, [](Vec4& a, Vec4& b) -> Vec4 { return a - b; }
		//sol::meta_function::multiplication, [](Vec4& a, float& b) -> Vec4 { return a * b; },
		//sol::meta_function::division, [](Vec4& a, float& b) -> Vec4 { return a / b; }
	);

	m_Lua.new_usertype<AABB2>(
		"AABB2", sol::constructors<AABB2(), AABB2(float, float, float, float), AABB2(Vec2 const&, Vec2 const&), AABB2(AABB2 const&)>(),
		"mins", &AABB2::m_Mins,
		"maxs", &AABB2::m_Maxs,
		"GetCenter", &AABB2::GetCenter,
		"GetDimensions", &AABB2::GetDimensions,
		"IsPointInside", [](AABB2& aabb, Vec2& point) { return aabb.IsPointInside(point); },
		"Intersects", [](AABB2& aabb, AABB2& other) { return aabb.Intersects(other); },
		"GetWidth", &AABB2::GetWidth,
		"GetHeight", &AABB2::GetHeight,
		"Translate", &AABB2::Translate
	);

	m_Lua.new_usertype<AABB3>(
		"AABB3", sol::constructors<AABB3(), AABB3(Vec3 const&, Vec3 const&), AABB3(AABB3 const&)>(),
		"mins", &AABB3::m_Mins,
		"maxs", &AABB3::m_Maxs,
		"GetCenter", &AABB3::GetCenter,
		"GetDimensions", &AABB3::GetDimensions,
		"IsPointInside", [](AABB3& aabb, Vec3& point) { return aabb.IsPointInside(point); },
		"Intersects", [](AABB3& aabb, AABB3& other) { return aabb.Intersects(other); },
		"GetWidth", &AABB3::GetWidth,
		"GetHeight", &AABB3::GetHeight,
		"GetDepth", &AABB3::GetDepth,
		"Translate", &AABB3::Translate
	);

	m_Lua.new_usertype<CollisionResult3D>(
			"CollisionResult3D", sol::constructors<CollisionResult3D()>(),
			"hit", &CollisionResult3D::m_Hit,
			"hitPosition", &CollisionResult3D::m_HitPosition,
			"hitSurfaceNormal", &CollisionResult3D::m_HitSurfaceNormal
		);

	m_Lua.new_usertype<OBB3>(
		"OBB3", sol::constructors<OBB3(), OBB3(Vec3 const&, float, float, float, EulerAngles), OBB3 (Vec3 const&, float, float, float, Vec3, Vec3, Vec3), OBB3(OBB3 const&)>(),
		"center", &OBB3::m_Center,
		"extents", &OBB3::m_Extents,
		"IBasis", &OBB3::m_IBasis,
		"JBasis", &OBB3::m_JBasis,
		"KBasis", &OBB3::m_KBasis
	);
		

	m_Lua.new_usertype<Mat4>(
		"Mat4", sol::constructors<Mat4(), Mat4(float, float, float, float, float, float, float, float, float, float, float, float, float, float, float, float), Mat4(Mat4 const&)>(),
		"values", sol::property([](Mat4& mat, size_t index) { return mat.m_Values[index]; }),
		"Ix", sol::property([](Mat4& mat) { return mat.m_Values[Mat4::Ix];  }),
		"Iy", sol::property([](Mat4& mat) { return mat.m_Values[Mat4::Iy];  }),
		"Iz", sol::property([](Mat4& mat) { return mat.m_Values[Mat4::Iz];  }),
		"Iw", sol::property([](Mat4& mat) { return mat.m_Values[Mat4::Iw];  }),
		"Jx", sol::property([](Mat4& mat) { return mat.m_Values[Mat4::Jx];  }),
		"Jy", sol::property([](Mat4& mat) { return mat.m_Values[Mat4::Jy];  }),
		"Jz", sol::property([](Mat4& mat) { return mat.m_Values[Mat4::Jz];  }),
		"Jw", sol::property([](Mat4& mat) { return mat.m_Values[Mat4::Jw];  }),
		"Kx", sol::property([](Mat4& mat) { return mat.m_Values[Mat4::Kx];  }),
		"Ky", sol::property([](Mat4& mat) { return mat.m_Values[Mat4::Ky];  }),
		"Kz", sol::property([](Mat4& mat) { return mat.m_Values[Mat4::Kz];  }),
		"Kw", sol::property([](Mat4& mat) { return mat.m_Values[Mat4::Kw];  }),
		"Tx", sol::property([](Mat4& mat) { return mat.m_Values[Mat4::Tx];  }),
		"Ty", sol::property([](Mat4& mat) { return mat.m_Values[Mat4::Ty];  }),
		"Tz", sol::property([](Mat4& mat) { return mat.m_Values[Mat4::Tz];  }),
		"Tw", sol::property([](Mat4& mat) { return mat.m_Values[Mat4::Tw];  }),
		"GetAppended", &Mat4::GetAppended,
		"Append", &Mat4::Append,
		"GetOrthonormalized", &Mat4::GetOrthonormalized,
		"GetTransposed", &Mat4::GetTransposed,
		"GetOrthonormalInverse", &Mat4::GetOrthonormalInverse,
		"GetDeterminant", &Mat4::GetDeterminant,
		"GetInverse", &Mat4::GetInverse,
		"TransformPoint2D", [](Mat4& mat, Vec2& point) { return mat.TransformPoint(point); },
		"TransformPoint3D", [](Mat4& mat, Vec3& point) { return mat.TransformPoint(point); },
		"TransformPoint4D", [](Mat4& mat, Vec4& point) { return mat.TransformPoint(point); },
		"CreateLookAt", & Mat4::CreateLookAt,
		"CreateTranslation3D", & Mat4::CreateTranslation3D,
		"CreateOrthographicProjection", & Mat4::CreateOrthographicProjection,
		"CreatePerspectiveProjection", & Mat4::CreatePerspectiveProjection,
		"CreateScale", [](float scale) { return Mat4::CreateScale(scale); },
		"CreateScale3D", [](Vec3& scale) { return Mat4::CreateScale(scale); },
		"Interpolate", & Mat4::Interpolate,
		"CreateTRS", [](Vec3& translation, Rotor3D& rotation, Vec3& scale) { return Mat4::CreateTRS(translation, rotation, scale); },
		"CreateTRS2", [](Vec3& translation, EulerAngles& rotation, Vec3& scale) { return Mat4::CreateTRS(translation, rotation, scale); },
		"Translate3D", & Mat4::Translate3D,
		"IDENTITY", sol::var(Mat4::IDENTITY),
		"ZERO", sol::var(Mat4::ZERO)
	);

	m_Lua.new_usertype<Rotor3D>(
		"Rotor3D", sol::constructors<Rotor3D(), Rotor3D(float, float, float, float), Rotor3D(Vec3 const&, float), Rotor3D(Rotor3D const&)>(),
		"i", &Rotor3D::i,
		"j", &Rotor3D::j,
		"k", &Rotor3D::k,
		"w", &Rotor3D::w,
		"GetInverse", &Rotor3D::GetInverse,
		"GetNormalized", &Rotor3D::GetNormalized,
		sol::meta_function::multiplication, [](Rotor3D& a, Rotor3D& b) -> Rotor3D { return a * b; },
		sol::meta_function::addition, [](Rotor3D& a, Rotor3D& b) -> Rotor3D { return a + b; },
		sol::meta_function::subtraction, [](Rotor3D& a, Rotor3D& b) -> Rotor3D { return a - b; }
	);

	m_Lua.new_usertype<LuaWrapper<int>>(
		"LuaInt",
		"get", &LuaWrapper<int>::get,
		"set", &LuaWrapper<int>::set
	);

	m_Lua.new_usertype<LuaWrapper<float>>(
		"LuaFloat",
		"get", &LuaWrapper<float>::get,
		"set", &LuaWrapper<float>::set
	);

	m_Lua.new_usertype<LuaWrapper<bool>>(
		"LuaBool",
		"get", &LuaWrapper<bool>::get,
		"set", &LuaWrapper<bool>::set
	);

	m_Lua.new_usertype<LuaWrapper<Vec2>>(
		"LuaVec2",
		"get", &LuaWrapper<Vec2>::get,
		"set", &LuaWrapper<Vec2>::set
	);

	m_Lua.new_usertype<LuaWrapper<Vec3>>(
		"LuaVec3",
		"get", &LuaWrapper<Vec3>::get,
		"set", &LuaWrapper<Vec3>::set
	);

	m_Lua.new_usertype<LuaWrapper<Vec4>>(
		"LuaVec4",
		"get", &LuaWrapper<Vec4>::get,
		"set", &LuaWrapper<Vec4>::set
	);

	m_Lua.new_usertype<LuaWrapper<Mat4>>(
		"LuaMat4",
		"get", &LuaWrapper<Mat4>::get,
		"set", &LuaWrapper<Mat4>::set
	);

	m_Lua.new_usertype<LuaWrapper<AABB2>>(
		"LuaAABB2",
		"get", &LuaWrapper<AABB2>::get,
		"set", &LuaWrapper<AABB2>::set
	);

	m_Lua.new_usertype<LuaWrapper<AABB3>>(
		"LuaAABB3",
		"get", &LuaWrapper<AABB3>::get,
		"set", &LuaWrapper<AABB3>::set
	);

	m_Lua.new_usertype<LuaWrapper<Rotor3D>>(
		"LuaRotor3D",
		"get", &LuaWrapper<Rotor3D>::get,
		"set", &LuaWrapper<Rotor3D>::set
	);

	m_Lua.new_usertype<LuaWrapper<EulerAngles>>(
		"LuaEulerAngles",
		"get", &LuaWrapper<EulerAngles>::get,
		"set", &LuaWrapper<EulerAngles>::set
	);

	m_Lua.new_enum(
		"Key",
		"Space", PBE::KEY_SPACE,
		"LeftShift", PBE::KEY_LEFT_SHIFT,
		"RightShift", PBE::KEY_RIGHT_SHIFT,
		"LeftControl", PBE::KEY_LEFT_CONTROL,
		"RightControl", PBE::KEY_RIGHT_CONTROL,
		"LeftAlt", PBE::KEY_LEFT_ALT,
		"RightAlt", PBE::KEY_RIGHT_ALT,
		"LeftSuper", PBE::KEY_LEFT_SUPER,
		"RightSuper", PBE::KEY_RIGHT_SUPER,
		"Backspace", PBE::KEY_BACKSPACE,
		"Tab", PBE::KEY_TAB,
		"Enter", PBE::KEY_ENTER,
		"Escape", PBE::KEY_ESCAPE,
		"Insert", PBE::KEY_INSERT,
		"Delete", PBE::KEY_DELETE,
		"Right", PBE::KEY_RIGHT,
		"Left", PBE::KEY_LEFT,
		"Down", PBE::KEY_DOWN,
		"Up", PBE::KEY_UP,
		"PageUp", PBE::KEY_PAGE_UP,
		"PageDown", PBE::KEY_PAGE_DOWN,
		"Home", PBE::KEY_HOME,
		"End", PBE::KEY_END,
		"PrintScreen", PBE::KEY_PRINT_SCREEN,
		"Pause", PBE::KEY_PAUSE,
		"CapsLock", PBE::KEY_CAPS_LOCK,
		"ScrollLock", PBE::KEY_SCROLL_LOCK,
		"NumLock", PBE::KEY_NUM_LOCK,
		"Menu", PBE::KEY_MENU,
		"LeftBracket", PBE::KEY_LEFT_BRACKET,
		"RightBracket", PBE::KEY_RIGHT_BRACKET,
		"Semicolon", PBE::KEY_SEMICOLON,
		"Comma", PBE::KEY_COMMA,
		"Period", PBE::KEY_PERIOD,
		"Apostrophe", PBE::KEY_APOSTROPHE,
		"Slash", PBE::KEY_SLASH,
		"Backslash", PBE::KEY_BACKSLASH,
		"Equal", PBE::KEY_EQUAL,
		"Minus", PBE::KEY_MINUS,
		"Keypad0", PBE::KEY_KP_0,
		"Keypad1", PBE::KEY_KP_1,
		"Keypad2", PBE::KEY_KP_2,
		"Keypad3", PBE::KEY_KP_3,
		"Keypad4", PBE::KEY_KP_4,
		"Keypad5", PBE::KEY_KP_5,
		"Keypad6", PBE::KEY_KP_6,
		"Keypad7", PBE::KEY_KP_7,
		"Keypad8", PBE::KEY_KP_8,
		"Keypad9", PBE::KEY_KP_9,
		"KeypadDecimal", PBE::KEY_KP_DECIMAL,
		"KeypadDivide", PBE::KEY_KP_DIVIDE,
		"KeypadMultiply", PBE::KEY_KP_MULTIPLY,
		"KeypadSubtract", PBE::KEY_KP_SUBTRACT,
		"KeypadAdd", PBE::KEY_KP_ADD,
		"KeypadEnter", PBE::KEY_KP_ENTER,
		"KeypadEqual", PBE::KEY_KP_EQUAL,
		"LeftShift", PBE::KEY_LEFT_SHIFT,
		"LeftControl", PBE::KEY_LEFT_CONTROL,
		"LeftAlt", PBE::KEY_LEFT_ALT,
		"LeftSuper", PBE::KEY_LEFT_SUPER,
		"RightShift", PBE::KEY_RIGHT_SHIFT,
		"RightControl", PBE::KEY_RIGHT_CONTROL,
		"RightAlt", PBE::KEY_RIGHT_ALT,
		"RightSuper", PBE::KEY_RIGHT_SUPER,
		"Menu", PBE::KEY_MENU,
		"Q", PBE::KEY_Q,
		"W", PBE::KEY_W,
		"E", PBE::KEY_E,
		"R", PBE::KEY_R,
		"T", PBE::KEY_T,
		"Y", PBE::KEY_Y,
		"U", PBE::KEY_U,
		"I", PBE::KEY_I,
		"O", PBE::KEY_O,
		"P", PBE::KEY_P,
		"A", PBE::KEY_A,
		"S", PBE::KEY_S,
		"D", PBE::KEY_D,
		"F", PBE::KEY_F,
		"G", PBE::KEY_G,
		"H", PBE::KEY_H,
		"J", PBE::KEY_J,
		"K", PBE::KEY_K,
		"L", PBE::KEY_L,
		"Z", PBE::KEY_Z,
		"X", PBE::KEY_X,
		"C", PBE::KEY_C,
		"V", PBE::KEY_V,
		"B", PBE::KEY_B,
		"N", PBE::KEY_N,
		"M", PBE::KEY_M,
		"Num0", PBE::KEY_0,
		"Num1", PBE::KEY_1,
		"Num2", PBE::KEY_2,
		"Num3", PBE::KEY_3,
		"Num4", PBE::KEY_4,
		"Num5", PBE::KEY_5,
		"Num6", PBE::KEY_6,
		"Num7", PBE::KEY_7,
		"Num8", PBE::KEY_8,
		"Num9", PBE::KEY_9
	);

	m_Lua.new_usertype<Widget>(
		"Widget", sol::no_constructor,
		"SetFocused", &Widget::SetFocused,
		"SetHovered", &Widget::SetHovered,
		"MarkDirty", &Widget::MarkDirty,
		"GetBounds", &Widget::GetBounds,
		"GetAxisAlignedBounds", &Widget::GetAxisAlignedBounds,
		"GetGlobalTransform", &Widget::GetGlobalTransform,
		"GetLocalTransform", &Widget::GetLocalTransform
	);

	m_Lua.new_usertype<Canvas>(
		"Canvas", sol::no_constructor,
		"AddChild", &Canvas::AddChild,
		"RemoveChild", &Canvas::RemoveChild
	);
}

void PBE::LuaSystem::RegisterLuaStateFunctions()
{
	m_Lua.set_function("ReloadScript", &LuaSystem::ReloadScript, this);
	m_Lua.set_function("ReloadAllScripts", &LuaSystem::ReloadAllScripts, this);
	m_Lua.set_function("GetScriptID", &LuaSystem::GetScriptID, this);
	m_Lua.set_function("GetScript", &LuaSystem::GetScript, this);

	m_Lua.new_usertype<nlohmann::json>("Json",
		"parse", [](std::string json) { return nlohmann::json::parse(json); },
		"stringify", [](nlohmann::json json) { return json.dump(); }
	);
}

PBE::LuaSystem::~LuaSystem()
{

}

sol::environment PBE::LuaSystem::CreateEnvironment()
{
	sol::environment env(m_Lua, sol::create, m_Lua.globals());
	return env;
}