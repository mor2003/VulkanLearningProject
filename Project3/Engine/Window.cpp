#include "Window.h"

namespace Engine
{
	Window::Window(int w, int h) : width{ w }, height{ h } {
		createWindow();
	}

	Window::~Window() {
		glfwDestroyWindow(window);
		glfwTerminate();
	}

	void Window::createWindow() {
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

		window = glfwCreateWindow(width, height, "Vulkan Tutorial Website", nullptr, nullptr);
		glfwSetWindowUserPointer(window, this);
		glfwSetFramebufferSizeCallback(window, framebufferResizedCallback);
	}

	void Window::framebufferResizedCallback(GLFWwindow* window, int width, int height) {
		auto newWindow = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
		newWindow->framebufferResized = true;
		newWindow->width = width;
		newWindow->height = height;
	}
}