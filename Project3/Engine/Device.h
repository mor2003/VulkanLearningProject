#pragma once

#include "Window.h"

//std
#include <vector>
#include <iostream>
#include <stdexcept>
#include <optional>
#include <set>

namespace Engine
{
	const int MAX_FRAME_IN_FLIGHT = 2;

	#define DEBUG
	#ifdef  DEBUG
		const bool EnableValidationLayers = true;
	#else
		const bool EnableValidationLayers = false;
	#endif //  NDEBUG

	const std::vector<const char*> ValidationLayers = {
		"VK_LAYER_KHRONOS_validation"
	};

	const std::vector<const char*> DeviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	struct QueueFamilyIndices {
		std::optional<uint32_t> graphicsQueue;
		std::optional<uint32_t> presentQueue;

		bool isComplete() {
			return graphicsQueue.has_value() && presentQueue.has_value();
		}
	};

	struct SwapChainSupportDetails
	{
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> Formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	class Device
	{
		public:
			Device(Window& wind);
			~Device();

			Device(const Device&) = delete;
			Device& operator=(const Device&) = delete;

			SwapChainSupportDetails GetSwapchainDetails() { return findSwapchainDetails(PhysicalDevice); }
			QueueFamilyIndices GetFamilyIndices() { return findQueueFamilies(PhysicalDevice); }

			VkDevice device() { return _device; }
			VkSurfaceKHR surface() { return _surface; }

			VkCommandPool CommandPool() { return _commandPool; }
			VkDescriptorPool DescriptorPool() { return _descriptorPool; }

			VkQueue GraphicsQueue() { return _GraphicsQueue; }
			VkQueue PresentQueue() { return _PresentQueue; }

			void createBuffer(
				VkBuffer& VertexBuffer,
				VkDeviceSize BufferSize,
				VkBufferUsageFlags Usage,
				VkMemoryPropertyFlags properties,
				VkDeviceMemory& VertexBufferMemory
			);

		private:
			void InitVulk();
			void setDebugMessenger();
			void CreateSurface();
			void GetPhysicalDevice();
			void createLogic();
			void createCommandPool();
			void createDescriptorPool();

			std::vector<const char*> GetInstanceExtensions();
			bool CheckRequiredLayers();
			bool checkForDeviceExtensions(VkPhysicalDevice device);
			void PopulateDebugMessenger(VkDebugUtilsMessengerCreateInfoEXT& debugInfo);

			static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
				VkDebugUtilsMessageSeverityFlagBitsEXT MessageSeverity,
				VkDebugUtilsMessageTypeFlagsEXT MessageType,
				const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
				void* pUserData
			);
			bool SuitableDevice(VkPhysicalDevice device);
			QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

			VkResult CreateDebugUtilsMessengerEXT(VkInstance Instance, const VkDebugUtilsMessengerCreateInfoEXT* pDebugInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pMessenger);
			void DestroyDebugUtilsMessengerEXT(VkInstance Instance, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT Messenger);
			SwapChainSupportDetails findSwapchainDetails(VkPhysicalDevice device);

			uint32_t findMemType(uint32_t TypeFilter, VkMemoryPropertyFlags properties);

			VkInstance Instance;
			VkDebugUtilsMessengerEXT debugMessenger;
			VkPhysicalDevice PhysicalDevice = VK_NULL_HANDLE;
			VkPhysicalDeviceProperties deviceProperites;
			VkDevice _device;

			VkCommandPool _commandPool;
			VkDescriptorPool _descriptorPool;

			//Queues
			VkQueue _GraphicsQueue;
			VkQueue _PresentQueue;

			VkSurfaceKHR _surface;

			Window& window;
	};
}