// VKRenderPass.cpp
//

#include "VKRenderPass.h"

namespace VK {

void RenderPass::destroy() {
	if (frame) {
		vkDestroyFramebuffer(vk, frame, NULL);
		frame = NULL;
	}
	if (pass) {
		vkDestroyRenderPass(vk, pass, NULL);
		pass = NULL;
	}
}

void RenderPass::create(std::vector<Image*> &colorImages, Image *depth
	, VkAttachmentLoadOp colorLoadOp, VkAttachmentLoadOp depthLoadOp
	, VkAttachmentStoreOp colorStoreOp, VkAttachmentStoreOp depthStoreOp
) {
	if (colorImages.empty() && !depth)
		throw "A render pass needs at least one target image!";
	std::vector<VkImageView> views;
	std::vector<AttachmentDescription> attachments;
	std::vector<AttachmentReference> colorAttachments;
	for (uint32_t i = 0; i < (uint32_t)colorImages.size(); i++) {
		VkImageLayout layout = colorImages[i]->getLayout() == VK_IMAGE_LAYOUT_GENERAL ? VK_IMAGE_LAYOUT_GENERAL : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		views.push_back(*colorImages[i]);
		attachments.push_back(AttachmentDescription(*colorImages[i], VK_SAMPLE_COUNT_1_BIT, colorLoadOp, colorStoreOp, layout));
		colorAttachments.push_back(AttachmentReference(i, layout));
	}
	AttachmentReference depthAttachment((uint32_t)colorAttachments.size(), VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
	if (depth) {
		views.push_back(*depth);
		attachments.push_back(AttachmentDescription(*depth, VK_SAMPLE_COUNT_1_BIT, depthLoadOp, depthStoreOp, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL));
	}

	std::vector<SubpassDescription> subpasses = { SubpassDescription(&colorAttachments, depth ? &depthAttachment : NULL) };
	RenderPassCreateInfo renderPassInfo(&attachments, &subpasses);
	OBJ_CHECK(vkCreateRenderPass(vk, &renderPassInfo, nullptr, &pass));

	FramebufferCreateInfo framebufferInfo(pass, &views);
	const ImageCreateInfo &info = colorImages.empty() ? depth->getImageInfo() : colorImages[0]->getImageInfo();
	framebufferInfo.width = info.extent.width;
	framebufferInfo.height = info.extent.height;
	framebufferInfo.layers = info.imageType == VK_IMAGE_TYPE_3D ? info.extent.depth : info.arrayLayers;
	OBJ_CHECK(vkCreateFramebuffer(vk, &framebufferInfo, nullptr, &frame));
}

} // namespace VK
