// VKImage.h
//

#ifndef __VKImage_h__
#define __VKImage_h__

#include "VKContext.h"

namespace VK {

class Image : public Object {
private:
	VkImage image;
	VkImageView view;
	VkDeviceMemory mem;
	VkImageLayout layout;
	ImageCreateInfo imageInfo;
	MemoryAllocateInfo allocInfo;

public:
	Image(VkImage h=NULL) : image(h), view(VK_NULL_HANDLE), mem(VK_NULL_HANDLE) {}
	~Image() { destroy();  }
	virtual void destroy();
	virtual bool isValid() const { return image != NULL; }

	void setLayout(VkImageAspectFlags aspect, VkImageLayout oldLayout, VkImageLayout newLayout);
	void createTexture(VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkFlags requiredProps, uint32_t width, uint32_t height = 1, uint32_t depth = 1, VkImageLayout iLayout = VK_IMAGE_LAYOUT_PREINITIALIZED, uint32_t layers = 1);
	void loadTexture(const char *path);
	void createDepth(uint32_t width, uint32_t height);

	/// Casting operators to provide easy access to any handle when you need to call a Vulkan function manually
	operator VkImage() const { return image; }
	operator VkImageView() const { return view; }
	operator VkDeviceMemory() const { return mem; }
	operator VkFormat() const { return imageInfo.format; }

	VkImageLayout getLayout() const { return layout; }
	const ImageCreateInfo &getImageInfo() const { return imageInfo; }
	const MemoryAllocateInfo &getAllocationInfo() const { return allocInfo; }
};

class ImageSampler : public Image {
private:
	VkDescriptorImageInfo descriptorInfo;
	VkDescriptorSetLayout descriptorSetLayout;
	VkDescriptorSet descriptorSet;

public:
	ImageSampler() : Image(), descriptorSet(NULL), descriptorSetLayout(NULL) {}
	virtual ~ImageSampler() { destroy(); }
	virtual bool isValid() const { return Image::isValid() && descriptorSet != NULL; }
	virtual void destroy() {
		if (descriptorSet) {
			// Descriptor pools often use reset instead of free
			descriptorSet = NULL;
		}
		if (descriptorSetLayout) {
			vkDestroyDescriptorSetLayout(vk, descriptorSetLayout, NULL);
			descriptorSetLayout = NULL;
		}
		if (descriptorInfo.sampler) {
			vkDestroySampler(vk, descriptorInfo.sampler, NULL);
			descriptorInfo.sampler = NULL;
		}

		Image::destroy();
	}

	operator VkImage() const { return Image::operator VkImage(); }
	operator VkImageView() const { return Image::operator VkImageView(); }
	operator VkDeviceMemory() const { return Image::operator VkDeviceMemory(); }
	operator VkFormat() const { return Image::operator VkFormat(); }
	operator VkSampler() const { return descriptorInfo.sampler; }
	operator VkDescriptorSetLayout() const { return descriptorSetLayout; }
	operator VkDescriptorSet() const { return descriptorSet; }

	void createDescriptor(VkDescriptorPool pool, VkShaderStageFlags flags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT) {
		VK::SamplerCreateInfo samplerInfo;
		descriptorInfo.imageLayout = getLayout();
		descriptorInfo.imageView = Image::operator VkImageView();
		OBJ_CHECK(vkCreateSampler(vk, &samplerInfo, NULL, &descriptorInfo.sampler));

		DescriptorSetLayoutBinding binding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, flags);
		DescriptorSetLayoutCreateInfo layoutInfo(&binding, 1);
		OBJ_CHECK(vkCreateDescriptorSetLayout(vk, &layoutInfo, NULL, &descriptorSetLayout));

		VK::DescriptorSetAllocateInfo setInfo(pool, &descriptorSetLayout);
		OBJ_CHECK(vkAllocateDescriptorSets(vk, &setInfo, &descriptorSet));

		WriteDescriptorSet write(descriptorSet, &descriptorInfo);
		vkUpdateDescriptorSets(vk, 1, &write, 0, NULL);
	}
};


} // namespace VK
#endif // __VKImage_h__
