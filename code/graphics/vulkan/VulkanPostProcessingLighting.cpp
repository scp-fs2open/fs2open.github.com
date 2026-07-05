#include "VulkanPostProcessing.h"

#include <array>

#include "gr_vulkan.h"
#include "VulkanRenderer.h"
#include "VulkanBuffer.h"
#include "VulkanTexture.h"
#include "VulkanPipeline.h"
#include "VulkanDescriptorManager.h"
#include "graphics/util/uniform_structs.h"
#include "graphics/util/primitives.h"
#include "graphics/grinternal.h"
#include "graphics/light.h"
#include "graphics/matrix.h"
#include "graphics/shadows.h"
#include "graphics/2d.h"
#include "bmpman/bmpman.h"
#include "lighting/lighting_profiles.h"
#include "lighting/lighting.h"
#include "math/vecmat.h"
#include "mod_table/mod_table.h"
#include "render/3d.h"
#include "tracing/tracing.h"
#include "nebula/neb.h"
#include "mission/missionparse.h"

extern float Sun_spot;
extern int Game_subspace_effect;
extern SCP_vector<light> Lights;
extern int Num_lights;


namespace graphics::vulkan {


// ===== Light Accumulation (Deferred Lighting) =====

void VulkanDeferredLighting::init(PostProcessContext& ctx, const RenderTarget& sceneColor,
                                  const VulkanDeferredGBuffer& gbuffer, const VulkanShadowMap& shadow)
{
	m_ctx = &ctx;
	m_sceneColor = &sceneColor;
	m_gbuffer = &gbuffer;
	m_shadow = &shadow;
}

bool VulkanDeferredLighting::initLightVolumes()
{
	if (m_lightVolumesInitialized) {
		return true;
	}

	// Generate sphere mesh (16 rings x 16 segments)
	{
		auto mesh = graphics::util::generate_sphere_mesh(16, 16);
		m_sphereMesh.vertexCount = mesh.vertex_count;
		m_sphereMesh.indexCount = mesh.index_count;

		// Create VBO
		vk::BufferCreateInfo vboInfo;
		vboInfo.size = mesh.vertices.size() * sizeof(float);
		vboInfo.usage = vk::BufferUsageFlagBits::eVertexBuffer;
		vboInfo.sharingMode = vk::SharingMode::eExclusive;

		try {
			m_sphereMesh.vbo = m_ctx->device.createBuffer(vboInfo);
		} catch (const vk::SystemError& e) {
			nprintf(("vulkan", "VulkanPostProcessor: Failed to create sphere VBO: %s\n", e.what()));
			return false;
		}

		if (!m_ctx->memoryManager->allocateBufferMemory(m_sphereMesh.vbo, MemoryUsage::CpuToGpu, m_sphereMesh.vboAlloc)) {
			m_ctx->device.destroyBuffer(m_sphereMesh.vbo);
			m_sphereMesh.vbo = nullptr;
			return false;
		}

		auto* mapped = m_ctx->memoryManager->mapMemory(m_sphereMesh.vboAlloc);
		if (mapped) {
			memcpy(mapped, mesh.vertices.data(), mesh.vertices.size() * sizeof(float));
			m_ctx->memoryManager->unmapMemory(m_sphereMesh.vboAlloc);
		}

		// Create IBO
		vk::BufferCreateInfo iboInfo;
		iboInfo.size = mesh.indices.size() * sizeof(ushort);
		iboInfo.usage = vk::BufferUsageFlagBits::eIndexBuffer;
		iboInfo.sharingMode = vk::SharingMode::eExclusive;

		try {
			m_sphereMesh.ibo = m_ctx->device.createBuffer(iboInfo);
		} catch (const vk::SystemError& e) {
			nprintf(("vulkan", "VulkanPostProcessor: Failed to create sphere IBO: %s\n", e.what()));
			return false;
		}

		if (!m_ctx->memoryManager->allocateBufferMemory(m_sphereMesh.ibo, MemoryUsage::CpuToGpu, m_sphereMesh.iboAlloc)) {
			m_ctx->device.destroyBuffer(m_sphereMesh.ibo);
			m_sphereMesh.ibo = nullptr;
			return false;
		}

		mapped = m_ctx->memoryManager->mapMemory(m_sphereMesh.iboAlloc);
		if (mapped) {
			memcpy(mapped, mesh.indices.data(), mesh.indices.size() * sizeof(ushort));
			m_ctx->memoryManager->unmapMemory(m_sphereMesh.iboAlloc);
		}
	}

	// Generate cylinder mesh (16 segments)
	{
		auto mesh = graphics::util::generate_cylinder_mesh(16);
		m_cylinderMesh.vertexCount = mesh.vertex_count;
		m_cylinderMesh.indexCount = mesh.index_count;

		vk::BufferCreateInfo vboInfo;
		vboInfo.size = mesh.vertices.size() * sizeof(float);
		vboInfo.usage = vk::BufferUsageFlagBits::eVertexBuffer;
		vboInfo.sharingMode = vk::SharingMode::eExclusive;

		try {
			m_cylinderMesh.vbo = m_ctx->device.createBuffer(vboInfo);
		} catch (const vk::SystemError& e) {
			nprintf(("vulkan", "VulkanPostProcessor: Failed to create cylinder VBO: %s\n", e.what()));
			return false;
		}

		if (!m_ctx->memoryManager->allocateBufferMemory(m_cylinderMesh.vbo, MemoryUsage::CpuToGpu, m_cylinderMesh.vboAlloc)) {
			m_ctx->device.destroyBuffer(m_cylinderMesh.vbo);
			m_cylinderMesh.vbo = nullptr;
			return false;
		}

		auto* mapped = m_ctx->memoryManager->mapMemory(m_cylinderMesh.vboAlloc);
		if (mapped) {
			memcpy(mapped, mesh.vertices.data(), mesh.vertices.size() * sizeof(float));
			m_ctx->memoryManager->unmapMemory(m_cylinderMesh.vboAlloc);
		}

		vk::BufferCreateInfo iboInfo;
		iboInfo.size = mesh.indices.size() * sizeof(ushort);
		iboInfo.usage = vk::BufferUsageFlagBits::eIndexBuffer;
		iboInfo.sharingMode = vk::SharingMode::eExclusive;

		try {
			m_cylinderMesh.ibo = m_ctx->device.createBuffer(iboInfo);
		} catch (const vk::SystemError& e) {
			nprintf(("vulkan", "VulkanPostProcessor: Failed to create cylinder IBO: %s\n", e.what()));
			return false;
		}

		if (!m_ctx->memoryManager->allocateBufferMemory(m_cylinderMesh.ibo, MemoryUsage::CpuToGpu, m_cylinderMesh.iboAlloc)) {
			m_ctx->device.destroyBuffer(m_cylinderMesh.ibo);
			m_cylinderMesh.ibo = nullptr;
			return false;
		}

		mapped = m_ctx->memoryManager->mapMemory(m_cylinderMesh.iboAlloc);
		if (mapped) {
			memcpy(mapped, mesh.indices.data(), mesh.indices.size() * sizeof(ushort));
			m_ctx->memoryManager->unmapMemory(m_cylinderMesh.iboAlloc);
		}
	}

	// Create deferred UBO for light data (per-frame, host-visible)
	{
		vk::BufferCreateInfo bufInfo;
		bufInfo.size = DEFERRED_UBO_SIZE;
		bufInfo.usage = vk::BufferUsageFlagBits::eUniformBuffer;
		bufInfo.sharingMode = vk::SharingMode::eExclusive;

		try {
			m_deferredUBO = m_ctx->device.createBuffer(bufInfo);
		} catch (const vk::SystemError& e) {
			nprintf(("vulkan", "VulkanPostProcessor: Failed to create deferred UBO: %s\n", e.what()));
			return false;
		}

		if (!m_ctx->memoryManager->allocateBufferMemory(m_deferredUBO, MemoryUsage::CpuToGpu, m_deferredUBOAlloc)) {
			m_ctx->device.destroyBuffer(m_deferredUBO);
			m_deferredUBO = nullptr;
			return false;
		}
	}

	m_lightVolumesInitialized = true;
	nprintf(("vulkan", "VulkanPostProcessor: Light volumes initialized (sphere: %u verts/%u idx, cylinder: %u verts/%u idx)\n",
		m_sphereMesh.vertexCount, m_sphereMesh.indexCount,
		m_cylinderMesh.vertexCount, m_cylinderMesh.indexCount));
	return true;
}

void VulkanDeferredLighting::shutdown()
{
	if (!m_ctx || !m_ctx->device) {
		return;
	}

	auto destroyMesh = [&](LightVolumeMesh& mesh) {
		if (mesh.vbo) { m_ctx->device.destroyBuffer(mesh.vbo); mesh.vbo = nullptr; }
		if (mesh.vboAlloc.isValid()) { m_ctx->memoryManager->freeAllocation(mesh.vboAlloc); }
		if (mesh.ibo) { m_ctx->device.destroyBuffer(mesh.ibo); mesh.ibo = nullptr; }
		if (mesh.iboAlloc.isValid()) { m_ctx->memoryManager->freeAllocation(mesh.iboAlloc); }
		mesh.vertexCount = 0;
		mesh.indexCount = 0;
	};

	destroyMesh(m_sphereMesh);
	destroyMesh(m_cylinderMesh);

	if (m_deferredUBO) {
		m_ctx->device.destroyBuffer(m_deferredUBO);
		m_deferredUBO = nullptr;
	}
	if (m_deferredUBOAlloc.isValid()) {
		m_ctx->memoryManager->freeAllocation(m_deferredUBOAlloc);
	}

	if (m_lightAccumFramebuffer) {
		m_ctx->device.destroyFramebuffer(m_lightAccumFramebuffer);
		m_lightAccumFramebuffer = nullptr;
	}
	if (m_lightAccumRenderPass) {
		m_ctx->device.destroyRenderPass(m_lightAccumRenderPass);
		m_lightAccumRenderPass = nullptr;
	}

	m_lightVolumesInitialized = false;
}

bool VulkanDeferredLighting::initLightAccumPass()
{
	// Light accumulation render pass: single RGBA16F color attachment
	// loadOp=eLoad (preserves emissive copy), storeOp=eStore
	// initialLayout=eColorAttachmentOptimal, finalLayout=eShaderReadOnlyOptimal
	{
		vk::AttachmentDescription att;
		att.format = HDR_COLOR_FORMAT;
		att.samples = vk::SampleCountFlagBits::e1;
		att.loadOp = vk::AttachmentLoadOp::eLoad;
		att.storeOp = vk::AttachmentStoreOp::eStore;
		att.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
		att.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
		att.initialLayout = vk::ImageLayout::eColorAttachmentOptimal;
		att.finalLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

		vk::AttachmentReference colorRef;
		colorRef.attachment = 0;
		colorRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

		vk::SubpassDescription subpass;
		subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorRef;

		vk::SubpassDependency dep;
		dep.srcSubpass = VK_SUBPASS_EXTERNAL;
		dep.dstSubpass = 0;
		dep.srcStageMask = vk::PipelineStageFlagBits::eTransfer
		                  | vk::PipelineStageFlagBits::eColorAttachmentOutput;
		dep.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput
		                  | vk::PipelineStageFlagBits::eFragmentShader;
		dep.srcAccessMask = vk::AccessFlagBits::eTransferWrite
		                  | vk::AccessFlagBits::eColorAttachmentWrite;
		dep.dstAccessMask = vk::AccessFlagBits::eColorAttachmentRead
		                  | vk::AccessFlagBits::eColorAttachmentWrite
		                  | vk::AccessFlagBits::eShaderRead;

		vk::RenderPassCreateInfo rpInfo;
		rpInfo.attachmentCount = 1;
		rpInfo.pAttachments = &att;
		rpInfo.subpassCount = 1;
		rpInfo.pSubpasses = &subpass;
		rpInfo.dependencyCount = 1;
		rpInfo.pDependencies = &dep;

		try {
			m_lightAccumRenderPass = m_ctx->device.createRenderPass(rpInfo);
		} catch (const vk::SystemError& e) {
			nprintf(("vulkan", "VulkanPostProcessor: Failed to create light accum render pass: %s\n", e.what()));
			return false;
		}
	}

	// Framebuffer using composite image as sole color attachment
	{
		std::array<vk::ImageView, 1> attachments = { m_gbuffer->compositeView() };

		vk::FramebufferCreateInfo fbInfo;
		fbInfo.renderPass = m_lightAccumRenderPass;
		fbInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		fbInfo.pAttachments = attachments.data();
		fbInfo.width = m_ctx->sceneExtent.width;
		fbInfo.height = m_ctx->sceneExtent.height;
		fbInfo.layers = 1;

		try {
			m_lightAccumFramebuffer = m_ctx->device.createFramebuffer(fbInfo);
		} catch (const vk::SystemError& e) {
			nprintf(("vulkan", "VulkanPostProcessor: Failed to create light accum framebuffer: %s\n", e.what()));
			return false;
		}
	}

	return true;
}

namespace ltp = lighting_profiles;

static graphics::deferred_light_data* prepare_light_uniforms(light& l, uint8_t* dest, const ltp::profile* lp)
{
	auto* light_data = reinterpret_cast<graphics::deferred_light_data*>(dest);
	memset(light_data, 0, sizeof(graphics::deferred_light_data));

	light_data->lightType = static_cast<int>(l.type);

	float intensity =
		(Lighting_mode == lighting_mode::COCKPIT) ? lp->cockpit_light_intensity_modifier.handle(l.intensity) : l.intensity;

	vec3d diffuse;
	diffuse.xyz.x = l.r * intensity;
	diffuse.xyz.y = l.g * intensity;
	diffuse.xyz.z = l.b * intensity;

	light_data->diffuseLightColor = diffuse;
	light_data->enable_shadows = 0;
	light_data->sourceRadius = l.source_radius;
	return light_data;
}

void VulkanDeferredLighting::render(vk::CommandBuffer cmd)
{
	TRACE_SCOPE(tracing::ApplyLights);

	if (!m_gbuffer->isInitialized()) {
		return;
	}

	// Lazy-init light volumes and accumulation pass on first use
	if (!m_lightVolumesInitialized) {
		if (!initLightVolumes() || !initLightAccumPass()) {
			return;
		}
	}

	auto* pipelineMgr = getPipelineManager();
	auto* descriptorMgr = getDescriptorManager();
	auto* bufferMgr = getBufferManager();
	auto* texMgr = getTextureManager();

	if (!pipelineMgr || !descriptorMgr || !bufferMgr || !texMgr) {
		return;
	}

	// Sort lights by type (same stable sort as OpenGL)
	std::stable_sort(Lights.begin(), Lights.end(), light_compare_by_type);

	// Categorize lights
	SCP_vector<light> full_frame_lights;
	SCP_vector<light> sphere_lights;
	SCP_vector<light> cylinder_lights;
	for (auto& l : Lights) {
		switch (l.type) {
		case Light_Type::Directional:
			full_frame_lights.push_back(l);
			break;
		case Light_Type::Cone:
		case Light_Type::Point:
			sphere_lights.push_back(l);
			break;
		case Light_Type::Tube:
			cylinder_lights.push_back(l);
			break;
		case Light_Type::Ambient:
			break;
		}
	}

	// Add ambient light
	{
		light& l = full_frame_lights.emplace_back();
		memset(&l, 0, sizeof(light));
		vec3d ambient;
		gr_get_ambient_light(&ambient);
		l.r = ambient.xyz.x;
		l.g = ambient.xyz.y;
		l.b = ambient.xyz.z;
		l.type = Light_Type::Ambient;
		l.intensity = 1.f;
		l.source_radius = 0.f;
	}

	size_t total_lights = full_frame_lights.size() + sphere_lights.size() + cylinder_lights.size();
	if (total_lights == 0) {
		return;
	}

	// Map UBO and pack data
	auto* uboMapped = static_cast<uint8_t*>(m_ctx->memoryManager->mapMemory(m_deferredUBOAlloc));
	if (!uboMapped) {
		return;
	}

	// Determine alignment requirement
	uint32_t uboAlign = getRendererInstance()->getMinUniformBufferOffsetAlignment();
	auto alignUp = [uboAlign](uint32_t v) -> uint32_t {
		return (v + uboAlign - 1) & ~(uboAlign - 1);
	};

	// Layout in UBO:
	// [0]: deferred_global_data (header)
	// [aligned offset 1..N]: deferred_light_data per light
	// [aligned offset N+1..2N]: matrix_uniforms per light
	uint32_t globalDataSize = alignUp(static_cast<uint32_t>(sizeof(graphics::deferred_global_data)));
	uint32_t lightDataSize = alignUp(static_cast<uint32_t>(sizeof(graphics::deferred_light_data)));
	uint32_t matrixDataSize = alignUp(static_cast<uint32_t>(sizeof(graphics::matrix_uniforms)));

	uint32_t lightDataOffset = globalDataSize;
	uint32_t matrixDataOffset = lightDataOffset + (static_cast<uint32_t>(total_lights) * lightDataSize);
	uint32_t totalUBOSize = matrixDataOffset + (static_cast<uint32_t>(total_lights) * matrixDataSize);

	if (totalUBOSize > DEFERRED_UBO_SIZE) {
		nprintf(("vulkan", "VulkanPostProcessor: Deferred UBO overflow (%u > %u), skipping lights\n", totalUBOSize, DEFERRED_UBO_SIZE));
		m_ctx->memoryManager->unmapMemory(m_deferredUBOAlloc);
		return;
	}

	// Pack global header
	auto lp = ltp::current();
	// Determine if environment maps are available
	bool envMapAvailable = (ENVMAP > 0);
	tcache_slot_vulkan* envMapSlot = nullptr;
	tcache_slot_vulkan* irrMapSlot = nullptr;
	if (envMapAvailable) {
		envMapSlot = texMgr->getTextureSlot(ENVMAP);
		if (!envMapSlot || !envMapSlot->imageView || !envMapSlot->isCubemap) {
			envMapAvailable = false;
		}
	}
	if (envMapAvailable && IRRMAP > 0) {
		irrMapSlot = texMgr->getTextureSlot(IRRMAP);
		if (!irrMapSlot || !irrMapSlot->imageView || !irrMapSlot->isCubemap) {
			irrMapSlot = nullptr;  // Fall back to fallback cube for irrmap
		}
	}

	{
		auto* header = reinterpret_cast<graphics::deferred_global_data*>(uboMapped);
		memset(header, 0, sizeof(graphics::deferred_global_data));
		header->invScreenWidth = 1.0f / gr_screen.max_w;
		header->invScreenHeight = 1.0f / gr_screen.max_h;
		header->nearPlane = gr_near_plane;

		if (m_shadow->isInitialized() && Shadow_quality != ShadowQuality::Disabled) {
			vm_inverse_matrix4(&header->inv_view_matrix, &Shadow_view_matrix_render);

			int offset = (Lighting_mode == lighting_mode::COCKPIT) ? 0 : Num_cockpit_shadow_cascades;
			int count  = (Lighting_mode == lighting_mode::COCKPIT) ? Num_cockpit_shadow_cascades : Num_shadow_cascades;
			shadow_cascade_params_bind(offset, count);
		}
	}

	// Pack per-light data
	size_t lightIdx = 0;
	int shadowedDirectionalCount = 0;
	// Cascade shadow maps are only ever built for a single light (Static_light.front()),
	// so only raytraced shadows can shadow more than one directional light here -- a second
	// light reading the first light's cascade data would produce incorrect shadows.
	int maxShadowedDirectionalLights = shadows_use_raytracing() ? Max_rt_shadow_lights : 1;

	for (auto& l : full_frame_lights) {
		auto* ld = prepare_light_uniforms(l, uboMapped + lightDataOffset + (lightIdx * lightDataSize), lp);

		if (l.type == Light_Type::Directional) {
			if (m_shadow->isInitialized() && Shadow_quality != ShadowQuality::Disabled
			    && shadowedDirectionalCount < maxShadowedDirectionalLights) {
				ld->enable_shadows = 1;
				++shadowedDirectionalCount;
			} else {
				ld->enable_shadows = 0;
			}

			vec4 light_dir;
			light_dir.xyzw.x = -l.vec.xyz.x;
			light_dir.xyzw.y = -l.vec.xyz.y;
			light_dir.xyzw.z = -l.vec.xyz.z;
			light_dir.xyzw.w = 0.0f;
			vec4 view_dir;
			vm_vec_transform(&view_dir, &light_dir, &gr_view_matrix);
			ld->lightDir.xyz.x = view_dir.xyzw.x;
			ld->lightDir.xyz.y = view_dir.xyzw.y;
			ld->lightDir.xyz.z = view_dir.xyzw.z;
		}

		// Matrix: env texture matrix for full-frame lights
		auto* md = reinterpret_cast<graphics::matrix_uniforms*>(uboMapped + matrixDataOffset + (lightIdx * matrixDataSize));
		memset(md, 0, sizeof(graphics::matrix_uniforms));
		md->modelViewMatrix = gr_env_texture_matrix;
		++lightIdx;
	}

	for (auto& l : sphere_lights) {
		auto* ld = prepare_light_uniforms(l, uboMapped + lightDataOffset + (lightIdx * lightDataSize), lp);

		if (l.type == Light_Type::Cone) {
			ld->dualCone = (l.flags & LF_DUAL_CONE) ? 1.0f : 0.0f;
			ld->coneAngle = l.cone_angle;
			ld->coneInnerAngle = l.cone_inner_angle;
			ld->coneDir = l.vec2;
		}
		float rad = (Lighting_mode == lighting_mode::COCKPIT)
						? lp->cockpit_light_radius_modifier.handle(MAX(l.rada, l.radb))
						: MAX(l.rada, l.radb);
		ld->lightRadius = rad;
		ld->scale.xyz.x = rad * 1.05f;
		ld->scale.xyz.y = rad * 1.05f;
		ld->scale.xyz.z = rad * 1.05f;

		// Matrix: model-view + projection for light volume
		auto* md = reinterpret_cast<graphics::matrix_uniforms*>(uboMapped + matrixDataOffset + (lightIdx * matrixDataSize));
		g3_start_instance_matrix(&l.vec, &vmd_identity_matrix, true);
		md->modelViewMatrix = gr_model_view_matrix;
		md->projMatrix = gr_projection_matrix;
		g3_done_instance(true);
		++lightIdx;
	}

	for (auto& l : cylinder_lights) {
		auto* ld = prepare_light_uniforms(l, uboMapped + lightDataOffset + (lightIdx * lightDataSize), lp);
		float rad =
			(Lighting_mode == lighting_mode::COCKPIT) ? lp->cockpit_light_radius_modifier.handle(l.radb) : l.radb;
		ld->lightRadius = rad;
		ld->lightType = LT_TUBE;

		vec3d a;
		vm_vec_sub(&a, &l.vec, &l.vec2);
		auto length = vm_vec_mag(&a);
		length += ld->lightRadius * 2.0f;

		ld->scale.xyz.x = rad * 1.05f;
		ld->scale.xyz.y = rad * 1.05f;
		ld->scale.xyz.z = length;

		// Matrix: oriented instance matrix for cylinder
		auto* md = reinterpret_cast<graphics::matrix_uniforms*>(uboMapped + matrixDataOffset + (lightIdx * matrixDataSize));
		vec3d dir, newPos;
		matrix orient;
		vm_vec_normalized_dir(&dir, &l.vec, &l.vec2);
		vm_vector_2_matrix_norm(&orient, &dir, nullptr, nullptr);
		vm_vec_scale_sub(&newPos, &l.vec2, &dir, l.radb);

		g3_start_instance_matrix(&newPos, &orient, true);
		md->modelViewMatrix = gr_model_view_matrix;
		md->projMatrix = gr_projection_matrix;
		g3_done_instance(true);
		++lightIdx;
	}

	m_ctx->memoryManager->unmapMemory(m_deferredUBOAlloc);

	// Both fullscreen and volume lights use the same vertex layout (POSITION3).
	// For fullscreen lights the shader ignores vertex data and generates positions
	// from gl_VertexIndex, but Vulkan requires all declared vertex inputs to have
	// matching pipeline attributes and bound buffers.
	vertex_layout volLayout;
	volLayout.add_vertex_component(vertex_format_data::POSITION3, sizeof(float) * 3, 0);

	bool rtShadowsActive = shadows_use_raytracing() && m_shadow->isInitialized() && Shadow_quality != ShadowQuality::Disabled;

	PipelineConfig lightConfig;
	lightConfig.shaderType = SDR_TYPE_DEFERRED_LIGHTING;
	lightConfig.shaderFlags = rtShadowsActive ? SDR_FLAG_DEFERRED_RT_SHADOWS : 0;
	lightConfig.vertexLayoutHash = volLayout.hash();
	lightConfig.primitiveType = PRIM_TYPE_TRIS;
	lightConfig.depthMode = ZBUFFER_TYPE_NONE;
	lightConfig.blendMode = ALPHA_BLEND_ADDITIVE;
	lightConfig.cullEnabled = false;
	lightConfig.depthWriteEnabled = false;
	lightConfig.renderPass = m_lightAccumRenderPass;

	vk::Pipeline lightPipeline = pipelineMgr->getPipeline(lightConfig, volLayout);
	if (!lightPipeline) {
		return;
	}

	vk::PipelineLayout pipelineLayout = pipelineMgr->getPipelineLayout();

	// Begin light accumulation render pass
	{
		vk::RenderPassBeginInfo rpBegin;
		rpBegin.renderPass = m_lightAccumRenderPass;
		rpBegin.framebuffer = m_lightAccumFramebuffer;
		rpBegin.renderArea.offset = vk::Offset2D(0, 0);
		rpBegin.renderArea.extent = m_ctx->sceneExtent;

		cmd.beginRenderPass(rpBegin, vk::SubpassContents::eInline);
	}

	// Set viewport and scissor
	vk::Viewport viewport;
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(m_ctx->sceneExtent.width);
	viewport.height = static_cast<float>(m_ctx->sceneExtent.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	cmd.setViewport(0, viewport);

	vk::Rect2D scissor;
	scissor.offset = vk::Offset2D(0, 0);
	scissor.extent = m_ctx->sceneExtent;
	cmd.setScissor(0, scissor);

	// Pre-build G-buffer texture array (shared across all light draws)
	const auto& fallbacks = descriptorMgr->getFallbacks();
	std::array<vk::DescriptorImageInfo, VulkanDescriptorManager::MAX_TEXTURE_BINDINGS> gbufTexArray;
	gbufTexArray.fill(fallbacks.texture2D);
	gbufTexArray[0] = {m_ctx->linearSampler, m_sceneColor->view, vk::ImageLayout::eShaderReadOnlyOptimal};
	gbufTexArray[1] = {m_ctx->linearSampler, m_gbuffer->normalView(), vk::ImageLayout::eShaderReadOnlyOptimal};
	gbufTexArray[2] = {m_ctx->linearSampler, m_gbuffer->positionView(), vk::ImageLayout::eShaderReadOnlyOptimal};
	gbufTexArray[3] = {m_ctx->linearSampler, m_gbuffer->specularView(), vk::ImageLayout::eShaderReadOnlyOptimal};

	// Pre-build shadow/env/irr image infos (shared across all light draws).
	// Shadow map is depth-only, sampled with a depth-compare sampler
	// (sampler2DArrayShadow) for hardware PCF.
	vk::DescriptorImageInfo shadowTexInfo;
	if (m_shadow->isInitialized() && m_shadow->depthView()) {
		shadowTexInfo = {m_shadow->compareSampler(), m_shadow->depthView(), vk::ImageLayout::eShaderReadOnlyOptimal};
	}

	// Shadow cascade params (projection matrices/distances) are bound per-frame via
	// shadow_cascade_params_bind() -> gr_bind_uniform_buffer(), landing in the draw
	// manager's pending uniform binding table; pick it up the same way applyMaterial()
	// does for other pending UBOs.
	vk::DescriptorBufferInfo shadowCascadeParamsInfo;
	{
		auto* drawManager = getDrawManager();
		const auto& pending = drawManager->getPendingUniformBinding(static_cast<size_t>(uniform_block_type::ShadowCascadeParams));
		if (pending.valid) {
			vk::Buffer buf = bufferMgr->getVkBuffer(pending.bufferHandle);
			if (buf) {
				shadowCascadeParamsInfo = vk::DescriptorBufferInfo(buf, pending.offset, pending.size);
			}
		}
	}
	vk::DescriptorImageInfo envTexInfo;
	if (envMapAvailable && envMapSlot) {
		envTexInfo = fallbacks.textureCube;
		envTexInfo.imageView = envMapSlot->imageView;
	}
	vk::DescriptorImageInfo irrTexInfo;
	if (envMapAvailable && irrMapSlot) {
		irrTexInfo = fallbacks.textureCube;
		irrTexInfo.imageView = irrMapSlot->imageView;
	}

	// Helper lambda to allocate + write descriptor sets for a single light draw
	auto bindLightDescriptors = [&](size_t li) {
		DescriptorWriter writer;
		writer.reset(m_ctx->device, fallbacks);

		// Set 0: Global
		vk::DescriptorSet globalSet = descriptorMgr->allocateFrameSet(DescriptorSetIndex::Global);
		if (!globalSet) return false;
		writer.writeSet(globalSet, VulkanDescriptorManager::getSetTemplate(DescriptorSetIndex::Global));
		writer.setBuffer(GlobalBinding::Lights, {m_deferredUBO,
			lightDataOffset + (li * lightDataSize), sizeof(graphics::deferred_light_data)});
		writer.setBuffer(GlobalBinding::DeferredData, {m_deferredUBO,
			0, sizeof(graphics::deferred_global_data)});
		writer.setImage(GlobalBinding::ShadowMap, shadowTexInfo);
		writer.setImage(GlobalBinding::EnvMap, envTexInfo);
		writer.setImage(GlobalBinding::IrradianceMap, irrTexInfo);
		writer.setBuffer(GlobalBinding::ShadowCascadeParams, shadowCascadeParamsInfo);

		// Set 1: Material
		vk::DescriptorSet materialSet = descriptorMgr->allocateFrameSet(DescriptorSetIndex::Material);
		if (!materialSet) return false;
		writer.writeSet(materialSet, VulkanDescriptorManager::getSetTemplate(DescriptorSetIndex::Material));
		writer.setImageArray(MaterialBinding::TextureArray, gbufTexArray);

		// Set 2: PerDraw
		vk::DescriptorSet perDrawSet = descriptorMgr->allocateFrameSet(DescriptorSetIndex::PerDraw);
		if (!perDrawSet) return false;
		writer.writeSet(perDrawSet, VulkanDescriptorManager::getSetTemplate(DescriptorSetIndex::PerDraw));
		writer.setBuffer(PerDrawBinding::Matrices, {m_deferredUBO,
			matrixDataOffset + (li * matrixDataSize), sizeof(graphics::matrix_uniforms)});
		writer.flush();

		std::array<vk::DescriptorSet, 3> sets = { globalSet, materialSet, perDrawSet };
		cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, 0, sets, {});

		return true;
	};

	cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, lightPipeline);

	// Draw full-frame lights (directional + ambient)
	// Bind sphere VBO as dummy — shader ignores vertex data for these light types.
	lightIdx = 0;
	if (!full_frame_lights.empty()) {
		cmd.bindVertexBuffers(0, m_sphereMesh.vbo, vk::DeviceSize(0));
		for (size_t i = 0; i < full_frame_lights.size(); ++i) {
			if (bindLightDescriptors(lightIdx)) {
				cmd.draw(3, 1, 0, 0);
			}
			++lightIdx;
		}
	}

	// Draw sphere lights (point + cone)
	if (!sphere_lights.empty()) {
		cmd.bindVertexBuffers(0, m_sphereMesh.vbo, vk::DeviceSize(0));
		cmd.bindIndexBuffer(m_sphereMesh.ibo, 0, vk::IndexType::eUint16);
		for (size_t i = 0; i < sphere_lights.size(); ++i) {
			if (bindLightDescriptors(lightIdx)) {
				cmd.drawIndexed(m_sphereMesh.indexCount, 1, 0, 0, 0);
			}
			++lightIdx;
		}
	}

	// Draw cylinder lights (tube)
	if (!cylinder_lights.empty()) {
		cmd.bindVertexBuffers(0, m_cylinderMesh.vbo, vk::DeviceSize(0));
		cmd.bindIndexBuffer(m_cylinderMesh.ibo, 0, vk::IndexType::eUint16);
		for (size_t i = 0; i < cylinder_lights.size(); ++i) {
			if (bindLightDescriptors(lightIdx)) {
				cmd.drawIndexed(m_cylinderMesh.indexCount, 1, 0, 0, 0);
			}
			++lightIdx;
		}
	}

	// End render pass (composite → eShaderReadOnlyOptimal)
	cmd.endRenderPass();
}

} // namespace graphics::vulkan
