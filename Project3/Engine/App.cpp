#include "App.h"

namespace Engine
{
	void App::Run() {
		SimpleRenderereSystem simpleRenderSystem{ device, renderer.GetSwapchainRenderPass() };

		while (!window.ShouldClose()) {
			glfwPollEvents();

			if (auto commandBuffer = renderer.StartFrame()) {
				renderer.StartSwapchainRenderPass(commandBuffer);
				simpleRenderSystem.RenderObject(commandBuffer, currentFrame);
				renderer.EndSwapchainRenderPass(commandBuffer);
				simpleRenderSystem.UniformUpdates(currentFrame, renderer.GetSwapchainExtent());
				renderer.EndFrame();
			}

			currentFrame = (currentFrame + 1) % MAX_FRAME_IN_FLIGHT;
		}

		vkDeviceWaitIdle(device.device());
	}
}