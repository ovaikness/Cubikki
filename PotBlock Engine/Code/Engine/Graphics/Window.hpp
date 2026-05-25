#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/Result.hpp"
#include "Engine/Core/EventDispatcher.hpp"
#include <string>
#include <filesystem>

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

namespace PBE
{
	class InputSystem;

	enum class CursorMode
	{
		NORMAL,
		FPS
	};

	struct WindowCreateInfo
	{
		std::filesystem::path m_IconPath = "Assets/Textures/Icon.png";
		std::string m_Title = "PBE Window";
		int m_Width = 1280;
		int m_Height = 720;
		bool m_Fullscreen = false;
		bool m_Resizable = true;
		bool m_Vsync = true;
	};
	class Window
	{
	public:
		EventDispatcher<float, float> m_OnResize;
		EventDispatcher<int, int> m_OnWindowMove;
		EventDispatcher<> m_OnWindowFocus;
		EventDispatcher<> m_OnWindowLoseFocus;

		EventDispatcher<int> m_OnWindowIconify;
		EventDispatcher<int> m_OnWindowMaximize;
		EventDispatcher<> m_OnWindowRefresh;
		EventDispatcher<> m_OnWindowClose;
		EventDispatcher<float> m_OnWindowContentScale;

		EventDispatcher<int, int> OnMouseButtonPressed;
		EventDispatcher<int, int> OnMouseButtonReleased;

		EventDispatcher<std::string> m_OnPaste;
		EventDispatcher<std::string&> m_OnCopy;

		EventDispatcher<int, int, int> m_OnKeyPressed;
		EventDispatcher<int, int, int> m_OnKeyReleased;
		EventDispatcher<int> m_OnKeyTyped;
	protected:
		GLFWwindow* m_pWindow = nullptr;
		CursorMode m_CursorMode = CursorMode::NORMAL;
		Vec2 m_CursorClientDelta = Vec2::ZERO;
		WindowCreateInfo m_Config;
		int m_WindowedWidth = 1280;
		int m_WindowedHeight = 720;
	public:
		Window(WindowCreateInfo const& config);
		~Window();

		PBE::Result Startup();
		void Shutdown();

		void BeginFrame();
		void EndFrame();

		CursorMode GetCursorMode() const {
			return m_CursorMode;
		}

		GLFWwindow* GetWindow() {
			return m_pWindow;
		}
		float GetAspectRatio() const {
			return static_cast<float>(m_Config.m_Width) / static_cast<float>(m_Config.m_Height);
		}

		void SetFullscreen(bool value);
		bool GetFullscreen() const;

		float GetWidth() const;
		float GetHeight() const;

		void SetIcon(std::filesystem::path path);

		void SetWindowShouldClose(bool value) {
			glfwSetWindowShouldClose(m_pWindow, value);
		}
		bool GetWindowShouldClose() {
			return glfwWindowShouldClose(m_pWindow);
		}

		static void TerminateWindowingSystem()
		{
			if (s_glfwInitialized)
			{
				glfwTerminate();
				s_glfwInitialized = false;
			}
		}

		static void InitializeWindowingSystem()
		{
			if (!s_glfwInitialized)
			{
				glfwInit(); s_glfwInitialized = true;
			}
		}
		static void SetErrorCalback(GLFWerrorfun callback) {
			glfwSetErrorCallback(callback);
		}

		static void PollEvents() {
			glfwPollEvents();
		}

		void SetWindowContentScaleCallback(GLFWwindowcontentscalefun callback) {
			glfwSetWindowContentScaleCallback(m_pWindow, callback);
		}
		void SetWindowUserPointer(void* pointer) {
			glfwSetWindowUserPointer(m_pWindow, pointer);
		}
		void* GetWindowUserPointer() {
			return glfwGetWindowUserPointer(m_pWindow);
		}
		void SetInputMode(int mode, int value) {
			glfwSetInputMode(m_pWindow, mode, value);
		}
		void GetWindowSize(int* width, int* height) {
			glfwGetWindowSize(m_pWindow, width, height);
		}
		void SetWindowSize(int width, int height) {
			glfwSetWindowSize(m_pWindow, width, height);
		}
		void SetWindowPos(int x, int y) {
			glfwSetWindowPos(m_pWindow, x, y);
		}
		void GetWindowPos(int* x, int* y) {
			glfwGetWindowPos(m_pWindow, x, y);
		}
		void SetWindowTitle(const char* title) {
			glfwSetWindowTitle(m_pWindow, title);
		}
		void SetWindowIcon(GLFWimage* images) {
			glfwSetWindowIcon(m_pWindow, 1, images);
		}
		void SetWindowMonitor(GLFWmonitor* monitor, int x, int y, int width, int height, int refreshRate) {
			glfwSetWindowMonitor(m_pWindow, monitor, x, y, width, height, refreshRate);
		}
		void GetFramebufferSize(int* width, int* height) {
			glfwGetFramebufferSize(m_pWindow, width, height);
		}
		void GetWindowFrameSize(int* left, int* top, int* right, int* bottom) {
			glfwGetWindowFrameSize(m_pWindow, left, top, right, bottom);
		}
		void IconifyWindow() {
			glfwIconifyWindow(m_pWindow);
		}
		void RestoreWindow() {
			glfwRestoreWindow(m_pWindow);
		}
		void MaximizeWindow() {
			glfwMaximizeWindow(m_pWindow);
		}
		void ShowWindow() {
			glfwShowWindow(m_pWindow);
		}
		void HideWindow() {
			glfwHideWindow(m_pWindow);
		}
		void FocusWindow() {
			glfwFocusWindow(m_pWindow);
		}
		void RequestWindowAttention() {
			glfwRequestWindowAttention(m_pWindow);
		}
		void SetWindowAttrib(int attrib, int value) {
			glfwSetWindowAttrib(m_pWindow, attrib, value);
		}
		void SetWindowAspectRatio(int numer, int denom) {
			glfwSetWindowAspectRatio(m_pWindow, numer, denom);
		}
		void SetWindowSizeLimits(int minWidth, int minHeight, int maxWidth, int maxHeight) {
			glfwSetWindowSizeLimits(m_pWindow, minWidth, minHeight, maxWidth, maxHeight);
		}
		void SetWindowOpacity(float opacity) {
			glfwSetWindowOpacity(m_pWindow, opacity);
		}
		void SetCursorMode(CursorMode mode);

		Vec2 GetCursorClientDelta() const {
			return m_CursorClientDelta;
		}

		Vec2 GetCursorPos() const {
			double x, y;
			glfwGetCursorPos(m_pWindow, &x, &y);
			return Vec2(static_cast<float>(x), m_Config.m_Height - static_cast<float>(y));
		}

		static bool s_glfwInitialized;
	};
}