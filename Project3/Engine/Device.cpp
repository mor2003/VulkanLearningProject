#include "Device.h"

namespace Engine {
	Device::Device(Window& wind) : window{wind} {
		InitVulk();
		setDebugMessenger();
		CreateSurface();
		GetPhysicalDevice();
		createLogic();
		createCommandPool();
		createDescriptorPool();
	}

	Device::~Device() {
		vkDestroyDescriptorPool(_device, _descriptorPool, nullptr);
		vkDestroyCommandPool(_device, _commandPool, nullptr);

		vkDestroyDevice(_device, nullptr);

		vkDestroySurfaceKHR(Instance, _surface, nullptr);

		if (EnableValidationLayers) {
			DestroyDebugUtilsMessengerEXT(Instance, nullptr, debugMessenger);
		}

		vkDestroyInstance(Instance, nullptr);
	}

	void Device::InitVulk() {
		if (EnableValidationLayers && !CheckRequiredLayers()) {
			throw std::runtime_error("Layer required but not available");
		}

		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pApplicationName = "Vulkan Tutorial Website";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "No Engine";
		appInfo.apiVersion = VK_VERSION_1_0;

		VkInstanceCreateInfo InstanceInfo{};
		InstanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		InstanceInfo.pApplicationInfo = &appInfo;

		auto extensions = GetInstanceExtensions();
		InstanceInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		InstanceInfo.ppEnabledExtensionNames = extensions.data();

		VkDebugUtilsMessengerCreateInfoEXT debugInfo;
		if (EnableValidationLayers) {
			InstanceInfo.enabledLayerCount = static_cast<uint32_t>(ValidationLayers.size());
			InstanceInfo.ppEnabledLayerNames = ValidationLayers.data();

			PopulateDebugMessenger(debugInfo);
			InstanceInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugInfo;
		}
		else {
			InstanceInfo.enabledLayerCount = 0;
		}
		
		if (vkCreateInstance(&InstanceInfo, nullptr, &Instance) != VK_SUCCESS) {
			throw std::runtime_error("failed to create the application's vulkan instance");
		}

		//std::cout << "Instance were created" << std::endl;
	}

	std::vector<const char*> Device::GetInstanceExtensions() {
		uint32_t ExtensionsCount;
		const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&ExtensionsCount);

		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + ExtensionsCount);

		if (EnableValidationLayers) {
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}

		return extensions;
	}

	bool Device::CheckRequiredLayers() {
		uint32_t layersCount = 0;
		vkEnumerateInstanceLayerProperties(&layersCount, nullptr);
		std::vector<VkLayerProperties> AvailableLayers(layersCount);
		vkEnumerateInstanceLayerProperties(&layersCount, AvailableLayers.data());

		for (const char* Layer : ValidationLayers) {
			bool LayerFound = false;

			for (const auto& LayerProperties : AvailableLayers) {
				if (strcmp(LayerProperties.layerName, Layer) == 0) {
					LayerFound = true;
					break;
				}
			}
			if (!LayerFound) {
				return false;
			}
		}

		return true;
	}

	void Device::setDebugMessenger() {
		if (!EnableValidationLayers) return;

		VkDebugUtilsMessengerCreateInfoEXT DebugInfo;
		PopulateDebugMessenger(DebugInfo);

		if (CreateDebugUtilsMessengerEXT(Instance, &DebugInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
			throw std::runtime_error("failed to create debug messenger");
		}
	}

	VKAPI_ATTR VkBool32 VKAPI_CALL Device::DebugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT MessageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT MessageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData
	) {
		std::cerr << pCallbackData->pMessage << "\n" << std::endl;
		return VK_FALSE;
	}

	void Device::PopulateDebugMessenger(VkDebugUtilsMessengerCreateInfoEXT& debugInfo) {
		debugInfo = {};
		debugInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		debugInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		debugInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
		debugInfo.pfnUserCallback = DebugCallback;
		debugInfo.pUserData = nullptr;
	}

	VkResult Device::CreateDebugUtilsMessengerEXT(VkInstance Instance, const VkDebugUtilsMessengerCreateInfoEXT* pDebugInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pMessenger) {
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(Instance, "vkCreateDebugUtilsMessengerEXT");
		if (func != nullptr) {
			return func(Instance, pDebugInfo, pAllocator, pMessenger);
		}
		else {
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}

	void Device::DestroyDebugUtilsMessengerEXT(VkInstance Instance, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT Messenger) {
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(Instance, "vkDestroyDebugUtilsMessengerEXT");
		if (func != nullptr) {
			func(Instance, Messenger, pAllocator);
		}
	}

	void Device::GetPhysicalDevice() {
		uint32_t devicesCount = 0;
		vkEnumeratePhysicalDevices(Instance, &devicesCount, nullptr);
		if (devicesCount == 0) {
			throw std::runtime_error("failed to find device which support vulkan");
		}

		std::vector<VkPhysicalDevice> devices(devicesCount);
		vkEnumeratePhysicalDevices(Instance, &devicesCount, devices.data());

		for (const auto& device : devices) {
			if (SuitableDevice(device)) {
				PhysicalDevice = device;
			}
		}

		std::cout << "device size" << devices.size() << std::endl;
		if (PhysicalDevice == VK_NULL_HANDLE) {
			throw std::runtime_error("failed to find a suitable device");
		}

		vkGetPhysicalDeviceProperties(PhysicalDevice, &deviceProperites);
		std::cout << "Using GPU: " << deviceProperites.deviceName << std::endl;
	}

	bool Device::SuitableDevice(VkPhysicalDevice device) {
		QueueFamilyIndices indices = findQueueFamilies(device);

		bool ExtensionsSupport = checkForDeviceExtensions(device);

		bool swapChainAdequate = false;
		if (ExtensionsSupport) {
			SwapChainSupportDetails swapchainSupport = findSwapchainDetails(device);
			swapChainAdequate = !swapchainSupport.Formats.empty() && !swapchainSupport.presentModes.empty();
		}

		/*
		In Case you want to use a specific device in your pc (Can be used for easy development) 
			- Pay attention that if you give wrong character the App won't be able to find the device
			  and will return an error in Lines: 168 - 172

			- if empty it will choose by default
		*/
		const char* YourDevice = "";

		VkPhysicalDeviceProperties devicePropery;
		vkGetPhysicalDeviceProperties(device, &devicePropery);
		std::string CurrentDeviceName = devicePropery.deviceName;
		bool RightDevice = false;
		if (CurrentDeviceName.find(YourDevice) != std::string::npos) {
			RightDevice = true;
		}

		return indices.isComplete() && ExtensionsSupport && swapChainAdequate && RightDevice;
	}

	QueueFamilyIndices Device::findQueueFamilies(VkPhysicalDevice device) {
		QueueFamilyIndices indices;

		uint32_t queueFamiliesCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamiliesCount, nullptr);
		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamiliesCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamiliesCount, queueFamilies.data());

		int i = 0;
		for (const auto& queueFamily : queueFamilies) {
			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				indices.graphicsQueue = i;
			}

			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, _surface, &presentSupport);
			if (presentSupport) {
				indices.presentQueue = i;
			}

			if (indices.isComplete()) {
				break;
			}

			i++;
		}

		return indices;
	}

	void Device::createLogic() {
		QueueFamilyIndices indices = findQueueFamilies(PhysicalDevice);

		float Priority = 1.0f;
		std::vector <VkDeviceQueueCreateInfo> DeviceQueuesInfo;
		std::set<uint32_t> DeviceQueuesValues = { indices.graphicsQueue.value(), indices.presentQueue.value() };

		for (uint32_t queueValue : DeviceQueuesValues)
		{
			VkDeviceQueueCreateInfo queueInfos{};
			queueInfos.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueInfos.queueCount = 1;
			queueInfos.queueFamilyIndex = queueValue;
			queueInfos.pQueuePriorities = &Priority;
			DeviceQueuesInfo.push_back(queueInfos);
		}

		VkPhysicalDeviceFeatures features{};


		VkDeviceCreateInfo deviceInfo{};
		deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceInfo.queueCreateInfoCount = static_cast<uint32_t>(DeviceQueuesInfo.size());
		deviceInfo.pQueueCreateInfos = DeviceQueuesInfo.data();

		deviceInfo.pEnabledFeatures = &features;

		deviceInfo.enabledExtensionCount = static_cast<uint32_t>(DeviceExtensions.size());
		deviceInfo.ppEnabledExtensionNames = DeviceExtensions.data();

		if (EnableValidationLayers) {
			deviceInfo.enabledLayerCount = static_cast<uint32_t>(ValidationLayers.size());
			deviceInfo.ppEnabledLayerNames = ValidationLayers.data();
		}
		else {
			deviceInfo.enabledLayerCount = 0;
		}

		if (vkCreateDevice(PhysicalDevice, &deviceInfo, nullptr, &_device) != VK_SUCCESS) {
			throw std::runtime_error("failed to create the device's logic");
		}

		vkGetDeviceQueue(_device, indices.graphicsQueue.value(), 0, &_GraphicsQueue);
		vkGetDeviceQueue(_device, indices.presentQueue.value(), 0, &_PresentQueue);

		std::cout << "Device has been created\n" << std::endl;
	}

	void Device::CreateSurface() {
		if (window.createWindowSurface(Instance, &_surface) != VK_SUCCESS) {
			throw std::runtime_error("failed to create the window surface");
		}
	}

	bool Device::checkForDeviceExtensions(VkPhysicalDevice device) {
		uint32_t deviceExtensionsCount = 0;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &deviceExtensionsCount, nullptr);
		std::vector<VkExtensionProperties> AvailableExtensions(deviceExtensionsCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &deviceExtensionsCount, AvailableExtensions.data());

		std::set <std::string> requiredExtensions(DeviceExtensions.begin(), DeviceExtensions.end());

		for (const auto& extensionProperties : AvailableExtensions) {
			requiredExtensions.erase(extensionProperties.extensionName);
		}

		return requiredExtensions.empty();
	}

	SwapChainSupportDetails Device::findSwapchainDetails(VkPhysicalDevice device) {
		SwapChainSupportDetails details;

		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, _surface, &details.capabilities);

		uint32_t FormatsCount = 0;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, _surface, &FormatsCount, nullptr);
		details.Formats.resize(FormatsCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, _surface, &FormatsCount, details.Formats.data());

		uint32_t PresentModeCounts = 0;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, _surface, &PresentModeCounts, nullptr);
		details.presentModes.resize(PresentModeCounts);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, _surface, &PresentModeCounts, details.presentModes.data());

		return details;
	}

	void Device::createCommandPool() {
		QueueFamilyIndices indices = findQueueFamilies(PhysicalDevice);

		VkCommandPoolCreateInfo PoolInfo{};
		PoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		PoolInfo.queueFamilyIndex = indices.graphicsQueue.value();
		PoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		
		if (vkCreateCommandPool(_device, &PoolInfo, nullptr, &_commandPool) != VK_SUCCESS) {
			throw std::runtime_error("failed to create the command pool");
		}
	}

	void Device::createBuffer(
		VkBuffer& VertexBuffer,
		VkDeviceSize BufferSize,
		VkBufferUsageFlags Usage,
		VkMemoryPropertyFlags properties,
		VkDeviceMemory& VertexBufferMemory

	) {
		VkBufferCreateInfo BufferInfo{};
		BufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		BufferInfo.size = BufferSize;
		BufferInfo.usage = Usage;
		BufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateBuffer(_device, &BufferInfo, nullptr, &VertexBuffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to create a vertex buffer");
		}

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(_device, VertexBuffer, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = findMemType(memRequirements.memoryTypeBits, properties);

		if (vkAllocateMemory(_device, &allocInfo, nullptr, &VertexBufferMemory) != VK_SUCCESS) {
			throw std::runtime_error("failed to create device memory object");
		}

		vkBindBufferMemory(_device, VertexBuffer, VertexBufferMemory, 0);
	}

	uint32_t Device::findMemType(uint32_t TypeFilter, VkMemoryPropertyFlags properties) {
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(PhysicalDevice, &memProperties);

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
			if ((TypeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
				return i;
			}
		}

		throw std::runtime_error("failed to find suitable memory type");
	}

	void Device::createDescriptorPool() {
		VkDescriptorPoolSize PoolSize{};
		PoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		PoolSize.descriptorCount = static_cast<uint32_t>(MAX_FRAME_IN_FLIGHT);

		VkDescriptorPoolCreateInfo PoolInfo{};
		PoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		PoolInfo.poolSizeCount = 1;
		PoolInfo.pPoolSizes = &PoolSize;
		PoolInfo.maxSets = static_cast<uint32_t>(MAX_FRAME_IN_FLIGHT);

		if (vkCreateDescriptorPool(_device, &PoolInfo, nullptr, &_descriptorPool) != VK_SUCCESS) {
			throw std::runtime_error("failed to create Descriptor pool");
		}
	}
}