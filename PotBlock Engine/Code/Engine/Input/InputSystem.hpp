#pragma once
#include "Engine/Input/KeyCodes.hpp"

namespace PBE
{	
	class Window;
	struct InputSystemCreateInfo
	{
		Window* m_pWindow;
	};

	class Window;
	class InputSystem
	{
	public:
		InputSystem(InputSystemCreateInfo const& info);
		virtual ~InputSystem() = default;

		bool WasKeyPressed(int key) const;
		bool WasKeyReleased(int key) const;
		bool IsKeyHeld(int key) const;

		bool IsMouseButtonPressed(int button) const;
		bool IsMouseButtonReleased(int button) const;
		void GetMousePosition(float& x, float& y) const;
		void GetMousePosition(int& x, int& y) const;

		//Required to be called by the application to function.
		void OnKeyPressed(int key);
		void OnKeyReleased(int key);
		void OnMouseButtonPressed(int button);
		void OnMouseButtonReleased(int button);
		void OnMouseMoved(float x, float y);

		void ClearIO();

		void BeginFrame();
		void EndFrame();
	protected:
		InputSystemCreateInfo m_CreateInfo;

		bool m_KeysPressed[512];
		bool m_KeysReleased[512];
		bool m_KeysHeld[512];

		bool m_MouseButtonsPressed[16];
		bool m_MouseButtonsReleased[16];
		bool m_MouseButtonsHeld[16];

		float m_MouseX;
		float m_MouseY;
	};
}