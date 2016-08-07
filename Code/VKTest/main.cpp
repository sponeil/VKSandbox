// main.cpp
//
#include "../VKContext/VKWindow.h"
#include "../VKContext/VKImage.h"
#include "../VKContext/VKRenderPass.h"
#include "../VKContext/VKShaderFile.h"
#include "../VKContext/VKBufferObject.h"
#include "../VKContext/VkTransform.h"
#include "../VKContext/VKPixelBuffer.h"
#include "../VKContext/VKFont.h"
#include "../VKContext/VKManager.h"
#include "../VKContext/VKGeometry.h"
#include "../VKContext/VKNoise.h"

#include "CubeFace.h"

#include <random>

/*
* Plate tectonics (low res):
* 1) Create a height map (cube) for planet crust simulations.
* 2) A vec4 could store crust mass, density, age of last mass change, and plate#.
* 3) Start with a random super-continent or two?
* 4) Plates could be determined by selecting random roughly equidistant points on the sphere.
* 5) Plate edges could follow a slightly randomized Voronoi path.
* 6) Each plate can be given random directions and plate tectonics could be simulated by shifting crust mass/density.
* 7) The combined mass and density would determine height, and age would determine level of erosion or roughness.
* 8) Generate random hotspots and volcanoes along edges to add lighter crust mass?
*
* River formation (medium res):
* 1) Determine a sea level and increase density of everything below it, or should sea level be involved in above calculations?
* 2) Use various mass, density, and age amounts to determine main river paths.
* 3) Build out a river drainage system as laid out in the PDF.
* 4) Build out a final height map that also contains some short-hand info on the drainage network?
* 5) Save to a SQLite db with mipmap levels to be loaded later, or generate every time?
* NOTE: There should probably be no peaks themselves centered at texels in the height map.
* The peaks should be between them with water-carved paths leading down to them.	
*
* Procedural details (high res):
* 1) Would this just be noise for peaks leading down to the river drainage network?
* 2) Roughness would be controlled by crust mass amount and age.
*/

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_real_distribution<float> real_random(-1, 1);

#ifdef _DEBUG
#define BINARY_PREFIX "_D"
#else
#define BINARY_PREFIX ""
#endif

#pragma comment(lib, BINARY_PREFIX "VKContext.lib")
#pragma comment(lib, BINARY_PREFIX "glslang.lib")

#define FONT_NAME "techno1"


struct Window : public VK::Window {
private:
	VK::Transform<float> camera;
	VK::vec3 velocity;

	double lastLogTime;
	uint32_t frameCount;
	float m_fFrameTime;
	double m_dLastFrame;

	VK::Manager manager;
	VK::RenderPass planetPass; // Render pass for updating the planet's height map
	VK::RenderPass graphicsPass; // Render pass for drawing to the main framebuffer(s)
	VK::RenderPass guiPass; // Render pass for swapping the main framebuffer to the main UI window
	VK::Image depth, color, normal;

	static const int MaxLevels = MAX_LEVELS; // More than 16 levels experiences precision problems in shaders
	static const int MaxPlanets = 64 - MaxLevels; // This is the max number of planets/moons we can render in a single frame
	static const int NodeWidth = 128; // 64x64 quads per level
	static const int NodeEdge = NodeWidth + 1; // 64x64 quads requires 65x65 vertices
	static const int NodeHalf = NodeWidth / 2; // This value is used a lot
	static const int NodeFourth = NodeWidth / 4; // This value is used a lot
	static const int HeightMapFactor = 2; // For better normals, take 4 height samples per vertex
	static const int HeightMapWidth = NodeWidth * HeightMapFactor + 1;
	static const int HeightMapLayers = MaxPlanets + MaxLevels;

	//enum Orientation { Leaf, Center, NorthEdge, SouthEdge, WestEdge, EastEdge, NWCorner, NECorner, SWCorner, SECorner, OrientationCount };
	//static uint32_t m_nIndex[OrientationCount]; // A global array of offsets into an index buffer (one for each orientation)

	static uint32_t m_nStack[HeightMapLayers]; // A global stack of unused height map indices (in the texture array)
	static uint32_t m_nStackIndex; // A global index into the stack of unused height map indices

	bool m_bUpdate;

	/*
	struct Level {
		uint32_t m_nTex;			// The layer of the height map texture array assigned to this level
		Orientation m_nOrientation;	// The current orientation of this level's child (determines which quads get rendered)
		VK::ivec2 m_ivPos;			// The current center point of this level (the point on the sphere closest to the camera)
	};
	Level m_level[6][MaxLevels]; // Tracks where each level is on this sphere
	*/
	VK::PlanetFaceData faceData[MAX_LEVELS * FaceCount];
	VK::UniformBuffer faceBuffer;

	VkPipelineLayout sceneOnlyLayout, pipelineLayout;
	VK::BufferObject vboClipmap, iboClipmap;

public:
	VK::PixelBuffer<float> pbHeight[6];
	VK::ImageSampler iHeight, iHeightHost;

	Window() {}

	virtual void onCreate() {
		VK::Window::onCreate();
		camera.pos = VK::vec3(0, 0, 2.5f);
		velocity = VK::vec3(0, 0, 0);

		m_bUpdate = false;

		// Corner with cracks position:
		//camera.from_s("t[1.000000, q[-0.104479, -0.401035, -0.266681, 0.870136], v[-1.085328, 0.952729, 1.114721]]");

		manager.init();
		manager.setNear(0.001f);
		manager.setFar(100.0f);
		manager.loadFont(FONT_NAME);
		manager.loadFX("VKTest.glfx");
		manager.updateShaders();

		faceBuffer.create(sizeof(faceData), manager.getDescriptorPool());

		iHeight.createTexture(
			VK_FORMAT_R32G32B32A32_SFLOAT,
			VK_IMAGE_TILING_OPTIMAL, // Can't use linear with device-local texture arrays
			VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			TestWidth, TestWidth, 1,
			VK_IMAGE_LAYOUT_UNDEFINED, // Don't need to keep the contents of this when we change layouts
			6
		);
		iHeightHost.createTexture(
			VK_FORMAT_R32G32B32A32_SFLOAT,
			VK_IMAGE_TILING_LINEAR,
			VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			TestWidth, TestWidth, 1,
			VK_IMAGE_LAYOUT_PREINITIALIZED, // Need to keep the contents of this when we change layouts
			1 // My ATI allowed 6 layers here, but not my nVidia
		);

#if 1 // Can't do this if it's not linear
		iHeightHost.setLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_PREINITIALIZED, VK_IMAGE_LAYOUT_GENERAL);
		iHeight.setLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		uint8_t *data = NULL;
		VkImageSubresource subres = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0 };
		VkSubresourceLayout sublayout;
		for (int face = 0; face < 6; face++) {
			//subres.arrayLayer = face;
			vkGetImageSubresourceLayout(vk, iHeightHost, &subres, &sublayout);
			if (VK_SUCCESS == vkMapMemory(vk, iHeightHost, sublayout.offset, sublayout.size, 0, (void **)&data)) {
				for (int y = 0; y < pbHeight[face].getHeight(); y++) {
					float *src = pbHeight[face](0, y);
					memcpy(data, src, pbHeight[face].getWidth() * pbHeight[face].getChannels() * 4);
					data += sublayout.rowPitch;
				}
				vkUnmapMemory(vk, iHeightHost);
			}
			VK::ImageCopy copy_region(TestWidth, TestWidth);
			copy_region.dstSubresource.baseArrayLayer = face;
			iHeightHost.setLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
			vkCmdCopyImage(vk, iHeightHost, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, iHeight, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_region);
			iHeightHost.setLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL);
			vk.flush();
		}
		iHeight.setLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL);
#endif

		this->onSize(m_nWidth, m_nHeight);

		iHeight.createDescriptor(manager.getDescriptorPool());
		std::vector<VK::Image*> planetImages = { &iHeight };
		planetPass.create(planetImages, NULL, VK_ATTACHMENT_LOAD_OP_LOAD);
		VK::ShaderTechnique *pPlanet = manager.getTechnique("TweakPlanet");
		if (pPlanet) {
			VkDescriptorSetLayout layouts[] = { manager.getSceneBuffer() };
			VK::PipelineLayoutCreateInfo pipelineLayoutInfo(layouts, 1);
			VkPushConstantRange push_constant_ranges[1] = {};
			push_constant_ranges[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
			push_constant_ranges[0].offset = 0;
			push_constant_ranges[0].size = 16;
			pipelineLayoutInfo.pushConstantRangeCount = 1;
			pipelineLayoutInfo.pPushConstantRanges = push_constant_ranges;
			vkCreatePipelineLayout(vk, &pipelineLayoutInfo, NULL, &sceneOnlyLayout);
			pPlanet->buildPipeline(planetPass, sceneOnlyLayout);
		}

		// Build the vertex array
		std::vector<VK::vec4> vertices(NodeEdge*NodeEdge);
		std::vector<uint16_t> indices;
		indices.reserve(64*1024);
		int n = 0;
		float y = 0.0f; // y goes from top to bottom
		for (int i = 0; i<NodeEdge; i++) {
			float yy = y*y;
			float x = 0.0f; // x goes from left to right
			for (int j = 0; j<NodeEdge; j++) {
				vertices[n++] = VK::vec4(x, y, (float)j, (float)i);
				x += 1.0f / NodeWidth;
			}
			y += 1.0f / NodeWidth;
		}
		vboClipmap.create(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, sizeof(VK::vec4) * vertices.size());
		vboClipmap.update(&vertices[0]);

		// The top row skips odd edge vertices to avoid cracks in mesh at border with parent node
		n = 0;
		for (int i = 0; i < NodeWidth; i+=2) {
			if (i > 0) {
				indices.push_back(n);
				indices.push_back(n + NodeEdge);
				indices.push_back(n + NodeEdge + 1);
			}
			indices.push_back(n+2);
			indices.push_back(n);
			indices.push_back(n + NodeEdge + 1);
			if (i < NodeWidth-2) {
				indices.push_back(n + 2);
				indices.push_back(n + NodeEdge + 1);
				indices.push_back(n + NodeEdge + 2);
			}
			n += 2;
		}
		n++;

		for (int i = 1; i < NodeWidth-1; i++) {
			for (int j = 0; j < NodeWidth; j++) {
				n++;
	
				if (j == 0) {
					if ((i & 1) != 0) {
						indices.push_back(n);
						indices.push_back(n - NodeEdge - 1);
						indices.push_back(n + NodeEdge - 1);
						indices.push_back(n);
						indices.push_back(n + NodeEdge - 1);
						indices.push_back(n + NodeEdge);
					} else {
						indices.push_back(n);
						indices.push_back(n-1);
						indices.push_back(n + NodeEdge);
					}
				} else if (j == NodeWidth - 1) {
					if ((i & 1) != 0) {
						indices.push_back(n - NodeEdge);
						indices.push_back(n - 1);
						indices.push_back(n + NodeEdge);
						indices.push_back(n - 1);
						indices.push_back(n + NodeEdge - 1);
						indices.push_back(n + NodeEdge);
					} else {
						indices.push_back(n);
						indices.push_back(n - 1);
						indices.push_back(n + NodeEdge - 1);
					}
				} else {
					indices.push_back(n);
					indices.push_back(n - 1);
					indices.push_back(n + NodeEdge);

					indices.push_back(n - 1);
					indices.push_back(n - 1 + NodeEdge);
					indices.push_back(n + NodeEdge);
				}
			}
			n++;
		}

		// The bottom row skips odd edge vertices to avoid cracks in mesh at border with parent node
		indices.push_back(n+1);
		indices.push_back(n - NodeEdge);
		indices.push_back(n + NodeEdge);
		for (int i = 0; i < NodeWidth; i += 2) {
			if (i > 0) {
				indices.push_back(n + 1);
				indices.push_back(n);
				indices.push_back(n + NodeEdge);
			}
			indices.push_back(n + 1);
			indices.push_back(n + NodeEdge);
			indices.push_back(n + NodeEdge + 2);
			if (i < NodeWidth - 2) {
				indices.push_back(n + 2);
				indices.push_back(n + 1);
				indices.push_back(n + NodeEdge + 2);
			}
			n += 2;
		}
		indices.push_back(n - NodeEdge);
		indices.push_back(n - 1);
		indices.push_back(n + NodeEdge);

		iboClipmap.create(VK_BUFFER_USAGE_INDEX_BUFFER_BIT, indices.size()*sizeof(uint16_t));
		iboClipmap.update(&indices[0]);

		VkDescriptorSetLayout layouts[] = { manager.getSceneBuffer(), faceBuffer, iHeight };
		VK::PipelineLayoutCreateInfo pipelineLayoutInfo(layouts, 3);
		vkCreatePipelineLayout(vk, &pipelineLayoutInfo, NULL, &pipelineLayout);
	}

	virtual void onDestroy() {
		if (sceneOnlyLayout) {
			vkDestroyPipelineLayout(vk, sceneOnlyLayout, NULL);
			sceneOnlyLayout = NULL;
		}

		manager.cleanup();
		guiPass.destroy();
		graphicsPass.destroy();
		planetPass.destroy();
		depth.destroy();
		color.destroy();
		normal.destroy();

		iHeightHost.destroy();
		iHeight.destroy();

		if (pipelineLayout) {
			vkDestroyPipelineLayout(vk, pipelineLayout, NULL);
			pipelineLayout = NULL;
		}
		iboClipmap.destroy();
		vboClipmap.destroy();
		faceBuffer.destroy();
		manager.destroy();
	}

	virtual void onSize(unsigned short nWidth, unsigned short nHeight) {
		double t = VK::Timer::Time();
		VK::Window::onSize(nWidth, nHeight);

		// Destroy everything that relies on the swapchain
		manager.cleanup();
		guiPass.destroy();
		graphicsPass.destroy();
		depth.destroy();
		color.destroy();
		normal.destroy();

		// Rebuild the swapchain
		vk.buildSwapchain(nWidth, nHeight);

		// Rebuild everything that relies on the swapchain
		depth.createDepth(nWidth, nHeight);
		color.createTexture(vk.getSurfaceFormat().format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, nWidth, nHeight, 1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		normal.createTexture(VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, nWidth, nHeight, 1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

		color.setLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
		normal.setLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

		std::vector<VK::Image*> graphicsImages = { &color, &normal };
		graphicsPass.create(graphicsImages, &depth, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_LOAD_OP_CLEAR);

		// The gui pass only writes to the primary color buffer (some techniques use the depth buffer)
		std::vector<VK::Image*> guiImages = { &color };
		guiPass.create(guiImages, &depth, VK_ATTACHMENT_LOAD_OP_LOAD, VK_ATTACHMENT_LOAD_OP_LOAD);

		manager.reinit(guiPass, nWidth, nHeight);
		VK::ShaderTechnique *pPlanet = manager.getTechnique("TweakPlanet");
		if (pPlanet)
			pPlanet->buildPipeline(planetPass, sceneOnlyLayout);

		VK::ShaderTechnique *pPlanetFace = manager.getTechnique("PlanetFace");
		if(pPlanetFace)
			pPlanetFace->buildPipeline(graphicsPass, pipelineLayout);

		lastLogTime = VK::Timer::Time();
		frameCount = 0;
		m_fFrameTime = 0.0f;
		m_dLastFrame = lastLogTime;
		VKLogNotice("onSize - %lf seconds", VK::Timer::Time() - t);
	}

	virtual void onIdle() {
		// Get the current time and the amount of time it took to draw the previous frame
		double dNow = VK::Timer::Time();
		m_fFrameTime = (float)(dNow - m_dLastFrame);
		m_dLastFrame = dNow;

		// Update the camera position
		//manager.update(1.0f);
		updateCamera();
		manager.setViewMatrix(camera.viewMatrix()); //camera.relativeViewMatrix();

		// Determine the patches we need to draw for the planet
		VK::vec3 v = camera.pos - VK::vec3(0, 0, 0); // Camera pos relative to planet (currently at origin)
		int ix, iy; // (0,0) = NW corner and (1<<24, 1<<24) = SE corner
		int front = CubeFace::GetFaceCoordinates(CubeFace::to_i(v), ix, iy);
		double dx = CubeFace::to_f(ix), dy = CubeFace::to_f(iy); // (0,0) = NW corner and (1,1) = SE corner
		float dist = camera.pos.mag() - 1.0f;

		// Calculate the smallest delta needed to shift the bottom level:
		// If level 0 has 64x64 quads and there are 16 levels, the bottom level has (64<<15) x (64<<15) quads.
		// However, x and y values go from -1.0 to 1.0, so divide by 2 to get 32 on each side of 0.0.
		// Then divide by 2 again because we can't shift by 1 quad. We have to shift by 1 parent quad (a block of 2x2 in the child).
		const int MaxScale = NodeWidth << (MaxLevels - 2);
		VK::ivec2 frontPos((int)(dx * MaxScale), (int)(dy * MaxScale)); // Cast to fixed-precision int based on max depth

		int instance = 0;
		int levels = MaxLevels;
		uint8_t neighborLevels[MaxLevels] = { 0 };
		faceData[instance].vHole = VK::vec4(-1, -1, -1, -1); // Instance 0 is the lowest level, so it has no hole.
		for (int level = levels - 1, factor = 1; level > 0; level--, factor <<= 1) {
			float f = 1.0f * powf(0.5f, (float)level);
			VK::ivec2 iPos = frontPos / factor;
			faceData[instance].iFace = VK::ivec4(front, level, 0, 0);
			faceData[instance].vCorners = VK::vec4(((float)(iPos.x - NodeFourth) / NodeHalf)*f, ((float)(iPos.y - NodeFourth) / NodeHalf)*f, ((float)(iPos.x + NodeFourth) / NodeHalf)*f, ((float)(iPos.y + NodeFourth) / NodeHalf)*f);

			if (dist > f*2.0f)
				continue;

			// Set bit flag for which neighbors have to render levels deeper than level 0
			if (faceData[instance].vCorners.x < -0.001f)
				neighborLevels[level] |= 1 << LeftEdge;
			if (faceData[instance].vCorners.y < -0.001f)
				neighborLevels[level] |= 1 << TopEdge;
			if (faceData[instance].vCorners.z > 1.001f)
				neighborLevels[level] |= 1 << RightEdge;
			if (faceData[instance].vCorners.w > 1.001f)
				neighborLevels[level] |= 1 << BottomEdge;

			// When the camera is directly over the center of a face, all holes in parent nodes for rendering child nodes are centered.
			// Imagine the leaf moving 1 quad to the right. None of the parent nodes move. The hole inside its parent shifts right 1.
			// Imagine the leaf moving to to the right again, causing the parent to shift right 1. This puts the hole for the leaf
			// node back to the center of its parent, and the hole in the grand-parent has to shift right 1 for the parent node.
			// So for most levels, the hole is always either dead center or off by 1 in any direction (N, S, E, W, NE, SE, SW, NW).
			// However, level 0 doesn't move, so the hole in it for level 1 can move ANYWHERE within its parent. It can even go off
			// the edge of the cube face (it needs to be rendered on the neighboring face to avoid cracks in the mesh).
			int xoff = 0, yoff = 0;
			if (level == 1) {
				xoff = iPos.x - NodeHalf;
				yoff = iPos.y - NodeHalf;
			} else {
				xoff = (iPos.x & 1) == 0 ? 0 : iPos.x > 0 ? 1 : -1;
				yoff = (iPos.y & 1) == 0 ? 0 : iPos.y > 0 ? 1 : -1;
			}

			// We use calculations from each child node to calculate the hole for its parent node
			// The hole is based on vertex position, which goes from (0,0) in the NW corner to (64, 64) in the SE corner
			faceData[++instance].vHole = VK::vec4((float)(NodeFourth + xoff), (float)(NodeFourth + yoff), (float)(3 * NodeFourth + xoff), (float)(3 * NodeFourth + yoff));
		}
		faceData[instance].iFace = VK::ivec4(front, 0, 0, 0);
		faceData[instance].vCorners = VK::vec4(0, 0, 1, 1); // Level 0 never moves. It always goes from (0,0) to (1,1)
		++instance;

		// In addition to the face beneath the camera, we need to render its 4 neighbors. We do not need to render the opposite face.
		// In theory from some angles we can get away with rendering 3 neighbor faces instead of 4, but the performance savings may
		// not be enough to justify the added complexity.
		uint8_t neighborFace[4];
		VK::ivec2 neighborPos[4];
		for (int edge = 0; edge < 4; edge++) {
			neighborFace[edge] = front;
			double x = dx, y = dy;
			CubeFace::GetNeighborCoordinates(edge, neighborFace[edge], x, y);
			neighborPos[edge] = VK::ivec2((int)(x * MaxScale), (int)(y * MaxScale)); // Cast to fixed-precision int based on max depth
		}

		for (int edge = 0; edge < 4; edge++) {
			uint8_t face = neighborFace[edge];

			faceData[instance].vHole = VK::vec4(-1, -1, -1, -1); // The lowest level (or leaf) of any neighboring face has no hole in the middle
			for (int level = MaxLevels - 1, factor = 1; level > 0; level--, factor <<= 1) {
				if ((neighborLevels[level] & (1 << edge)) == 0)
					continue;

				float f = 1.0f * powf(0.5f, (float)level);
				VK::ivec2 iPos = neighborPos[edge] / factor;
				VK::vec4 test = VK::vec4(((float)(iPos.x - NodeFourth) / NodeHalf)*f, ((float)(iPos.y - NodeFourth) / NodeHalf)*f, ((float)(iPos.x + NodeFourth) / NodeHalf)*f, ((float)(iPos.y + NodeFourth) / NodeHalf)*f);

				// Check to see if any other neighbor is also rendering this level.
				// If so and it comes closer to the center of that face, we need to extend it farther
				// out on this face to avoid cracks in the mesh at the corners where 3 cube faces meet.
				for (int edge2 = 0; edge2 < 4; edge2++) {
					if (edge2 == edge || (neighborLevels[level] & (1 << edge2)) == 0)
						continue;
					const int Mid = MaxScale / factor / 2;
					VK::ivec2 iPos2 = neighborPos[edge2] / factor;
					int ox = VK::Math::Abs(Mid - iPos.x); // Find how far outside the current face x is
					int oy = VK::Math::Abs(Mid - iPos.y); // Find how far outside the current face y is
					int ox2 = VK::Math::Abs(Mid - iPos2.x); // Do the same for this neighbor
					int oy2 = VK::Math::Abs(Mid - iPos2.y); // Do the same for this neighbor
					int max2 = VK::Math::Max(ox2, oy2);
					if (ox > oy && max2 < ox) {
						iPos.x -= iPos.x > Mid ? (ox - max2) : (max2 - ox);
					} else if (oy > ox && max2 < oy) {
						iPos.y -= iPos.y > Mid ? (oy - max2) : (max2 - oy);
					}
					test = VK::vec4(((float)(iPos.x - NodeFourth) / NodeHalf)*f, ((float)(iPos.y - NodeFourth) / NodeHalf)*f, ((float)(iPos.x + NodeFourth) / NodeHalf)*f, ((float)(iPos.y + NodeFourth) / NodeHalf)*f);
				}

				// Yes, I know copy+paste code is bad
				int xoff = 0, yoff = 0;
				if (level == 1) {
					xoff = iPos.x - NodeHalf;
					yoff = iPos.y - NodeHalf;
				} else {
					xoff = (iPos.x & 1) == 0 ? 0 : iPos.x > 0 ? 1 : -1;
					yoff = (iPos.y & 1) == 0 ? 0 : iPos.y > 0 ? 1 : -1;
				}
				faceData[instance].iFace = VK::ivec4(face, level, 0, 0);
				faceData[instance].vCorners = test;
				faceData[++instance].vHole = VK::vec4((float)(NodeFourth + xoff), (float)(NodeFourth + yoff), (float)(3 * NodeFourth + xoff), (float)(3 * NodeFourth + yoff));
			}

			// Finally we finish up level 0 for this neighbor face
			faceData[instance].iFace = VK::ivec4(face, 0, 0, 0);
			faceData[instance].vCorners = VK::vec4(0, 0, 1, 1); // Level 0 never moves. It always goes from (0,0) to (1,1)
			++instance;
		}

		color.setLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		normal.setLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

		VK::ShaderTechnique *p;

		if (m_bUpdate && (p = manager.getTechnique("TweakPlanet")) != NULL) {
			VK::Plane plane;
			VK::vec3 normal(real_random(gen), real_random(gen), real_random(gen));
			float f = real_random(gen);
			plane.init(normal.normalize(), f * 0.5f);
			VK::vec4 v(plane.normal, plane.constant);
			vkCmdPushConstants(vk, sceneOnlyLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, 16, &v);
			
			VK::Viewport viewport((float)TestWidth, (float)TestWidth);
			VK::Rect2D scissor(TestWidth, TestWidth);
			VK::RenderPassBeginInfo planetBegin(planetPass, planetPass, scissor);
			vkCmdBeginRenderPass(vk, &planetBegin, VK_SUBPASS_CONTENTS_INLINE);
			vkCmdSetScissor(vk, 0, 1, &scissor);
			vkCmdSetViewport(vk, 0, 1, &viewport);

			VkDescriptorSet descriptorSets[] = { manager.getSceneBuffer() };
			vkCmdBindDescriptorSets(vk, VK_PIPELINE_BIND_POINT_GRAPHICS, sceneOnlyLayout, 0, 1, descriptorSets, 0, NULL);
			vkCmdBindPipeline(vk, VK_PIPELINE_BIND_POINT_GRAPHICS, *p);
			vkCmdDraw(vk, 6, 1, 0, 0);
			vkCmdEndRenderPass(vk);
		}
		vk.flush();

		std::vector<VkClearValue> clearValues = { { 0.0f, 0.1f, 0.0f, 0.0f },{ 0.0f, 0.0f, 0.0f, 0.0f },{ 1.0f, 0 } };

		VkCommandBuffer cmd;
		VK::CommandBufferAllocateInfo bufferInfo(vk, 1);
		VK::CommandBufferBeginInfo cmdBeginInfo(0);
		vkAllocateCommandBuffers(vk, &bufferInfo, &cmd);
		vkBeginCommandBuffer(cmd, &cmdBeginInfo);

		VK::Viewport viewport(m_nWidth, m_nHeight);
		VK::Rect2D scissor(m_nWidth, m_nHeight);
		vkCmdSetScissor(cmd, 0, 1, &scissor);
		vkCmdSetViewport(cmd, 0, 1, &viewport);

		VK::RenderPassBeginInfo graphicsBegin(graphicsPass, graphicsPass, scissor, &clearValues);
		vkCmdBeginRenderPass(cmd, &graphicsBegin, VK_SUBPASS_CONTENTS_INLINE);

		if (p = manager.getTechnique("PlanetFace")) {
			VkDeviceSize offsets[1] = { 0 };
			VkBuffer buffers[] = { vboClipmap };
			vkCmdBindVertexBuffers(cmd, 0, 1, buffers, offsets);
			vkCmdBindIndexBuffer(cmd, iboClipmap, 0, VK_INDEX_TYPE_UINT16);

			VkDescriptorSet descriptorSets[] = { manager.getSceneBuffer(), faceBuffer, iHeight };
			vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 3, descriptorSets, 0, NULL);

			vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, *p);
			vkCmdDrawIndexed(cmd, iboClipmap.getSize() / sizeof(uint16_t), instance, 0, 0, 0);
			faceBuffer.update(faceData, 0, sizeof(VK::PlanetFaceData) * instance);
		}

		vkCmdEndRenderPass(cmd);

		VK::RenderPassBeginInfo guiBegin(guiPass, guiPass, scissor, NULL);
		vkCmdBeginRenderPass(cmd, &guiBegin, VK_SUBPASS_CONTENTS_INLINE);

		manager.begin(cmd);

		static char text[256] = { 0 };

		VK::GUIData gui;

		//gui.vGUIRect = VK::vec4(0, 0, m_nWidth, m_nHeight);
		//gui.vGUIColor = VK::vec4(0, 1, 0, 1);
		//gui.vGUIOptions = VK::vec4(0, 10, 0, 0);
		//manager.addGUIElements(cmd, "GUIBubble", &gui, 1);

		//gui.vGUIRect = VK::vec4(0, m_nHeight/2-25, m_nWidth, 50);
		//gui.vGUIColor = VK::vec4(1, 0, 0, 1);
		//gui.vGUIOptions = VK::vec4(10, 100, 0, 0);
		//manager.addGUIElements(cmd, "GUIBar", &gui, 1);

		static int buttonState = 1;
		//gui.vGUIRect = VK::vec4(0, m_nHeight - 50, 200, 50);
		//gui.vGUIColor = VK::vec4(0.5f, 0.5f, 0.5f, 1.0f);
		//gui.vGUIOptions = VK::vec4(buttonState, 0, 0, 0);
		//manager.addGUIElements(cmd, "GUIButton", &gui, 1);

		manager.addText(cmd, FONT_NAME, text, VK::vec2(100.0f-buttonState, m_nHeight+buttonState - 25.0f), VK::vec4(1,0,0,0), 20, VK::Font::AlignXCenter, VK::Font::AlignYCenter);

		//gui.vGUIRect = VK::vec4(0, 0, m_nWidth, 22);
		//gui.vGUIColor = VK::vec4(0, 0, 0, 0.8f);
		//gui.vGUIOptions = VK::vec4(0, 0, 0, 0);
		//manager.addGUIElements(cmd, "GUIBox", &gui, 1);

		//manager.addText(cmd, "arial1", "This is a test of the emergency broadcast system! If this had been a real emergency...", VK::vec2(5, 5), VK::vec4(0.8f, 0.8f, 0.8f, 1), 8.5f, VK::Font::AlignXLeft, VK::Font::AlignYBottom);

		manager.end();

		vkCmdEndRenderPass(cmd);
		vkEndCommandBuffer(cmd);

		VK::SubmitInfo submitInfo(&cmd);
		vkQueueSubmit(vk, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(vk);
		vkDeviceWaitIdle(vk);
		vkFreeCommandBuffers(vk, vk, 1, &cmd);

		color.setLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
		normal.setLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
		vk.present(color);

		++frameCount;
		double t = VK::Timer::Time();
		if (t - lastLogTime >= 1.0) {
			//buttonState *= -1;

			sprintf(text, "%u FPS", frameCount);
			VKLogNotice("Frames per second: %u (save time %lf)", frameCount, (t - lastLogTime));
			lastLogTime = t;

#if 0
			VK::Image staging;
			staging.createTexture(VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_LINEAR, VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, m_nWidth, m_nHeight);
			staging.setLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_PREINITIALIZED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
			VK::ImageCopy copy_region(m_nWidth, m_nHeight);
			vkCmdCopyImage(vk, color, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, staging, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_region);
			staging.setLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL);
			vk.flush(); // Wait for the copy command to complete before the staging texture goes out of scope!

			VkImageSubresource subres = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0 };
			VkSubresourceLayout sublayout;
			vkGetImageSubresourceLayout(vk, staging, &subres, &sublayout);
			uint8_t *data = NULL;
			if (VK_SUCCESS == vkMapMemory(vk, staging, 0, staging.getAllocationInfo().allocationSize, 0, (void **)&data)) {
				VK::PixelBuffer<uint8_t> pb;
				pb.create(m_nWidth, m_nHeight, 1, 3);
				uint8_t *bytes = new uint8_t[sublayout.rowPitch * m_nHeight];
				memcpy(bytes, data, sublayout.rowPitch * m_nHeight);
				vkUnmapMemory(vk, staging);

				data = bytes;
				for (uint16_t y = 0; y < pb.getHeight(); y++) {
					uint8_t *src = data;
					for (uint16_t x = 0; x < pb.getWidth(); x++) {
					uint8_t *dest = pb(x, y);
					*dest++ = src[0];
					*dest++ = src[1];
					*dest++ = src[2];
					src += 4;
					}
					data += sublayout.rowPitch;
				}

				delete bytes;
				pb.savePNG(VK::Path::Images() + "color.png");
			}
#endif
			t = VK::Timer::Time();
			lastLogTime = t;
			frameCount = 0;
		}
	}

	virtual void onKeyDown(uint16_t nKey) {
		VK::ShaderTechnique *p = NULL;
		switch (nKey) {
			case 'U':
				m_bUpdate = !m_bUpdate;
				break;
			case 'P':
				p = manager.getTechnique("PlanetFace");
				if (p && p->isValid()) {
					p->setFillMode(p->getFillMode() == VK_POLYGON_MODE_FILL ? VK_POLYGON_MODE_LINE : VK_POLYGON_MODE_FILL);
					p->buildPipeline(graphicsPass, pipelineLayout);
				}
				break;
		}
	}
	void updateCamera() {
		float fSeconds = m_fFrameTime;
		char buffer[256];
		camera.to_s(buffer);

		if (isKeyDown(VK_NUMPAD2) || isKeyDown(VK_DOWN))
			camera.rotate(VK::quat(camera.getRightAxis(), fSeconds*0.5f));
		if (isKeyDown(VK_NUMPAD8) || isKeyDown(VK_UP))
			camera.rotate(VK::quat(camera.getRightAxis(), -fSeconds*0.5f));
		if (isKeyDown(VK_NUMPAD4) || isKeyDown(VK_LEFT))
			camera.rotate(VK::quat(camera.getUpAxis(), fSeconds*0.5f));
		if (isKeyDown(VK_NUMPAD6) || isKeyDown(VK_RIGHT))
			camera.rotate(VK::quat(camera.getUpAxis(), -fSeconds*0.5f));
		if (/*isKeyDown(VK_NUMPAD7) || */ isKeyDown('Q'))
			camera.rotate(VK::quat(camera.getViewAxis(), -fSeconds*0.5f));
		if (/*isKeyDown(VK_NUMPAD9) || */ isKeyDown('E'))
			camera.rotate(VK::quat(camera.getViewAxis(), fSeconds*0.5f));
		camera.dir = camera.dir.normalize();

		VK::vec3 vAccel(0, 0, 0);
		if (isKeyDown(' '))
			velocity = VK::vec3(0, 0, 0);
		else {
			if (isKeyDown('W'))
				vAccel += camera.getViewAxis();
			if (isKeyDown('S'))
				vAccel -= camera.getViewAxis();
			if (isKeyDown('D'))
				vAccel += camera.getRightAxis();
			if (isKeyDown('A'))
				vAccel -= camera.getRightAxis();

			float m_fThrust = 1.0f;
			if (isKeyDown(VK_LCONTROL) || isKeyDown(VK_RCONTROL))
				vAccel *= m_fThrust * 100.0f;
			else
				vAccel *= m_fThrust;

			camera.translate(velocity * fSeconds + vAccel * (0.5f * fSeconds * fSeconds));
			velocity += vAccel * fSeconds;
		}
	}
};

Window window;
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow) {
	VK::Timer::Init();
	VK::Logger logger;

	VK::Noise noise;
	noise.init(3, 12345);
	std::vector<VK::vec3> vectors[6];
	for (int face = 0; face < 6; face++) {
		vectors[face].resize(TestWidth * TestWidth);
		window.pbHeight[face].create(TestWidth, TestWidth, 1, 4);
		window.pbHeight[face] = 0;
		int n = 0;
		float y = 0.0f; // y goes from top to bottom
		for (int i = 0; i < TestWidth; i++) {
			float yy = y*y;
			float x = 0.0f; // x goes from left to right
			for (int j = 0; j < TestWidth; j++) {
				VK::dvec3 dVec = CubeFace::GetPlanetaryVector(face, x, y);
				vectors[face][n++] = dVec.normalize();
				x += 1.0f / (TestWidth - 1);
			}
			y += 1.0f / (TestWidth - 1);
		}
	}

#define VORONOI_CELLS 10
	// Generate random centers of "plates" (which will be treated like Voronoi cells)
	VK::vec3 plates[VORONOI_CELLS], push[VORONOI_CELLS];
	for (int i = 0; i < VORONOI_CELLS; i++)
		plates[i] = VK::vec3(real_random(gen), real_random(gen), real_random(gen)).normalize();

	// Run a number of passes to push the centers apart to try to keep them fairly evenly spaced.
	for (int pass = 0; pass < 100; pass++) {
		for (int i = 0; i < VORONOI_CELLS; i++)
			push[i].x = push[i].y = push[i].z = 0;
		for (int i = 0; i < VORONOI_CELLS; i++) {
			for (int j = 0; j < VORONOI_CELLS; j++) {
				if (j == i)
					continue;
				float dist2 = plates[i].dist2(plates[j]);
				if (dist2 < 0.01f) // Set a min on dist2 to keep the push vector from going to inf
					dist2 = 0.01f;
				push[i] += (plates[i] - plates[j]) / dist2;
			}
		}
		for (int i = 0; i < VORONOI_CELLS; i++) {
			push[i] *= 0.01f / push[i].mag();
			plates[i] = (plates[i] + push[i]).normalize();
		}
	}

	// Find the "plate" each texel on the height map belongs to using Voronoi distance checks
	for (int face = 0; face < 6; face++) {
		VK::vec4 *pv = (VK::vec4 *)window.pbHeight[face][0];
		for (int n = 0; n < TestWidth*TestWidth; n++) {
			float dist = 1e+10f;
			pv[n].w = -1;

			// Add a little noise to each position to avoid perfectly straight plate edges
			VK::vec3 v = vectors[face][n] * 4.0;
			v += noise.noise(&v.x) * 0.25f;
			v = v.normalize();
			for (int i = 0; i < VORONOI_CELLS; i++) {
				float d = plates[i].dist2(v);
				if (d < dist) {
					dist = d;
					pv[n].w = (float)i;
				}
			}
		}
	}

	//srand(12345);
	int up = 0, down = 0;
	VK::Plane plane;
	for (int i = 0; i < 10; i++) {
		//VK::vec3 normal((rand() - RAND_MAX/2) / (float)RAND_MAX, (rand() - RAND_MAX / 2) / (float)RAND_MAX, (rand() - RAND_MAX / 2) / (float)RAND_MAX);
		//float f = (rand() - RAND_MAX / 2) / (float)RAND_MAX;
		VK::vec3 normal(real_random(gen), real_random(gen), real_random(gen));
		float f = real_random(gen);

		plane.init(normal.normalize(), 0);
		for (int face = 0; face < 6; face++) {
			VK::vec4 *pv = (VK::vec4 *)window.pbHeight[face][0];
			for (int n = 0; n < TestWidth*TestWidth; n++) {
				float d = plane.distance(vectors[face][n]);
				if (d > 0) {
					pv[n].x += 1;
					++up;
				} else {
					pv[n].x -= 1;
					++down;
				}
			}
		}
		if ((i % 1000) == 0) {
			VKLogInfo("up(%d), down(%d)", up, down);
			up = down = 0;
		}
	}

	float total = 0;
	for (int face = 0; face < 6; face++) {
		VK::vec4 *pv = (VK::vec4 *)window.pbHeight[face][0];
		for (int n = 0; n < TestWidth*TestWidth; n++)
			total += pv[n].x;
	}
	float avg = ((total / (TestWidth*TestWidth * 6)) * 0.9f);
	for (int face = 0; face < 6; face++) {
		VK::vec4 *pv = (VK::vec4 *)window.pbHeight[face][0];
		for (int n = 0; n < TestWidth*TestWidth; n++)
			pv[n].x -= avg;
	}

	typedef std::vector<VK::ivec3> CoastLine;
	typedef std::vector<CoastLine> LandMasses;
	LandMasses land;
	for (int face = 0; face < 6; face++) {
		for (int y = 0; y < TestWidth; y++) {
			for (int x = 0; x < TestWidth; x++) {
				VK::vec4 *pv = (VK::vec4 *)window.pbHeight[face](x, y);
				float h = pv->x; // The height of this point
				float l = pv->y; // The land mass index already assigned to this point
				if (h > 0 && l == 0) {
					land.push_back(CoastLine());
					pv->y = l = (float)land.size();
					CoastLine &coast = land.back();
					CoastLine stack;
					stack.push_back(VK::ivec3(x, y, face));
					while (!stack.empty()) {
						CoastLine next_stack;

						while (!stack.empty()) {
							bool land_locked = true;
							VK::ivec3 current = stack.back();
							stack.pop_back();
							VK::ivec3 v[8] = {
								VK::ivec3(current.x - 1, current.y, current.z), // W
								VK::ivec3(current.x + 1, current.y, current.z), // E
								VK::ivec3(current.x, current.y - 1, current.z), // S
								VK::ivec3(current.x, current.y + 1, current.z), // N
								VK::ivec3(current.x - 1, current.y - 1, current.z), // SW
								VK::ivec3(current.x - 1, current.y + 1, current.z), // NW
								VK::ivec3(current.x + 1, current.y - 1, current.z), // SE
								VK::ivec3(current.x + 1, current.y + 1, current.z), // NE
							};
							for (int i = 0; i < 8; i++) {
								uint8_t nFace = v[i].z;
								CubeFace::AdjustCoords(TestWidth - 1, nFace, v[i].x, v[i].y);
								v[i].z = nFace;
								VK::vec4 *pv = (VK::vec4 *)window.pbHeight[v[i].z](v[i].x, v[i].y);
								if (pv->x > 0) {
									if (pv->y == 0) {
										pv->y = l;
										next_stack.push_back(v[i]);
									}
								} else
									land_locked = false;
							}
							if (!land_locked)
								coast.push_back(current);
						}
						next_stack.swap(stack);
					}

				}
			}
		}
	}

	VKLogInfo("Final up(%d), down(%d), land masses(%d)", up, down, (int)land.size());
	//exit(0);

	try {
		//VK::Profiler profiler("VKTest", 3);
		VK::Context::Init();
		VK::Window::Init wInit(hInstance);
		window.create(VK_API_VERSION_1_0, "VKTest", 800, 600, false);
		window.Run();
		window.destroy();
	} catch (const char *error) {
		::MessageBox(NULL, error, "Aborting due to exception!", MB_OK);
	}
	return 0;
}
