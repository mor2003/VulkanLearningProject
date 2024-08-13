#include "App.h"

namespace Engine
{
	void App::Run() {
		Camera camera{ device, static_cast<int>(renderer.GetSwapchainExtent().width), static_cast<int>(renderer.GetSwapchainExtent().height), glm::vec3(0.0f, 0.0f, 2.0f) };
		SimpleRenderereSystem simpleRenderSystem{ device, renderer.GetSwapchainRenderPass(), camera};

		while (!window.ShouldClose()) {
			glfwPollEvents();

			if (auto commandBuffer = renderer.StartFrame()) {
				renderer.StartSwapchainRenderPass(commandBuffer);
				simpleRenderSystem.RenderObject(commandBuffer, currentFrame);
				renderer.EndSwapchainRenderPass(commandBuffer);
				camera.Inputs(window.WindowHandler());
				camera.Matrix(currentFrame);
				renderer.EndFrame();
			}

			currentFrame = (currentFrame + 1) % MAX_FRAME_IN_FLIGHT;
		}

		vkDeviceWaitIdle(device.device());
	}
}