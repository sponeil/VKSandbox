// VKRenderPass.h
//
#ifndef __VKRenderPass_h__
#define __VKRenderPass_h__

#include "VKContext.h"
#include "VKImage.h"

namespace VK {

class RenderPass : public Object {
private:
	VkRenderPass pass;
	VkFramebuffer frame;

public:
	RenderPass() : pass(NULL), frame(NULL) {}
	~RenderPass() { destroy(); }
	virtual bool isValid() const { return pass != NULL && frame != NULL; }

	virtual void destroy();
	void create(std::vector<Image*> &colorImages, Image *depth
		, VkAttachmentLoadOp colorLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR, VkAttachmentLoadOp depthLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR
		, VkAttachmentStoreOp colorStoreOp = VK_ATTACHMENT_STORE_OP_STORE, VkAttachmentStoreOp depthStoreOp = VK_ATTACHMENT_STORE_OP_STORE
	);

	/// Casting operators to provide easy access to any handle when you need to call a Vulkan function manually
	operator VkRenderPass() const { return pass; }
	operator VkFramebuffer() const { return frame; }
};

} // namespace VK

#endif // __VKRenderPass_h__
