// VKUniformBuffer.h
//
#ifndef __VKUniformBuffer_h__
#define __VKUniformBuffer_h__

#include "VKContext.h"

namespace VK {

class BufferObject : public Object {
private:
	VkBuffer buffer;
	MemoryAllocateInfo allocInfo;
	VkDeviceMemory mem;
	VkDeviceSize size;

public:
	BufferObject() : buffer(NULL), mem(NULL) {}
	virtual ~BufferObject() { destroy(); }
	virtual uint32_t getSize() const { return (uint32_t)size; }
	virtual bool isValid() const { return buffer != NULL; }
	virtual void destroy() {
		if (mem) {
			vkFreeMemory(vk, mem, NULL);
			mem = NULL;
		}
		if (buffer) {
			vkDestroyBuffer(vk, buffer, NULL);
			buffer = NULL;
		}
	}

	operator VkBuffer() const { return buffer; }
	operator VkDeviceMemory() const { return mem; }

	void create(VkBufferUsageFlags usage, VkDeviceSize size) {
		this->size = size;
		BufferCreateInfo info(size, usage);
		OBJ_CHECK(vkCreateBuffer(vk, &info, NULL, &buffer));
		VkMemoryRequirements requirements;
		vkGetBufferMemoryRequirements(vk, buffer, &requirements);
		allocInfo.allocationSize = requirements.size;

		bool pass = findMemoryType(requirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &allocInfo.memoryTypeIndex);
		OBJ_CHECK(vkAllocateMemory(vk, &allocInfo, NULL, &mem));
		OBJ_CHECK(vkBindBufferMemory(vk, buffer, mem, 0));
	}

	void update(void *src, VkDeviceSize offset = 0, VkDeviceSize bytes = -1) {
		if (bytes == -1)
			bytes = size - offset;
		void *dest = NULL;
		OBJ_CHECK(vkMapMemory(vk, mem, offset, bytes, 0, &dest));
		memcpy(dest, src, bytes);
		vkUnmapMemory(vk, mem);
	}
};


class UniformBuffer : public BufferObject {
private:
	VkDescriptorBufferInfo descriptorInfo;
	VkDescriptorSetLayout descriptorSetLayout;
	VkDescriptorSet descriptorSet;

public:
	UniformBuffer() : BufferObject(), descriptorSet(NULL), descriptorSetLayout(NULL) {}
	virtual ~UniformBuffer() { destroy(); }
	virtual bool isValid() const { return BufferObject::isValid() && descriptorSet != NULL; }
	virtual void destroy() {
		if (descriptorSet) {
			// Descriptor pools often use reset instead of free
			descriptorSet = NULL;
		}
		if (descriptorSetLayout) {
			vkDestroyDescriptorSetLayout(vk, descriptorSetLayout, NULL);
			descriptorSetLayout = NULL;
		}
		BufferObject::destroy();
	}

	operator VkBuffer() const { return BufferObject::operator VkBuffer(); }
	operator VkDeviceMemory() const { return BufferObject::operator VkDeviceMemory(); }
	operator VkDescriptorSetLayout() const { return descriptorSetLayout; }
	operator VkDescriptorSet() const { return descriptorSet; }

	void create(VkDeviceSize size, VkDescriptorPool pool = NULL, VkShaderStageFlags flags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT) {
		BufferObject::create(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, size);

		if (pool && flags != 0) {
			descriptorInfo.buffer = *this;
			descriptorInfo.offset = 0;
			descriptorInfo.range = size;

			DescriptorSetLayoutBinding binding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, flags);
			DescriptorSetLayoutCreateInfo layoutInfo(&binding, 1);
			OBJ_CHECK(vkCreateDescriptorSetLayout(vk, &layoutInfo, NULL, &descriptorSetLayout));

			VK::DescriptorSetAllocateInfo setInfo(pool, &descriptorSetLayout);
			OBJ_CHECK(vkAllocateDescriptorSets(vk, &setInfo, &descriptorSet));

			WriteDescriptorSet write(descriptorSet, &descriptorInfo);
			vkUpdateDescriptorSets(vk, 1, &write, 0, NULL);
		}
	}
};

} // namespace VK

#endif // __VKUniformBuffer_h__
