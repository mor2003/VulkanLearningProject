#pragma once

#include "Window.h"

//std
#include <vector>
#include <iostream>
#include <string>
#include <array>
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

			VkDescriptorPool DescriptorPool() { return _descriptorPool; }
			VkCommandPool CommandPool() { return _commandPool; }

			VkQueue GraphicsQueue() { return _GraphicsQueue; }
			VkQueue PresentQueue() { return _PresentQueue; }
			float GetMaxAntisotropy() { return deviceProperites.limits.maxSamplerAnisotropy; }

			void createBuffer(
				VkBuffer& VertexBuffer,
				VkDeviceSize BufferSize,
				VkBufferUsageFlags Usage,
				VkMemoryPropertyFlags properties,
				VkDeviceMemory& VertexBufferMemory
			);

			void createImage(
				VkImage& Image,
				VkExtent2D TexExtent,
				VkImageTiling ImageTiling,
				VkFormat ColorFormat,
				VkDeviceSize ImageSize,
				VkImageUsageFlags Usage,
				VkMemoryPropertyFlags properties,
				VkDeviceMemory& ImageMemory
			);

			VkFormat findDepthFormat() {
				return findSupportedDepthFormats(
					{VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
					VK_IMAGE_TILING_OPTIMAL,
					VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
				);
			}

			bool isStencilTestSupported(VkFormat format) {
				return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
			}

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
			bool SuitableDevice(VkPhysicalDevice device, uint32_t devicesCount);
			QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

			VkResult CreateDebugUtilsMessengerEXT(VkInstance Instance, const VkDebugUtilsMessengerCreateInfoEXT* pDebugInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pMessenger);
			void DestroyDebugUtilsMessengerEXT(VkInstance Instance, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT Messenger);
			SwapChainSupportDetails findSwapchainDetails(VkPhysicalDevice device);

			uint32_t findMemType(uint32_t TypeFilter, VkMemoryPropertyFlags properties);

			VkFormat findSupportedDepthFormats(const std::vector<VkFormat> candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

			VkInstance Instance;
			VkDebugUtilsMessengerEXT debugMessenger;
			VkPhysicalDevice PhysicalDevice = VK_NULL_HANDLE;
			VkPhysicalDeviceProperties deviceProperites;
			VkDevice _device;

			VkDescriptorPool _descriptorPool;
			VkCommandPool _commandPool;

			//Queues
			VkQueue _GraphicsQueue;
			VkQueue _PresentQueue;

			VkSurfaceKHR _surface;

			Window& window;
	};
}