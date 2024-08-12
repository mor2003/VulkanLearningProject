#pragma once

#include "Window.h"
#include "Device.h"

#include "Renderer.h"
#include "SimpleRenderereSystem.h"


#include <memory>

namespace Engine
{
	class App
	{
	public:

		static constexpr int width = 800;
		static constexpr int height = 800;

		void Run();

	private:

		uint32_t currentFrame = 0;

		Window window{ width, height };
		Device device{ window };
		Renderer renderer{ device, window };

		std::unique_ptr<Model> model;
	};
}