// VKManager.cpp
// This code is part of the VKContext library, an object-oriented class
// library designed to make Vulkan easier to use with object-oriented
// languages. It was designed and written by Sean O'Neil, who disclaims
// any copyright to release it in the public domain.
//

#include "VKCore.h"
#include "VKManager.h"

namespace VK {

void Manager::init() {
	// Load all shader techniques for the first time
	loadFX("GLManager.glfx"); // This FX file is required by this class
	updateShaders();
	m_dLastShaderUpdate = (float)VK::Timer::Time();

	// The Text2D shader and arial font are required for this Manager to function
	VK::ShaderProgram *pText = getTechnique("Text2D");
	if(!pText || !pText->isValid())
		VKLogException("Failed to load/compile Text2D technique!");
	if(!loadFont("arial1"))
		VKLogException("Failed to load arial font!");

	VkDescriptorPoolSize typeCounts[] = {
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 10 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 10 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 10 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 10 }
	};
	VK::DescriptorPoolCreateInfo poolInfo(typeCounts, 5);
	OBJ_CHECK(vkCreateDescriptorPool(vk, &poolInfo, NULL, &descriptorPool));

	sceneBuffer.create(sizeof(scene), descriptorPool);
	guiBuffer.create(sizeof(gui), descriptorPool, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_SHADER_STAGE_VERTEX_BIT);
	textBuffer.create(sizeof(text), descriptorPool, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_SHADER_STAGE_VERTEX_BIT);

	VkDescriptorSetLayout layouts[] = { sceneBuffer, guiBuffer, textBuffer };
	VK::PipelineLayoutCreateInfo pipelineLayoutInfo(layouts, 3);
	OBJ_CHECK(vkCreatePipelineLayout(vk, &pipelineLayoutInfo, NULL, &pipelineLayout));
}

void Manager::destroy() {
	textBuffer.destroy();
	guiBuffer.destroy();
	sceneBuffer.destroy();
	if (descriptorPool) {
		vkDestroyDescriptorPool(vk, descriptorPool, NULL);
		descriptorPool = NULL;
	}
	if (pipelineLayout) {
		vkDestroyPipelineLayout(vk, pipelineLayout, NULL);
		pipelineLayout = NULL;
	}

	m_mapFonts.clear();
	m_mapTechniques.clear();
	m_mapGLSLFiles.clear();
	m_mapGLFXFiles.clear();
}

void Manager::update(float fPeriodLength) {
#ifndef ANDROID // Android shaders require a new apk to be updated
	// Periodically check for updated shader files to reload
	double dTime = VK::Timer::Time();
	if(dTime - m_dLastShaderUpdate > fPeriodLength) {
		updateShaders();
		m_dLastShaderUpdate = dTime;
	}
#endif

	// Render any VK warnings on the screen using 2D text
	/*
	std::list<std::string> &lWarnings = vk.getWarningList();
	Font *pFont = getFont("arial1");
	if(pFont && !lWarnings.empty()) {
		pFont->setSize(12.0f);
		pFont->setColor(VK::vec4(1, 0, 0, 1));
		pFont->begin(getTechnique("Text2D"));
		std::list<std::string>::iterator it;
		float fFontSize = 18.0f;
		float fYPos = fFontSize * (lWarnings.size()-1);
		for(it = lWarnings.begin(); it != lWarnings.end(); it++) {
			pFont->draw2D(it->c_str(), VK::vec2(0.0f, fYPos), VK::vec2(fFontSize, fFontSize), Font::AlignXLeft, Font::AlignYBottom);
			fYPos -= fFontSize;
		}
		pFont->end(getTechnique("Text2D"));
	}
	*/
}

void Manager::cleanup() {
	// Destroy the pipelines for every technique we're managing (not just Text and GUI).
	for (std::map<std::string, VK::ShaderTechnique>::iterator it = m_mapTechniques.begin(); it != m_mapTechniques.end(); it++)
		it->second.destroyPipeline();
}

void Manager::reinit(RenderPass &guiPass, uint16_t nWidth, uint16_t nHeight) {
	scene.vSize.x = (float)nWidth;
	scene.vSize.y = (float)nHeight;
	scene.mProjection = VK::mat4::Perspective(m_fFOV, (float)nWidth/(float)nHeight, m_fNear, m_fFar);
	scene.mOrtho = VK::mat4::Ortho(0, nWidth, 0, nHeight, -1.0f, 1.0f);

	// We can only rebuild the Text and GUI pipelines because they're the only ones using this class's pipelineLayout.
	for (std::map<std::string, VK::ShaderTechnique>::iterator it = m_mapTechniques.begin(); it != m_mapTechniques.end(); it++) {
		if(it->second.isValid() && (it->first.substr(0, 3) == "GUI" || it->first.substr(0, 4) == "Text"))
			it->second.buildPipeline(guiPass, pipelineLayout);
	}
}

bool Manager::updateShaders() {
	bool bRebuild = false;
	Path::List::iterator itFile;
	Path::List lFiles;

#ifdef ANDROID
	lFiles = Path::Shader().apk_files();
#else
	lFiles = Path::Shader().files();
#endif
	
	for(itFile = lFiles.begin(); itFile != lFiles.end(); itFile++) {
#ifdef _WIN32
		_strlwr((char *)itFile->c_str());
#endif
		if(itFile->extension() == "glsl" || itFile->extension() == "h") {
			m_mapGLSLFiles[itFile->basename()].load(*itFile);
		}
	}

	std::map<std::string, FXFile>::iterator itFX;
	for(itFX = m_mapGLFXFiles.begin(); itFX != m_mapGLFXFiles.end(); itFX++) {
		// Check the fx file and all its dependencies to see if its techniques needs to be parsed again
		FXFile &fx = itFX->second;
		Path p = Path::Shader() + itFX->first;
		if(p.exists()) {
			bool bNeedsUpdate = fx.load(p);
			for(FXFile::iterator it = fx.begin(); !bNeedsUpdate && it != fx.end(); it++)
				bNeedsUpdate = m_mapGLSLFiles[it->basename()] != *it;
			if(!bNeedsUpdate)
				continue;

			if(!bRebuild) {
				bRebuild = true;
				//gl.getWarningList().clear();
			}
			fx.parse(vk, m_mapGLSLFiles, m_mapTechniques);
		}
	}
	return bRebuild;
}

bool Manager::loadFont(const char *pszName) {
	return m_mapFonts[pszName].load(VK::Path::Font().add("%s.glf", pszName));
}

} // namespace VK
