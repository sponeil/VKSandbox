// VKImage.cpp
//

#include "VKImage.h"
#include "VKPixelBuffer.h"

namespace VK {

void Image::destroy() {
	if (view) {
		vkDestroyImageView(vk, view, NULL);
		view = NULL;
	}
	if (mem) {
		vkFreeMemory(vk, mem, NULL);
		mem = NULL;
		if (image)
			vkDestroyImage(vk, image, NULL);
	}
	image = NULL;
}

void Image::setLayout(VkImageAspectFlags aspect, VkImageLayout oldLayout, VkImageLayout newLayout) {
	VkResult err = VK_SUCCESS;
	uint32_t srcAccessMask = 0, dstAccessMask = 0;
	switch (oldLayout) {
		case VK_IMAGE_LAYOUT_PREINITIALIZED:
			srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
			break;
		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
			srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			break;
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			break;
		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
			srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			break;
	}
	switch (newLayout) {
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			break;
		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
			dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			break;
		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
			dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			break;
		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
			dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			break;
		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
			dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
			break;
	}
	ImageMemoryBarrier barrier(image, srcAccessMask, dstAccessMask, oldLayout, newLayout, aspect);
	barrier.subresourceRange.layerCount = imageInfo.arrayLayers;
	vkCmdPipelineBarrier(vk, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0, NULL, 0, NULL, 1, &barrier);
	layout = newLayout;
}

void Image::createTexture(VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkFlags requiredProps, uint32_t width, uint32_t height, uint32_t depth, VkImageLayout iLayout, uint32_t layers) {
	imageInfo = ImageCreateInfo(format, usage, width, height, depth, layers);
	imageInfo.tiling = tiling;
	imageInfo.initialLayout = iLayout == VK_IMAGE_LAYOUT_PREINITIALIZED ? VK_IMAGE_LAYOUT_PREINITIALIZED : VK_IMAGE_LAYOUT_UNDEFINED;
	OBJ_CHECK(vkCreateImage(vk, &imageInfo, NULL, &image));

	VkMemoryRequirements requirements;
	vkGetImageMemoryRequirements(vk, image, &requirements);

	allocInfo = MemoryAllocateInfo(requirements, requiredProps);
	OBJ_CHECK(vkAllocateMemory(vk, &allocInfo, NULL, &mem));
	OBJ_CHECK(vkBindImageMemory(vk, image, mem, 0));
	if (iLayout != VK_IMAGE_LAYOUT_PREINITIALIZED && iLayout != VK_IMAGE_LAYOUT_UNDEFINED) {
		setLayout(VK_IMAGE_ASPECT_COLOR_BIT, imageInfo.initialLayout, iLayout);
	}

	if ((usage & (VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)) != 0) {
		ImageViewCreateInfo viewInfo(image, format, VK_IMAGE_ASPECT_COLOR_BIT);
		if (layers == 1) {
			viewInfo.viewType = depth > 1 ? VK_IMAGE_VIEW_TYPE_3D : height > 1 ? VK_IMAGE_VIEW_TYPE_2D : VK_IMAGE_VIEW_TYPE_1D;
		} else {
			viewInfo.subresourceRange.layerCount = layers;
			viewInfo.viewType = height > 1 ? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_1D_ARRAY;
		}
		OBJ_CHECK(vkCreateImageView(vk, &viewInfo, NULL, &view));
	}
}

void Image::loadTexture(const char *path) {
	PixelBuffer<uint8_t> pb;
	if(!pb.load(path))
		throw "Failed to load texture";

	VkFormatProperties &props = formatProperties[VK_FORMAT_R8G8B8A8_UNORM];
	bool direct = (props.linearTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT) != 0;

	VkImageSubresource subres = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0 };
	VkSubresourceLayout sublayout;
	createTexture(VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_LINEAR, direct ? VK_IMAGE_USAGE_SAMPLED_BIT : VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, pb.getWidth(), pb.getHeight());
	vkGetImageSubresourceLayout(vk, image, &subres, &sublayout);
	uint8_t *data = NULL;
	OBJ_CHECK(vkMapMemory(vk, mem, 0, allocInfo.allocationSize, 0, (void **)&data));

	uint32_t x = 0, y = 0;
	uint8_t temp[4] = {0, 0, 0, 255};
	for (y = 0; y < pb.getHeight(); y++) {
		uint32_t *dest = (uint32_t *)data;
		uint8_t *src = pb(0, y);
		switch (pb.getChannels()) {
			case 4:
				memcpy(dest, src, pb.getWidth() * 4);
				break;
			case 3:
				for (x = 0; x < pb.getWidth(); x++) {
					temp[0] = *src++; // R
					temp[1] = *src++; // G
					temp[2] = *src++; // B
					*dest++ = *(uint32_t *)temp;
				}
				break;
			case 2:
				for (x = 0; x < pb.getWidth(); x++) {
					temp[0] = *src++; // R
					temp[1] = *src++; // G
					*dest++ = *(uint32_t *)temp;
				}
				break;
			case 1:
				for (x = 0; x < pb.getWidth(); x++) {
					temp[0] = *src++; // R
					*dest++ = *(uint32_t *)temp;
				}
				break;
		}
		data += sublayout.rowPitch;
	}
	vkUnmapMemory(vk, mem);

	if (direct) {
		setLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_PREINITIALIZED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	} else {
		// Create a staging image visible to the host to load the texture into with linear tiling
		Image staging;
		Math::Swap(staging.image, image);
		Math::Swap(mem, staging.mem);
		Math::Swap(layout, staging.layout);
		Math::Swap(imageInfo, staging.imageInfo);
		Math::Swap(allocInfo, staging.allocInfo);
		staging.setLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_PREINITIALIZED, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

		// Now create the actual device-local image and copy into it from the staging image
		createTexture(VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL, (VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT), VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, pb.getWidth(), pb.getHeight());
		setLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_PREINITIALIZED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		ImageCopy copy_region(pb.getWidth(), pb.getHeight());
		vkCmdCopyImage(vk, staging.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_region);
		setLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		vk.flush(); // Wait for the copy command to complete before the staging texture goes out of scope!
	}

	ImageViewCreateInfo viewInfo(image, imageInfo.format, VK_IMAGE_ASPECT_COLOR_BIT);
	OBJ_CHECK(vkCreateImageView(vk, &viewInfo, NULL, &view));
}

void Image::createDepth(uint32_t width, uint32_t height) {
	imageInfo = ImageCreateInfo(VK_FORMAT_D16_UNORM, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, width, height);
	OBJ_CHECK(vkCreateImage(vk, &imageInfo, NULL, &image));

	VkMemoryRequirements requirements;
	vkGetImageMemoryRequirements(vk, image, &requirements);

	allocInfo = MemoryAllocateInfo(requirements);
	OBJ_CHECK(vkAllocateMemory(vk, &allocInfo, NULL, &mem));
	OBJ_CHECK(vkBindImageMemory(vk, image, mem, 0));
	setLayout(VK_IMAGE_ASPECT_DEPTH_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

	ImageViewCreateInfo viewInfo(image, imageInfo.format, VK_IMAGE_ASPECT_DEPTH_BIT);
	OBJ_CHECK(vkCreateImageView(vk, &viewInfo, NULL, &view));
}

} // namespace VK
