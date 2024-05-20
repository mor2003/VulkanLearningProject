#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace Engine
{
	class Window {
	public:
		Window(int w, int h);
		~Window();

		Window(const Window&) = delete;
		Window& operator=(const Window&) = delete;

		bool ShouldClose() { return glfwWindowShouldClose(window); }
		VkResult createWindowSurface(VkInstance Instance, VkSurfaceKHR* pSurface) { return glfwCreateWindowSurface(Instance, window, nullptr, pSurface); }
		VkExtent2D WindowExtent() { return { static_cast<uint32_t>(width), static_cast<uint32_t>(height) }; }

		bool ResizedFlag() { return framebufferResized; }
		void ResetResizedFlag() { framebufferResized = false; }

	private:
		void createWindow();

		static void framebufferResizedCallback(GLFWwindow* window, int width, int height);

		GLFWwindow* window = nullptr;

		int width;
		int height;
		bool framebufferResized = false;
	};
}