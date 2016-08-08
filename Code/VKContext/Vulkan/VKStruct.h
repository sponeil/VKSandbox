// VKStruct.h
//
#ifndef __VKStruct_h__
#define __VKStruct_h__


namespace VK {

// Declaring these here so the struct initializers can use them as needed
extern VkPhysicalDeviceProperties deviceProperties;
extern VkPhysicalDeviceFeatures deviceFeatures;
extern VkPhysicalDeviceMemoryProperties memoryProperties;
extern std::vector<VkFormatProperties> formatProperties;

// Same here
inline bool findMemoryType(uint32_t typeBits, VkFlags requirements_mask, uint32_t *typeIndex) {
	// Search memtypes to find first index with those properties
	for (uint32_t i = 0; i < VK_MAX_MEMORY_TYPES; i++) {
		if ((typeBits & 1) == 1) {
			// Type is available, does it match user properties?
			if ((memoryProperties.memoryTypes[i].propertyFlags & requirements_mask) == requirements_mask) {
				*typeIndex = i;
				return true;
			}
		}
		typeBits >>= 1;
	}
	// No memory types matched, return failure
	return false;
}


struct ApplicationInfo : public VkApplicationInfo {
	ApplicationInfo(const char *appName = "VulkanTest", uint32_t major = 1, uint32_t minor = 0, uint32_t patch = 0) {
		sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		pNext = nullptr;
		pEngineName = pApplicationName = appName;
		engineVersion = applicationVersion = VK_MAKE_VERSION(major, minor, patch);
		apiVersion = VK_MAKE_VERSION(1, 0, 11); //VK_API_VERSION;
	}
};

struct InstanceCreateInfo : public VkInstanceCreateInfo {
	ApplicationInfo appInfo;

	InstanceCreateInfo(const char *appName, std::vector<const char *> *layers = nullptr, std::vector<const char *> *extensions = nullptr) : appInfo(appName) {
		sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		pNext = nullptr;
		flags = 0;
		pApplicationInfo = &appInfo;
		enabledLayerCount = layers ? (uint32_t)layers->size() : 0;
		ppEnabledLayerNames = enabledLayerCount ? &(*layers)[0] : nullptr;
		enabledExtensionCount = extensions ? (uint32_t)extensions->size() : 0;
		ppEnabledExtensionNames = enabledExtensionCount ? &(*extensions)[0] : nullptr;
	}
};

struct DebugReportCallbackCreateInfoEXT : public VkDebugReportCallbackCreateInfoEXT {
	DebugReportCallbackCreateInfoEXT(PFN_vkDebugReportCallbackEXT callback) {
		sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
		pNext = nullptr;
		pfnCallback = callback;
		pUserData = nullptr;
		flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
	}
};

#ifdef _WIN32
struct Win32SurfaceCreateInfoKHR : public VkWin32SurfaceCreateInfoKHR {
	Win32SurfaceCreateInfoKHR(HINSTANCE hInstance, HWND hWnd) {
		sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		pNext = nullptr;
		flags = 0;
		hinstance = hInstance;
		hwnd = hWnd;
	}
};
#endif

struct DeviceQueueCreateInfo : public VkDeviceQueueCreateInfo {
	DeviceQueueCreateInfo(uint32_t index = 0) {
		static float defaultPriorities[1] = { 0.0 };
		sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		pNext = nullptr;
		flags = 0;
		queueFamilyIndex = index;
		queueCount = 1;
		pQueuePriorities = defaultPriorities;
	}
};

struct DeviceCreateInfo : public VkDeviceCreateInfo {
	DeviceCreateInfo(std::vector<DeviceQueueCreateInfo> &queues, std::vector<const char *> &layers, std::vector<const char *> &extensions) {
		sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		pNext = nullptr;
		flags = 0;
		queueCreateInfoCount = (uint32_t)queues.size();
		pQueueCreateInfos = &queues[0];
		enabledLayerCount = (uint32_t)layers.size();
		ppEnabledLayerNames = layers.empty() ? nullptr : &layers[0];
		enabledExtensionCount = (uint32_t)extensions.size();
		ppEnabledExtensionNames = extensions.empty() ? nullptr : &extensions[0];
		pEnabledFeatures = nullptr; // If specific features are required, pass them in here
	}
};

struct CommandPoolCreateInfo : public VkCommandPoolCreateInfo {
	CommandPoolCreateInfo(uint32_t queueIndex) {
		sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		pNext = nullptr;
		queueFamilyIndex = queueIndex;
		flags = 0;
	}
};

struct CommandBufferAllocateInfo : public VkCommandBufferAllocateInfo {
	CommandBufferAllocateInfo(VkCommandPool pool, uint32_t count = 1) {
		sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		pNext = nullptr;
		commandPool = pool;
		level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		commandBufferCount = count;
	}
};

struct CommandBufferInheritanceInfo : public VkCommandBufferInheritanceInfo {
	CommandBufferInheritanceInfo() {
		sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
		pNext = nullptr;
		renderPass = VK_NULL_HANDLE;
		subpass = 0;
		framebuffer = VK_NULL_HANDLE;
		occlusionQueryEnable = VK_FALSE;
		queryFlags = 0;
		pipelineStatistics = 0;
	}
};

struct CommandBufferBeginInfo : public VkCommandBufferBeginInfo {
	CommandBufferInheritanceInfo inheritance;

	CommandBufferBeginInfo(VkCommandBufferUsageFlags f = 0) {
		sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		pNext = nullptr;
		flags = f;
		pInheritanceInfo = &inheritance;
	}
};

struct Viewport : public VkViewport {
	Viewport(float w, float h, float _x = 0, float _y = 0, float min = 0, float max = 1) {
		x = _x;
		y = _y;
		width = w;
		height = h;
		minDepth = min;
		maxDepth = max;
	}
};

struct Extent2D : public VkExtent2D {
	Extent2D(uint32_t w = 1, uint32_t h = 1) {
		width = w;
		height = h;
	}
};

struct Offset2D : public VkOffset2D {
	Offset2D(int32_t _x = 0, int32_t _y = 0) {
		x = _x;
		y = _y;
	}
};

struct Rect2D : public VkRect2D {
	Rect2D(uint32_t w, uint32_t h, int32_t x = 0, int32_t y = 0) {
		offset = Offset2D(x, y);
		extent = Extent2D(w, h);
	}
};

struct Extent3D : public VkExtent3D {
	Extent3D(uint32_t w = 1, uint32_t h = 1, uint32_t d = 1) {
		width = w;
		height = h;
		depth = d;
	}
};

struct Offset3D : public VkOffset3D {
	Offset3D(int32_t _x = 0, int32_t _y = 0, int32_t _z = 0) {
		x = _x;
		y = _y;
		z = _z;
	}
};

struct ComponentMapping : public VkComponentMapping {
	ComponentMapping(VkComponentSwizzle _r = VK_COMPONENT_SWIZZLE_R, VkComponentSwizzle _g = VK_COMPONENT_SWIZZLE_G, VkComponentSwizzle _b = VK_COMPONENT_SWIZZLE_B, VkComponentSwizzle _a = VK_COMPONENT_SWIZZLE_A) {
		r = _r;
		g = _g;
		b = _b;
		a = _a;
	}
};

struct ImageSubresourceLayers : public VkImageSubresourceLayers {
	ImageSubresourceLayers(VkImageAspectFlags a = VK_IMAGE_ASPECT_COLOR_BIT, uint32_t mip = 0, uint32_t arr = 0, uint32_t layers = 1) {
		aspectMask = a;
		mipLevel = mip;
		baseArrayLayer = arr;
		layerCount = layers;
	}
};

struct ImageSubresourceRange : public VkImageSubresourceRange {
	ImageSubresourceRange(VkImageAspectFlags a = VK_IMAGE_ASPECT_COLOR_BIT, uint32_t mip = 0, uint32_t levels = 1, uint32_t arr = 0, uint32_t layers = 1) {
		aspectMask = a;
		baseMipLevel = mip;
		levelCount = levels;
		baseArrayLayer = arr;
		layerCount = layers;
	}
};

struct ImageCopy : public VkImageCopy {
	ImageCopy(uint32_t w, uint32_t h, uint32_t d = 1) {
		srcSubresource = ImageSubresourceLayers();
		srcOffset = Offset3D();
		dstSubresource = ImageSubresourceLayers();
		dstOffset = Offset3D();
		extent = Extent3D(w, h, d);
	}
};

struct ImageMemoryBarrier : public VkImageMemoryBarrier {
	ImageMemoryBarrier(VkImage i, VkAccessFlags src, VkAccessFlags dest, VkImageLayout _old, VkImageLayout _new, VkImageAspectFlags aspect = VK_IMAGE_ASPECT_COLOR_BIT) {
		sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		pNext = nullptr;
		srcAccessMask = src;
		dstAccessMask = dest;
		oldLayout = _old;
		newLayout = _new;
		srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		image = i;
		subresourceRange = ImageSubresourceRange(aspect);
	}
};

struct SwapchainCreateInfoKHR : public VkSwapchainCreateInfoKHR {
	SwapchainCreateInfoKHR(VkSurfaceKHR s, uint32_t min, VkFormat f, VkColorSpaceKHR c, uint32_t w, uint32_t h, VkSurfaceCapabilitiesKHR cap, VkPresentModeKHR present) {
		sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		pNext = nullptr;
		surface = s;
		minImageCount = min;
		imageFormat = f;
		imageColorSpace = c;
		imageExtent = Extent2D(w, h);
		imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		// NOTE: My current Vulkan driver validation complains that this bit isn't supported (even though it works great).
		// If I set it here, I see 1 error every time the swapchain is rebuilt. If I don't set it, I see 1 error every frame.
		//if ((cap.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT) != 0)
			imageUsage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		preTransform = cap.currentTransform;
		if ((cap.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) != 0)
			preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
		compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		imageArrayLayers = 1;
		imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		queueFamilyIndexCount = 0;
		pQueueFamilyIndices = nullptr;
		presentMode = present;
		oldSwapchain = nullptr;
		clipped = true;
	}
};

struct ImageViewCreateInfo : public VkImageViewCreateInfo {
	ImageViewCreateInfo(VkImage i, VkFormat f, VkImageAspectFlags a) {
		sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		pNext = nullptr;
		image = i;
		format = f;
		components = ComponentMapping();
		subresourceRange = ImageSubresourceRange(a);
		viewType = VK_IMAGE_VIEW_TYPE_2D;
		flags = 0;
	}
};

struct ImageCreateInfo : public VkImageCreateInfo {
	ImageCreateInfo(VkFormat f = VK_FORMAT_R8G8B8A8_UNORM, VkImageUsageFlags u = VK_IMAGE_USAGE_SAMPLED_BIT, uint32_t w = 1, uint32_t h = 1, uint32_t d = 1, uint32_t l = 1) {
		sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		pNext = nullptr;
		flags = 0;
		imageType = d > 1 ? VK_IMAGE_TYPE_3D : h > 1 ? VK_IMAGE_TYPE_2D : VK_IMAGE_TYPE_1D;
		format = f;
		extent = Extent3D(w, h, d);
		mipLevels = 1;
		arrayLayers = l;
		samples = VK_SAMPLE_COUNT_1_BIT;
		tiling = VK_IMAGE_TILING_OPTIMAL;
		usage = u;
		sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		queueFamilyIndexCount = 0;
		pQueueFamilyIndices = nullptr;
		initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	}
};

struct SubmitInfo : public VkSubmitInfo {
	SubmitInfo(VkCommandBuffer *command = nullptr, VkSemaphore *wait = nullptr, VkSemaphore *signal = nullptr) {
		sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		pNext = nullptr;
		waitSemaphoreCount = wait ? 1 : 0;
		pWaitSemaphores = wait;
		pWaitDstStageMask = nullptr;
		commandBufferCount = command ? 1 : 0;
		pCommandBuffers = command;
		signalSemaphoreCount = signal ? 1 : 0;
		pSignalSemaphores = signal;
	}
	SubmitInfo(std::vector<VkCommandBuffer> *commands = nullptr, std::vector<VkSemaphore> *wait = nullptr, std::vector<VkSemaphore> *signal = nullptr) {
		sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		pNext = nullptr;
		waitSemaphoreCount = wait ? (uint32_t)wait->size() : 0;
		pWaitSemaphores = waitSemaphoreCount ? &(*wait)[0] : nullptr;
		pWaitDstStageMask = nullptr;
		commandBufferCount = commands ? (uint32_t)commands->size() : 0;
		pCommandBuffers = commandBufferCount ? &(*commands)[0] : nullptr;
		signalSemaphoreCount = signal ? (uint32_t)signal->size() : 0;
		pSignalSemaphores = signalSemaphoreCount ? &(*signal)[0] : nullptr;
	}
};

struct PresentInfoKHR : public VkPresentInfoKHR {
	PresentInfoKHR(VkSwapchainKHR *swap, uint32_t *indices, std::vector<VkSemaphore> *wait = nullptr) {
		sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		pNext = nullptr;
		waitSemaphoreCount = wait ? (uint32_t)wait->size() : 0;
		pWaitSemaphores = waitSemaphoreCount ? &(*wait)[0] : nullptr;
		swapchainCount = 1;
		pSwapchains = swap;
		pImageIndices = indices;
		pResults = nullptr;
	}
};

struct SamplerCreateInfo : public VkSamplerCreateInfo {
	SamplerCreateInfo() {
		sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		pNext = nullptr;
		flags = 0;
		magFilter = VK_FILTER_LINEAR;
		minFilter = VK_FILTER_LINEAR;
		mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
		addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		mipLodBias = 0.0f;
		anisotropyEnable = VK_FALSE;
		maxAnisotropy = 1;
		compareEnable = VK_FALSE;
		compareOp = VK_COMPARE_OP_NEVER;
		minLod = 0.0f;
		maxLod = 0.0f;
		borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
		unnormalizedCoordinates = VK_FALSE;
	}
};

struct AttachmentDescription : public VkAttachmentDescription {
	AttachmentDescription(VkFormat f, VkSampleCountFlagBits s = VK_SAMPLE_COUNT_1_BIT, VkAttachmentLoadOp load = VK_ATTACHMENT_LOAD_OP_CLEAR, VkAttachmentStoreOp store = VK_ATTACHMENT_STORE_OP_STORE, VkImageLayout layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
		flags = 0;
		format = f;
		samples = s;
		loadOp = load;
		storeOp = store;
		stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		initialLayout = layout;
		finalLayout = layout;
	}
};

struct AttachmentReference : public VkAttachmentReference {
	AttachmentReference(uint32_t a = 0, VkImageLayout l = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
		attachment = a;
		layout = l;
	}
};

struct SubpassDescription : public VkSubpassDescription {
	SubpassDescription(std::vector<AttachmentReference> *colorAttachments = nullptr, AttachmentReference *depth = nullptr, VkSubpassDescriptionFlags f = 0) {
		flags = 0;
		pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		inputAttachmentCount = 0;
		pInputAttachments = nullptr;
		colorAttachmentCount = colorAttachments ? (uint32_t)colorAttachments->size() : 0;
		pColorAttachments = colorAttachmentCount ? &(*colorAttachments)[0] : nullptr;
		pResolveAttachments = nullptr;
		pDepthStencilAttachment = depth;
		preserveAttachmentCount = 0;
		pPreserveAttachments = nullptr;
	}
};

struct SubpassDependency : public VkSubpassDependency {
	// TODO: Implement when we need one of these
};

struct RenderPassCreateInfo : public VkRenderPassCreateInfo {
	RenderPassCreateInfo(std::vector<AttachmentDescription> *attachments = nullptr, std::vector<SubpassDescription> *subpasses = nullptr, std::vector<SubpassDependency> *dependencies = nullptr, VkRenderPassCreateFlags f = 0) {
		sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		pNext = nullptr;
		flags = f;
		attachmentCount = attachments ? (uint32_t)attachments->size() : 0;
		pAttachments = attachmentCount ? &(*attachments)[0] : nullptr;
		subpassCount = subpasses ? (uint32_t)subpasses->size() : 0;
		pSubpasses = subpassCount ? &(*subpasses)[0] : nullptr;
		dependencyCount = dependencies ? (uint32_t)dependencies->size() : 0;
		pDependencies = dependencyCount ? &(*dependencies)[0] : nullptr;
	}
};

struct FramebufferCreateInfo : public VkFramebufferCreateInfo {
	FramebufferCreateInfo(VkRenderPass pass, std::vector<VkImageView> *views = nullptr, VkFramebufferCreateFlags f = 0) {
		sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		pNext = nullptr;
		flags = f;
		renderPass = pass;
		attachmentCount = views ? (uint32_t)views->size() : 0;
		pAttachments = attachmentCount ? &(*views)[0] : nullptr;
		width = height = 0;
		layers = 1;
	}
};

struct ShaderModuleCreateInfo : public VkShaderModuleCreateInfo {
	ShaderModuleCreateInfo(std::string &code, VkShaderModuleCreateFlags f = 0) {
		sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		pNext = nullptr;
		flags = f;
		codeSize = code.size();
		pCode = reinterpret_cast<const uint32_t*>(&code[0]);
	}
	ShaderModuleCreateInfo(std::vector<uint32_t> &code, VkShaderModuleCreateFlags f = 0) {
		sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		pNext = nullptr;
		flags = f;
		codeSize = code.size() * sizeof(uint32_t);
		pCode = &code[0];
	}
};

struct PipelineShaderStageCreateInfo : public VkPipelineShaderStageCreateInfo {
	PipelineShaderStageCreateInfo(VkShaderStageFlagBits s, VkShaderModule m, VkPipelineShaderStageCreateFlags f = 0, const char *main = "main") {
		sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		pNext = nullptr;
		flags = f;
		stage = s;
		module = m;
		pName = main;
		pSpecializationInfo = nullptr;
	}
};

struct BufferCreateInfo : public VkBufferCreateInfo {
	BufferCreateInfo(VkDeviceSize s, VkBufferUsageFlags u) {
		sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		pNext = NULL;
		flags = 0;
		size = s;
		usage = u;
		sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		queueFamilyIndexCount = 0;
		pQueueFamilyIndices = NULL;
	}
};

struct VertexInputBindingDescription : public VkVertexInputBindingDescription {
	VertexInputBindingDescription(uint32_t b, uint32_t s, VkVertexInputRate r = VK_VERTEX_INPUT_RATE_VERTEX) {
		binding = b;
		stride = s;
		inputRate = r;
	}
};

struct VertexInputAttributeDescription : public VkVertexInputAttributeDescription {
	VertexInputAttributeDescription(uint32_t l=0, uint32_t b=0, VkFormat f=VK_FORMAT_R32G32B32A32_SFLOAT, uint32_t off=0) {
		location = l;
		binding = b;
		format = f;
		offset = off;
	}
};

struct PipelineVertexInputStateCreateInfo : public VkPipelineVertexInputStateCreateInfo {
	PipelineVertexInputStateCreateInfo(std::vector<VertexInputBindingDescription> *bindings = nullptr, std::vector<VertexInputAttributeDescription> *attributes = nullptr, VkPipelineVertexInputStateCreateFlags f = 0) {
		sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		pNext = nullptr;
		flags = f;
		vertexBindingDescriptionCount = bindings ? (uint32_t)bindings->size() : 0;
		pVertexBindingDescriptions = vertexBindingDescriptionCount ? &(*bindings)[0] : nullptr;
		vertexAttributeDescriptionCount = attributes ? (uint32_t)attributes->size() : 0;
		pVertexAttributeDescriptions = vertexAttributeDescriptionCount ? &(*attributes)[0] : nullptr;
	}
};

struct PipelineInputAssemblyStateCreateInfo : public VkPipelineInputAssemblyStateCreateInfo {
	PipelineInputAssemblyStateCreateInfo(VkPrimitiveTopology t = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VkBool32 restart = VK_FALSE, VkPipelineInputAssemblyStateCreateFlags f = 0) {
		sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		pNext = nullptr;
		flags = f;
		topology = t;
		primitiveRestartEnable = restart;
	}
};

struct PipelineViewportStateCreateInfo : public VkPipelineViewportStateCreateInfo {
	PipelineViewportStateCreateInfo() {
		sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		pNext = nullptr;
		flags = 0;
		viewportCount = 0;
		pViewports = nullptr;
		scissorCount = 0;
		pScissors = nullptr;
	}
};

struct PipelineRasterizationStateCreateInfo : public VkPipelineRasterizationStateCreateInfo {
	PipelineRasterizationStateCreateInfo(VkPolygonMode fill = VK_POLYGON_MODE_FILL, VkCullModeFlags cull = VK_CULL_MODE_BACK_BIT, VkFrontFace front = VK_FRONT_FACE_COUNTER_CLOCKWISE, VkPipelineRasterizationStateCreateFlags f = 0) {
		sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		pNext = nullptr;
		flags = f;
		depthClampEnable = VK_FALSE;
		rasterizerDiscardEnable = VK_FALSE;
		polygonMode = fill;
		cullMode = cull;
		frontFace = front;
		depthBiasEnable = VK_FALSE;
		depthBiasConstantFactor = 0.0f;
		depthBiasClamp = 0.0f;
		depthBiasSlopeFactor = 0.0f;
		lineWidth = 1.0f;
	}
};

struct PipelineMultisampleStateCreateInfo : public VkPipelineMultisampleStateCreateInfo {
	PipelineMultisampleStateCreateInfo(VkPipelineMultisampleStateCreateFlags f = 0) {
		sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		pNext = nullptr;
		flags = f;
		rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		sampleShadingEnable = VK_FALSE;
		minSampleShading = 1.0f;
		pSampleMask = nullptr;
		alphaToCoverageEnable = VK_FALSE;
		alphaToOneEnable = VK_FALSE;
	}
};

struct PipelineColorBlendAttachmentState : public VkPipelineColorBlendAttachmentState {
	PipelineColorBlendAttachmentState() {
		blendEnable = VK_FALSE;
		srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
		dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorBlendOp = VK_BLEND_OP_ADD;
		srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		alphaBlendOp = VK_BLEND_OP_ADD;
		colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	}
};

struct PipelineColorBlendStateCreateInfo : public VkPipelineColorBlendStateCreateInfo {
	PipelineColorBlendAttachmentState attachment;

	PipelineColorBlendStateCreateInfo() {
		sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		pNext = nullptr;
		flags = 0;
		logicOpEnable = VK_FALSE;
		logicOp = VK_LOGIC_OP_COPY;
		attachmentCount = 1;
		pAttachments = &attachment;
		blendConstants[0] = blendConstants[1] = blendConstants[2] = blendConstants[3] = 0.0f;
	}
};

struct PipelineLayoutCreateInfo : public VkPipelineLayoutCreateInfo {
	PipelineLayoutCreateInfo(VkDescriptorSetLayout *layouts = NULL, uint32_t count = 0, VkPipelineLayoutCreateFlags f = 0) {
		sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pNext = nullptr;
		flags = f;
		setLayoutCount = count;
		pSetLayouts = layouts;
		pushConstantRangeCount = 0;
		pPushConstantRanges = nullptr;
	}
};

struct RenderPassBeginInfo : public VkRenderPassBeginInfo {
	RenderPassBeginInfo(VkRenderPass pass, VkFramebuffer frame, Rect2D &rect, std::vector<VkClearValue> *clear = nullptr) {
		sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		pNext = nullptr;
		renderPass = pass;
		framebuffer = frame;
		renderArea = rect;
		clearValueCount = clear ? (uint32_t)clear->size() : 0;
		pClearValues = clearValueCount ? &(*clear)[0] : nullptr;
	}
};

struct PipelineDepthStencilStateCreateInfo : public VkPipelineDepthStencilStateCreateInfo {
	PipelineDepthStencilStateCreateInfo() {
		sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		pNext = nullptr;
		flags = 0;
		depthTestEnable = VK_TRUE;
		depthWriteEnable = VK_TRUE;
		depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
		depthBoundsTestEnable = VK_FALSE;
		stencilTestEnable = VK_FALSE;
		memset(&back, 0, sizeof(back));
		//back.failOp = VK_STENCIL_OP_KEEP; -- The memset already sets these to 0
		//back.passOp = VK_STENCIL_OP_KEEP;
		back.compareOp = VK_COMPARE_OP_ALWAYS;
		front = back;
		minDepthBounds = 0.0f;
		maxDepthBounds = 0.0f;
	}
};

struct PipelineDynamicStateCreateInfo : public VkPipelineDynamicStateCreateInfo {
	VkDynamicState enabled[VK_DYNAMIC_STATE_RANGE_SIZE];
	PipelineDynamicStateCreateInfo() {
		memset(enabled, 0, sizeof(enabled));
		sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		pNext = nullptr;
		flags = 0;
		dynamicStateCount = 0;
		pDynamicStates = enabled;
	}
};
struct PipelineTessellationStateCreateInfo : public VkPipelineTessellationStateCreateInfo {
	PipelineTessellationStateCreateInfo() {
		sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
		pNext = nullptr;
		flags = 0;
		patchControlPoints = 0;
	}
};

struct GraphicsPipelineCreateInfo : public VkGraphicsPipelineCreateInfo {
	PipelineVertexInputStateCreateInfo vertex;
	PipelineInputAssemblyStateCreateInfo inputAssembly;
	PipelineTessellationStateCreateInfo tesselation;
	PipelineViewportStateCreateInfo viewport;
	PipelineRasterizationStateCreateInfo rasterization;
	PipelineMultisampleStateCreateInfo multisample;
	PipelineDepthStencilStateCreateInfo depth;
	PipelineColorBlendStateCreateInfo blend;
	PipelineDynamicStateCreateInfo dynamic;

	GraphicsPipelineCreateInfo(VkRenderPass p, VkPipelineLayout l, std::vector<PipelineShaderStageCreateInfo> *stages = nullptr, VkPipelineCreateFlags f = 0) {
		sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pNext = nullptr;
		flags = f;
		stageCount = stages ? (uint32_t)stages->size() : 0;
		pStages = stageCount ? &(*stages)[0] : nullptr;
		pVertexInputState = &vertex;
		pInputAssemblyState = &inputAssembly;
		pTessellationState = &tesselation;
		pViewportState = &viewport;
		pRasterizationState = &rasterization;
		pMultisampleState = &multisample;
		pDepthStencilState = &depth;
		pColorBlendState = &blend;
		pDynamicState = &dynamic;
		layout = l;
		renderPass = p;
		subpass = 0;
		basePipelineHandle = VK_NULL_HANDLE;
		basePipelineIndex = -1;
	}
};

struct MemoryAllocateInfo : public VkMemoryAllocateInfo {
	MemoryAllocateInfo() {
		sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		pNext = nullptr;
		allocationSize = 0;
		memoryTypeIndex = 0;
	}

	MemoryAllocateInfo(VkMemoryRequirements &requirements, VkFlags requiredProps = 0) {
		sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		pNext = nullptr;
		allocationSize = requirements.size;
		memoryTypeIndex = 0;
		findMemoryType(requirements.memoryTypeBits, requiredProps, &memoryTypeIndex);
	}
};

struct DescriptorSetLayoutBinding : public VkDescriptorSetLayoutBinding {
	DescriptorSetLayoutBinding(uint32_t i, VkDescriptorType type, uint32_t count, VkShaderStageFlags flags) {
		binding = i;
		descriptorType = type;
		descriptorCount = count;
		stageFlags = flags;
		pImmutableSamplers = nullptr;
	}
};

struct DescriptorSetLayoutCreateInfo : public VkDescriptorSetLayoutCreateInfo {
	DescriptorSetLayoutCreateInfo(DescriptorSetLayoutBinding *bindings, uint32_t count=0) {
		sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		pNext = nullptr;
		flags = 0;
		bindingCount = count;
		pBindings = bindings;
	}
	DescriptorSetLayoutCreateInfo(std::vector<DescriptorSetLayoutBinding> *bindings = nullptr) {
		sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		pNext = nullptr;
		flags = 0;
		bindingCount = bindings ? (uint32_t)bindings->size() : 0;
		pBindings = bindingCount ? &(*bindings)[0] : nullptr;
	}
};

struct DescriptorPoolCreateInfo : public VkDescriptorPoolCreateInfo {
	DescriptorPoolCreateInfo(const VkDescriptorPoolSize *p = nullptr, uint32_t count = 0) {
		sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pNext = nullptr;
		flags = 0;
		maxSets = 0;
		poolSizeCount = count;
		pPoolSizes = p;
		for (uint32_t i = 0; i < count; i++)
			maxSets += p[i].descriptorCount;
	}
};

struct DescriptorSetAllocateInfo : public VkDescriptorSetAllocateInfo {
	DescriptorSetAllocateInfo(VkDescriptorPool pool, const VkDescriptorSetLayout *layout) {
		sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		pNext = nullptr;
		descriptorPool = pool;
		descriptorSetCount = 1;
		pSetLayouts = layout;
	}
};

struct WriteDescriptorSet : public VkWriteDescriptorSet {
	WriteDescriptorSet(VkDescriptorSet set, VkDescriptorBufferInfo *pInfo, VkDescriptorType type=VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, uint32_t b=0) {
		sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		pNext = NULL;
		dstSet = set;
		dstBinding = b;
		dstArrayElement = 0;
		descriptorCount = 1;
		descriptorType = type;
		pImageInfo = NULL;
		pBufferInfo = pInfo;
		pTexelBufferView = NULL;
	}
	WriteDescriptorSet(VkDescriptorSet set, VkDescriptorImageInfo *pInfo, VkDescriptorType type= VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, uint32_t b = 0) {
		sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		pNext = NULL;
		dstSet = set;
		dstBinding = b;
		dstArrayElement = 0;
		descriptorCount = 1;
		descriptorType = type;
		pImageInfo = pInfo;
		pBufferInfo = NULL;
		pTexelBufferView = NULL;
	}
};

} // namespace VK


#endif // __VKStruct_h__
