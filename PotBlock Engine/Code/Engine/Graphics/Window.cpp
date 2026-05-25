#include "Engine/Graphics/Window.hpp"

#include <stb/stb_image.h>
#include "imgui_impl_glfw.h"
bool PBE::Window::s_glfwInitialized = false;

namespace PBE
{
	class WindowException : public std::exception
	{
	private:
		const char* m_message;
	public:
		WindowException(const char* message) : m_message(message) {}
		const char* what() const noexcept override {
			return m_message;
		}
	};
}

PBE::Window::Window(WindowCreateInfo const& config)
	: m_pWindow(nullptr)
	, m_Config(config)
{
	
}

PBE::Window::~Window()
{
	if (m_pWindow)
	{
		Shutdown();
	}
}

void PBE::Window::SetCursorMode(CursorMode mode)
{
	m_CursorMode = mode;
	switch (m_CursorMode)
	{
		case CursorMode::NORMAL:
			glfwSetInputMode(m_pWindow, GLFW_CURSOR, GLFW_CURSOR_CAPTURED);
			break;
		case CursorMode::FPS:
			glfwSetInputMode(m_pWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			break;
	}
}

PBE::Result PBE::Window::Startup()
{
	if (!s_glfwInitialized)
	{
		int result = glfwInit();
		s_glfwInitialized = true;
		if (!result)
		{
			return PBE::RESULT_FAILURE;
		}
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, m_Config.m_Resizable);
	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

	if (m_Config.m_Fullscreen)
	{
		GLFWmonitor* monitor = glfwGetPrimaryMonitor();
		m_Config.m_Height = glfwGetVideoMode(monitor)->height;
		m_Config.m_Width = glfwGetVideoMode(monitor)->width;
		m_pWindow = glfwCreateWindow(m_Config.m_Width, m_Config.m_Height, m_Config.m_Title.c_str(), monitor, nullptr);
	}
	else
	{
		m_pWindow = glfwCreateWindow(m_Config.m_Width, m_Config.m_Height, m_Config.m_Title.c_str(), nullptr, nullptr);
	}

	ImGui_ImplGlfw_InitForVulkan(m_pWindow, false);

	glfwSetWindowUserPointer(m_pWindow, this);

	glfwSetFramebufferSizeCallback(m_pWindow, [](GLFWwindow* window, int width, int height)
		{
			PBE::Window* pWindow = static_cast<PBE::Window*>(glfwGetWindowUserPointer(window));
			pWindow->m_Config.m_Width = width;
			pWindow->m_Config.m_Height = height;
			pWindow->m_OnResize.Invoke(static_cast<float>(width), static_cast<float>(height));
		});

	glfwSetKeyCallback(m_pWindow, [](GLFWwindow* window, int key, int scancode, int action, int mods)
		{
			PBE::Window* pWindow = static_cast<PBE::Window*>(glfwGetWindowUserPointer(window));

			ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
			switch (action)
			{
			case GLFW_PRESS:
				pWindow->m_OnKeyPressed.Invoke(key, scancode, mods);

				if (key == GLFW_KEY_C && (mods & GLFW_MOD_CONTROL))
				{
					std::string copyString;
					pWindow->m_OnCopy.Invoke(copyString);
					glfwSetClipboardString(window, copyString.c_str());
				}
				else if (key == GLFW_KEY_V && (mods & GLFW_MOD_CONTROL))
				{
					pWindow->m_OnPaste.Invoke(glfwGetClipboardString(window));
				}
				else if (
					key == GLFW_KEY_BACKSPACE ||
					key == GLFW_KEY_LEFT ||
					key == GLFW_KEY_RIGHT ||
					key == GLFW_KEY_UP ||
					key == GLFW_KEY_DOWN
					)
				{
					pWindow->m_OnKeyTyped.Invoke(key);
				}
				break;
			case GLFW_REPEAT:
				if (
					key == GLFW_KEY_V &&
					(mods & GLFW_MOD_CONTROL)
					)
				{
					pWindow->m_OnPaste.Invoke(glfwGetClipboardString(window));
				}
				else if (
					key == GLFW_KEY_BACKSPACE ||
					key == GLFW_KEY_LEFT ||
					key == GLFW_KEY_RIGHT ||
					key == GLFW_KEY_UP ||
					key == GLFW_KEY_DOWN
					)
				{
					pWindow->m_OnKeyTyped.Invoke(key);
				}
				break;
			case GLFW_RELEASE:
				pWindow->m_OnKeyReleased.Invoke(key, scancode, mods);
				break;
			}
		});

	glfwSetMouseButtonCallback(m_pWindow, [](GLFWwindow* window, int button, int action, int mods)
		{
			PBE::Window* pWindow = static_cast<PBE::Window*>(glfwGetWindowUserPointer(window));

			ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
			switch (action)
			{
			case GLFW_PRESS:
				pWindow->OnMouseButtonPressed.Invoke(button, mods);
				break;
			case GLFW_RELEASE:
				pWindow->OnMouseButtonReleased.Invoke(button, mods);
				break;
			}
		});

	glfwSetScrollCallback(m_pWindow, [](GLFWwindow* window, double xOffset, double yOffset)
		{
			ImGui_ImplGlfw_ScrollCallback(window, xOffset, yOffset);
		}
	);

	glfwSetWindowFocusCallback(m_pWindow, [](GLFWwindow* window, int gainedFocus)
		{
			PBE::Window* pWindow = static_cast<PBE::Window*>(glfwGetWindowUserPointer(window));

			ImGui_ImplGlfw_WindowFocusCallback(window, gainedFocus);
			if (gainedFocus == GLFW_TRUE)
			{
				pWindow->m_OnWindowFocus.Invoke();
			}
			else
			{
				pWindow->m_OnWindowLoseFocus.Invoke();
			}
		}
	);

	glfwSetCharCallback(m_pWindow, [](GLFWwindow* window, unsigned int codepoint)
		{
			ImGui_ImplGlfw_CharCallback(window, codepoint);
			PBE::Window* pWindow = static_cast<PBE::Window*>(glfwGetWindowUserPointer(window));
			pWindow->m_OnKeyTyped.Invoke(codepoint);
		});
	
	SetIcon(m_Config.m_IconPath);
	glfwShowWindow(m_pWindow);

	return PBE::RESULT_SUCCESS;
}

void PBE::Window::Shutdown()
{
	if (m_pWindow)
	{
		glfwDestroyWindow(m_pWindow);
		m_pWindow = nullptr;
	}
}

void PBE::Window::BeginFrame()
{
}

void PBE::Window::EndFrame()
{
	double x, y;
	glfwGetCursorPos(m_pWindow, &x, &y);
	static double lastX = x;
	static double lastY = y;
	static bool first = true;
	switch (m_CursorMode)
	{
		case CursorMode::NORMAL:
			glfwSetInputMode(m_pWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			if (first)
			{
				lastX = x;
				lastY = y;
				first = false;
			}

			m_CursorClientDelta = Vec2(static_cast<float>(x - lastX), static_cast<float>(y - lastY));

			lastX = x;
			lastY = y;
			break;
		case CursorMode::FPS:
			glfwSetInputMode(m_pWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

			double dx = x - (m_Config.m_Width * 0.5);
			double dy = y - (m_Config.m_Height * 0.5);
			m_CursorClientDelta = Vec2(static_cast<float>(dx), static_cast<float>(dy));

			// Reset cursor to center for the next frame
			glfwSetCursorPos(m_pWindow, x - dx, y - dy);
			break;
	}

	if (glfwRawMouseMotionSupported())
	{
		glfwSetInputMode(m_pWindow, GLFW_RAW_MOUSE_MOTION,
			m_CursorMode == CursorMode::FPS ? GLFW_TRUE : GLFW_FALSE);
	}
}

void PBE::Window::SetFullscreen(bool value)
{
	m_Config.m_Fullscreen = value;
	if (m_Config.m_Fullscreen)
	{
		m_WindowedWidth = m_Config.m_Width;
		m_WindowedHeight = m_Config.m_Height;
		GLFWmonitor* monitor = glfwGetPrimaryMonitor();
		m_Config.m_Height = glfwGetVideoMode(monitor)->height;
		m_Config.m_Width = glfwGetVideoMode(monitor)->width;
		glfwSetWindowMonitor(m_pWindow, monitor, 0, 0, m_Config.m_Width, m_Config.m_Height, GLFW_DONT_CARE);
	}
	else
	{
		m_Config.m_Height = m_WindowedHeight;
		m_Config.m_Width = m_WindowedWidth;
		glfwSetWindowMonitor(m_pWindow, nullptr, 0, 0, m_WindowedWidth, m_WindowedHeight, GLFW_DONT_CARE);
	}

	int x, y;
	x = (int)((glfwGetVideoMode(glfwGetPrimaryMonitor())->width - m_Config.m_Width) * 0.5f);
	y = (int)((glfwGetVideoMode(glfwGetPrimaryMonitor())->height - m_Config.m_Height) * 0.5f);

	glfwSetWindowSize(m_pWindow, m_Config.m_Width, m_Config.m_Height);
	glfwSetWindowPos(m_pWindow, x, y);
}

bool PBE::Window::GetFullscreen() const
{
	return m_Config.m_Fullscreen;
}

float PBE::Window::GetWidth() const
{
	return (float)m_Config.m_Width;
}

float PBE::Window::GetHeight() const
{
	return (float)m_Config.m_Height;
}

void PBE::Window::SetIcon(std::filesystem::path path)
{
	GLFWimage images[1]{};
	images[0].pixels = stbi_load(path.string().c_str(), &images[0].width, &images[0].height, 0, 4); //rgba channels 
	glfwSetWindowIcon(m_pWindow, 1, images);
	stbi_image_free(images[0].pixels);
}

