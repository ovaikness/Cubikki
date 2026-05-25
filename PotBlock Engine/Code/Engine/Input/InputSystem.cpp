#include "Engine/Input/InputSystem.hpp"
#include "Engine/Graphics/Window.hpp"

namespace PBE
{
	std::map<int, int> GLFWKeytoPBEKey;

	void InitializeKeyMaps()
	{
		GLFWKeytoPBEKey[GLFW_KEY_LEFT] = PBE::KEY_LEFT;
		GLFWKeytoPBEKey[GLFW_KEY_RIGHT] = PBE::KEY_RIGHT;
		GLFWKeytoPBEKey[GLFW_KEY_SPACE] = PBE::KEY_SPACE;
		GLFWKeytoPBEKey[GLFW_KEY_ENTER] = PBE::KEY_ENTER;
		GLFWKeytoPBEKey[GLFW_KEY_ESCAPE] = PBE::KEY_ESCAPE;
		GLFWKeytoPBEKey[GLFW_KEY_TAB] = PBE::KEY_TAB;
		GLFWKeytoPBEKey[GLFW_KEY_BACKSPACE] = PBE::KEY_BACKSPACE;
		GLFWKeytoPBEKey[GLFW_KEY_DELETE] = PBE::KEY_DELETE;
		GLFWKeytoPBEKey[GLFW_KEY_INSERT] = PBE::KEY_INSERT;
		GLFWKeytoPBEKey[GLFW_KEY_HOME] = PBE::KEY_HOME;
		GLFWKeytoPBEKey[GLFW_KEY_END] = PBE::KEY_END;
		GLFWKeytoPBEKey[GLFW_KEY_PAGE_UP] = PBE::KEY_PAGE_UP;
		GLFWKeytoPBEKey[GLFW_KEY_PAGE_DOWN] = PBE::KEY_PAGE_DOWN;
		GLFWKeytoPBEKey[GLFW_KEY_CAPS_LOCK] = PBE::KEY_CAPS_LOCK;
		GLFWKeytoPBEKey[GLFW_KEY_SCROLL_LOCK] = PBE::KEY_SCROLL_LOCK;
		GLFWKeytoPBEKey[GLFW_KEY_NUM_LOCK] = PBE::KEY_NUM_LOCK;
		GLFWKeytoPBEKey[GLFW_KEY_PRINT_SCREEN] = PBE::KEY_PRINT_SCREEN;
		GLFWKeytoPBEKey[GLFW_KEY_PAUSE] = PBE::KEY_PAUSE;
		GLFWKeytoPBEKey[GLFW_KEY_F1] = PBE::KEY_F1;
		GLFWKeytoPBEKey[GLFW_KEY_F2] = PBE::KEY_F2;
		GLFWKeytoPBEKey[GLFW_KEY_F3] = PBE::KEY_F3;
		GLFWKeytoPBEKey[GLFW_KEY_F4] = PBE::KEY_F4;
		GLFWKeytoPBEKey[GLFW_KEY_F5] = PBE::KEY_F5;
		GLFWKeytoPBEKey[GLFW_KEY_F6] = PBE::KEY_F6;
		GLFWKeytoPBEKey[GLFW_KEY_F7] = PBE::KEY_F7;
		GLFWKeytoPBEKey[GLFW_KEY_F8] = PBE::KEY_F8;
		GLFWKeytoPBEKey[GLFW_KEY_F9] = PBE::KEY_F9;
		GLFWKeytoPBEKey[GLFW_KEY_F10] = PBE::KEY_F10;
		GLFWKeytoPBEKey[GLFW_KEY_F11] = PBE::KEY_F11;
		GLFWKeytoPBEKey[GLFW_KEY_F12] = PBE::KEY_F12;
		GLFWKeytoPBEKey[GLFW_KEY_A] = PBE::KEY_A;
		GLFWKeytoPBEKey[GLFW_KEY_B] = PBE::KEY_B;
		GLFWKeytoPBEKey[GLFW_KEY_C] = PBE::KEY_C;
		GLFWKeytoPBEKey[GLFW_KEY_D] = PBE::KEY_D;
		GLFWKeytoPBEKey[GLFW_KEY_E] = PBE::KEY_E;
		GLFWKeytoPBEKey[GLFW_KEY_F] = PBE::KEY_F;
		GLFWKeytoPBEKey[GLFW_KEY_G] = PBE::KEY_G;
		GLFWKeytoPBEKey[GLFW_KEY_H] = PBE::KEY_H;
		GLFWKeytoPBEKey[GLFW_KEY_I] = PBE::KEY_I;
		GLFWKeytoPBEKey[GLFW_KEY_J] = PBE::KEY_J;
		GLFWKeytoPBEKey[GLFW_KEY_K] = PBE::KEY_K;
		GLFWKeytoPBEKey[GLFW_KEY_L] = PBE::KEY_L;
		GLFWKeytoPBEKey[GLFW_KEY_M] = PBE::KEY_M;
		GLFWKeytoPBEKey[GLFW_KEY_N] = PBE::KEY_N;
		GLFWKeytoPBEKey[GLFW_KEY_O] = PBE::KEY_O;
		GLFWKeytoPBEKey[GLFW_KEY_P] = PBE::KEY_P;
		GLFWKeytoPBEKey[GLFW_KEY_Q] = PBE::KEY_Q;
		GLFWKeytoPBEKey[GLFW_KEY_R] = PBE::KEY_R;
		GLFWKeytoPBEKey[GLFW_KEY_S] = PBE::KEY_S;
		GLFWKeytoPBEKey[GLFW_KEY_T] = PBE::KEY_T;
		GLFWKeytoPBEKey[GLFW_KEY_U] = PBE::KEY_U;
		GLFWKeytoPBEKey[GLFW_KEY_V] = PBE::KEY_V;
		GLFWKeytoPBEKey[GLFW_KEY_W] = PBE::KEY_W;
		GLFWKeytoPBEKey[GLFW_KEY_X] = PBE::KEY_X;
		GLFWKeytoPBEKey[GLFW_KEY_Y] = PBE::KEY_Y;
		GLFWKeytoPBEKey[GLFW_KEY_Z] = PBE::KEY_Z;
		GLFWKeytoPBEKey[GLFW_KEY_1] = PBE::KEY_1;
		GLFWKeytoPBEKey[GLFW_KEY_2] = PBE::KEY_2;
		GLFWKeytoPBEKey[GLFW_KEY_3] = PBE::KEY_3;
		GLFWKeytoPBEKey[GLFW_KEY_4] = PBE::KEY_4;
		GLFWKeytoPBEKey[GLFW_KEY_5] = PBE::KEY_5;
		GLFWKeytoPBEKey[GLFW_KEY_6] = PBE::KEY_6;
		GLFWKeytoPBEKey[GLFW_KEY_7] = PBE::KEY_7;
		GLFWKeytoPBEKey[GLFW_KEY_8] = PBE::KEY_8;
		GLFWKeytoPBEKey[GLFW_KEY_9] = PBE::KEY_9;
		GLFWKeytoPBEKey[GLFW_KEY_0] = PBE::KEY_0;
		GLFWKeytoPBEKey[GLFW_KEY_ESCAPE] = PBE::KEY_ESCAPE;
		GLFWKeytoPBEKey[GLFW_KEY_BACKSPACE] = PBE::KEY_BACKSPACE;
		GLFWKeytoPBEKey[GLFW_KEY_TAB] = PBE::KEY_TAB;
		GLFWKeytoPBEKey[GLFW_KEY_SPACE] = PBE::KEY_SPACE;
		GLFWKeytoPBEKey[GLFW_KEY_MINUS] = PBE::KEY_MINUS;
		GLFWKeytoPBEKey[GLFW_KEY_EQUAL] = PBE::KEY_EQUAL;
		GLFWKeytoPBEKey[GLFW_KEY_LEFT_BRACKET] = PBE::KEY_LEFT_BRACKET;
		GLFWKeytoPBEKey[GLFW_KEY_RIGHT_BRACKET] = PBE::KEY_RIGHT_BRACKET;
		GLFWKeytoPBEKey[GLFW_KEY_BACKSLASH] = PBE::KEY_BACKSLASH;
		GLFWKeytoPBEKey[GLFW_KEY_SEMICOLON] = PBE::KEY_SEMICOLON;
		GLFWKeytoPBEKey[GLFW_KEY_APOSTROPHE] = PBE::KEY_APOSTROPHE;
		GLFWKeytoPBEKey[GLFW_KEY_GRAVE_ACCENT] = PBE::KEY_TILDE;
		GLFWKeytoPBEKey[GLFW_KEY_COMMA] = PBE::KEY_COMMA;
		GLFWKeytoPBEKey[GLFW_KEY_PERIOD] = PBE::KEY_PERIOD;
		GLFWKeytoPBEKey[GLFW_KEY_SLASH] = PBE::KEY_SLASH;
		GLFWKeytoPBEKey[GLFW_KEY_LEFT_SHIFT] = PBE::KEY_LEFT_SHIFT;
		GLFWKeytoPBEKey[GLFW_KEY_RIGHT_SHIFT] = PBE::KEY_RIGHT_SHIFT;
	}

	int GLFWKeyToPBEKey(int glfwKey)
	{
		if (GLFWKeytoPBEKey.empty())
		{
			InitializeKeyMaps();
		}

		auto it = GLFWKeytoPBEKey.find(glfwKey);
		if (it != GLFWKeytoPBEKey.end())
		{
			return it->second;
		}

		return PBE::KEY_UNKNOWN;
	}
}
PBE::InputSystem::InputSystem(InputSystemCreateInfo const& info)
	: m_CreateInfo(info)
	, m_MouseX(0.0f)
	, m_MouseY(0.0f)
	, m_KeysPressed{ false }
	, m_KeysReleased{ false }
	, m_KeysHeld{ false }
	, m_MouseButtonsPressed{ false }
{
	m_CreateInfo.m_pWindow->m_OnWindowLoseFocus.AddSubscriber([this]()
		{
			for (int i = 0; i < 512; i++)
			{
				m_KeysHeld[i] = false;
			}
			for (int i = 0; i < 16; i++)
			{
				m_MouseButtonsHeld[i] = false;
			}

			return false;
		}
	);

	m_CreateInfo.m_pWindow->m_OnKeyPressed.AddSubscriber([this](int key, int /*scancode*/, int /*mods*/)
		{
			OnKeyPressed(key);
			return false;
		}
	);

	m_CreateInfo.m_pWindow->m_OnKeyReleased.AddSubscriber([this](int key, int /*scancode*/, int /*mods*/)
		{
			OnKeyReleased(key);
			return false;
		}
	);

	m_CreateInfo.m_pWindow->OnMouseButtonPressed.AddSubscriber([this](int button, int /*mods*/)
		{
			OnMouseButtonPressed(button);
			return false;
		}
	);

	m_CreateInfo.m_pWindow->OnMouseButtonReleased.AddSubscriber([this](int button, int /*mods*/)
		{
			OnMouseButtonReleased(button);
			return false;
		}
	);

	m_CreateInfo.m_pWindow->m_OnWindowFocus.AddSubscriber([this]()
		{
			ClearIO();
			return false;
		}
	);

	m_CreateInfo.m_pWindow->m_OnWindowLoseFocus.AddSubscriber([this]()
		{
			ClearIO();
			return false;
		}
	);
}

bool PBE::InputSystem::WasKeyPressed(int key) const
{
	return m_KeysPressed[key];
}

bool PBE::InputSystem::WasKeyReleased(int key) const
{
	return m_KeysReleased[key];
}

bool PBE::InputSystem::IsKeyHeld(int key) const
{
	return m_KeysHeld[key];
}

bool PBE::InputSystem::IsMouseButtonPressed(int button) const
{
	return m_MouseButtonsPressed[button];
}

bool PBE::InputSystem::IsMouseButtonReleased(int button) const
{
	return m_MouseButtonsReleased[button];
}

void PBE::InputSystem::GetMousePosition(float& x, float& y) const
{
	x = m_MouseX;
	y = m_MouseY;
}

void PBE::InputSystem::OnKeyPressed(int key)
{
	m_KeysPressed[key] = true;
	m_KeysHeld[key] = true;
}

void PBE::InputSystem::OnKeyReleased(int key)
{
	m_KeysReleased[key] = true;
	m_KeysHeld[key] = false;
}

void PBE::InputSystem::OnMouseButtonPressed(int button)
{
	m_MouseButtonsPressed[button] = true;
	m_MouseButtonsHeld[button] = true;
}

void PBE::InputSystem::OnMouseButtonReleased(int button)
{
	m_MouseButtonsReleased[button] = false;
	m_MouseButtonsHeld[button] = false;
}

void PBE::InputSystem::OnMouseMoved(float x, float y)
{
	m_MouseX = x;
	m_MouseY = y;
}

void PBE::InputSystem::ClearIO()
{
	for (bool& key : m_KeysPressed)
	{
		key = false;
	}
	for (bool& key : m_KeysReleased)
	{
		key = false;
	}
	for (bool& key : m_MouseButtonsPressed)
	{
		key = false;
	}
	for (bool& key : m_MouseButtonsReleased)
	{
		key = false;
	}
}

void PBE::InputSystem::BeginFrame()
{
}

void PBE::InputSystem::EndFrame()
{
	ClearIO();
}

void PBE::InputSystem::GetMousePosition(int& x, int& y) const
{
	x = static_cast<int>(m_MouseX);
	y = static_cast<int>(m_MouseY);
}
