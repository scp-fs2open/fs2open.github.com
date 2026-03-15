#include "VulkanShader.h"
#include "VulkanShaderCompiler.h"

namespace graphics::vulkan {

// Global shader manager pointer
static VulkanShaderManager* g_shaderManager = nullptr;

VulkanShaderManager* getShaderManager()
{
	Assertion(g_shaderManager != nullptr, "Vulkan ShaderManager not initialized!");
	return g_shaderManager;
}

void setShaderManager(VulkanShaderManager* manager)
{
	g_shaderManager = manager;
}

// ========== gr_screen function pointer implementations ==========

int vulkan_maybe_create_shader(shader_type shader_t, unsigned int flags)
{
	auto* shaderManager = getShaderManager();
	return shaderManager->maybeCreateShader(shader_t, flags);
}

void vulkan_recompile_all_shaders(const std::function<void(size_t, size_t)>& progressCallback)
{
	auto* shaderManager = getShaderManager();
	shaderManager->recompileAllShaders(progressCallback);
}

VulkanShaderManager::VulkanShaderManager() = default;
VulkanShaderManager::~VulkanShaderManager() = default;

bool VulkanShaderManager::init(vk::Device device)
{
	if (m_initialized) {
		return true;
	}

	m_device = device;

	// Initialize runtime shader compiler
	m_compiler = std::make_unique<VulkanShaderCompiler>();
	if (!m_compiler->init()) {
		mprintf(("VulkanShaderManager: Failed to initialize shader compiler!\n"));
		return false;
	}

	VulkanShaderCompiler::purgeOldCache();

	m_initialized = true;

	mprintf(("VulkanShaderManager: Initialized with runtime shader compilation\n"));
	return true;
}

void VulkanShaderManager::shutdown()
{
	if (!m_initialized) {
		return;
	}

	// Clear all shaders (unique_ptrs will clean up)
	m_shaders.clear();
	m_shaderMap.clear();
	m_freeSlots.clear();

	if (m_compiler) {
		m_compiler->shutdown();
		m_compiler.reset();
	}

	m_initialized = false;
	mprintf(("VulkanShaderManager: Shutdown complete\n"));
}

int VulkanShaderManager::maybeCreateShader(shader_type type, unsigned int flags)
{
	if (!m_initialized) {
		return -1;
	}

	shader_key_t key(static_cast<int>(type), flags);
	auto it = m_shaderMap.find(key);
	if (it != m_shaderMap.end()) {
		return static_cast<int>(it->second);
	}

	return loadShader(type, flags);
}

void VulkanShaderManager::recompileAllShaders(const std::function<void(size_t, size_t)>& progressCallback)
{
	if (!m_initialized || !m_compiler) {
		return;
	}

	size_t total = m_shaders.size();
	size_t current = 0;

	for (auto& shader : m_shaders) {
		if (shader.valid) {
			shader_type type = shader.type;
			unsigned int flags = shader.flags;

			// Release old modules
			shader.vertexModule.reset();
			shader.fragmentModule.reset();
			shader.valid = false;

			const ShaderTypeInfo* typeInfo = shader_get_type_info(type);
			if (typeInfo) {
				SCP_string vertFile = typeInfo->vert;
				auto vertSpirv = m_compiler->compile(vertFile, vk::ShaderStageFlagBits::eVertex, type, flags);
				if (!vertSpirv.empty()) {
					vk::ShaderModuleCreateInfo createInfo;
					createInfo.codeSize = vertSpirv.size() * sizeof(uint32_t);
					createInfo.pCode = vertSpirv.data();
					try {
						shader.vertexModule = m_device.createShaderModuleUnique(createInfo);
					} catch (const vk::SystemError& e) {
						mprintf(("VulkanShaderManager: Failed to create vertex module: %s\n", e.what()));
					}
				}
				shader.vertexInputMask = 0;
				for (auto attr : typeInfo->attributes) {
					shader.vertexInputMask |= (1u << attr);
				}

				SCP_string fragFile = typeInfo->frag;
				auto fragSpirv = m_compiler->compile(fragFile, vk::ShaderStageFlagBits::eFragment, type, flags);
				if (!fragSpirv.empty()) {
					vk::ShaderModuleCreateInfo createInfo;
					createInfo.codeSize = fragSpirv.size() * sizeof(uint32_t);
					createInfo.pCode = fragSpirv.data();
					try {
						shader.fragmentModule = m_device.createShaderModuleUnique(createInfo);
					} catch (const vk::SystemError& e) {
						mprintf(("VulkanShaderManager: Failed to create fragment module: %s\n", e.what()));
					}
				}

				shader.valid = shader.vertexModule && shader.fragmentModule;
			}
		}

		++current;
		if (progressCallback) {
			progressCallback(current, total);
		}
	}

	mprintf(("VulkanShaderManager: Recompiled %zu shaders\n", total));
}

const VulkanShaderModule* VulkanShaderManager::getShader(int handle) const
{
	if (handle < 0 || static_cast<size_t>(handle) >= m_shaders.size()) {
		return nullptr;
	}

	const VulkanShaderModule& shader = m_shaders[handle];
	return shader.valid ? &shader : nullptr;
}

const VulkanShaderModule* VulkanShaderManager::getShaderByType(shader_type type) const
{
	// Find the first shader of this type (any flags).
	// Used for vertex input mask queries where the specific variant doesn't matter.
	for (const auto& pair : m_shaderMap) {
		if (pair.first.first == static_cast<int>(type)) {
			return getShader(static_cast<int>(pair.second));
		}
	}
	return nullptr;
}

int VulkanShaderManager::loadShader(shader_type type, unsigned int flags)
{
	const ShaderTypeInfo* typeInfo = shader_get_type_info(type);
	if (!typeInfo) {
		mprintf(("VulkanShaderManager: Unknown shader type: %d\n", static_cast<int>(type)));
		return -1;
	}

	VulkanShaderModule shader;
	shader.type = type;
	shader.flags = flags;
	shader.description = typeInfo->description;
	if (flags != 0) {
		shader.description += " (flags=0x";
		char buf[16];
		snprintf(buf, sizeof(buf), "%x", flags);
		shader.description += buf;
		shader.description += ")";
	}

	// Compile vertex shader
	SCP_string vertFile = typeInfo->vert;
	auto vertSpirv = m_compiler->compile(vertFile, vk::ShaderStageFlagBits::eVertex, type, flags);
	if (!vertSpirv.empty()) {
		vk::ShaderModuleCreateInfo createInfo;
		createInfo.codeSize = vertSpirv.size() * sizeof(uint32_t);
		createInfo.pCode = vertSpirv.data();
		try {
			shader.vertexModule = m_device.createShaderModuleUnique(createInfo);
		} catch (const vk::SystemError& e) {
			mprintf(("VulkanShaderManager: Failed to create vertex module for %s: %s\n",
				vertFile.c_str(), e.what()));
		}
	}
	shader.vertexInputMask = 0;
	for (auto attr : typeInfo->attributes) {
		shader.vertexInputMask |= (1u << attr);
	}

	// Compile fragment shader
	SCP_string fragFile = typeInfo->frag;
	auto fragSpirv = m_compiler->compile(fragFile, vk::ShaderStageFlagBits::eFragment, type, flags);
	if (!fragSpirv.empty()) {
		vk::ShaderModuleCreateInfo createInfo;
		createInfo.codeSize = fragSpirv.size() * sizeof(uint32_t);
		createInfo.pCode = fragSpirv.data();
		try {
			shader.fragmentModule = m_device.createShaderModuleUnique(createInfo);
		} catch (const vk::SystemError& e) {
			mprintf(("VulkanShaderManager: Failed to create fragment module for %s: %s\n",
				fragFile.c_str(), e.what()));
		}
	}

	// Check if essential modules loaded
	shader.valid = shader.vertexModule && shader.fragmentModule;

	if (!shader.valid) {
		mprintf(("VulkanShaderManager: Failed to load shader type %d (flags=0x%x)\n",
			static_cast<int>(type), flags));
	}

	// Find or allocate slot
	size_t index;
	if (!m_freeSlots.empty()) {
		index = m_freeSlots.back();
		m_freeSlots.pop_back();
		m_shaders[index] = std::move(shader);
	} else {
		index = m_shaders.size();
		m_shaders.push_back(std::move(shader));
	}

	// Add to lookup map
	shader_key_t key(static_cast<int>(type), flags);
	m_shaderMap[key] = index;

	if (m_shaders[index].valid) {
		nprintf(("Vulkan", "VulkanShaderManager: Created shader %zu: %s\n",
			index, m_shaders[index].description.c_str()));
	}

	return static_cast<int>(index);
}

} // namespace graphics::vulkan
