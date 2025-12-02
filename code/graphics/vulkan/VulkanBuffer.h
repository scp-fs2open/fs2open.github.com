#pragma once

#include "graphics/2d.h"

#include <array>
#include <vulkan/vulkan.hpp>

namespace graphics {
namespace vulkan {

// Forward declarations
class VulkanRenderer;
class VulkanDescriptorManager;

/**
 * @brief Buffer data tracking synchronization, memory, and mapping state
 */
struct VulkanBufferData {
	vk::Buffer buffer;
	vk::DeviceMemory memory;
	BufferType type = BufferType::Vertex;
	BufferUsageHint usage = BufferUsageHint::Static;
	size_t size = 0;
	void* mappedPtr = nullptr;           // For persistent mapping
	uint32_t lastUsedFrame = 0;          // Frame sync tracking
	bool hostVisible = false;            // Memory property
	vk::DeviceAddress deviceAddress = 0; // GPU address for BDA (Vulkan 1.2+)
};

/**
 * @brief Pending buffer deletion entry for deferred destruction
 */
struct PendingBufferDeletion {
	vk::Buffer buffer;
	vk::DeviceMemory memory;
};

/**
 * @brief Manages Vulkan buffer allocation, updates, and lifecycle
 *
 * Design notes:
 * - Uses slot allocator (free list + dense array) for fast handle management
 * - Tracks frame usage for synchronization (can't update in-flight buffers)
 * - Supports persistent mapping for streaming/dynamic buffers
 * - Staging buffer pool for device-local buffer updates
 */
class VulkanBufferManager {
public:
	explicit VulkanBufferManager(vk::Device device, vk::PhysicalDevice physicalDevice, vk::Queue graphicsQueue, uint32_t graphicsQueueFamily);
	~VulkanBufferManager();

	// Disable copy
	VulkanBufferManager(const VulkanBufferManager&) = delete;
	VulkanBufferManager& operator=(const VulkanBufferManager&) = delete;

	/**
	 * @brief Initialize the buffer manager
	 * @param device Logical Vulkan device
	 * @param physicalDevice Physical device for memory properties
	 * @param graphicsQueue Graphics queue for transfer commands (ensures ordering with graphics work)
	 * @param graphicsQueueFamily Graphics queue family index for command pool creation
	 *
	 * @note We use the graphics queue rather than a dedicated transfer queue to ensure
	 * proper synchronization between buffer transfers and graphics work without semaphores.
	 * This sacrifices potential async transfer parallelism for simplicity and correctness.
	 */
	void initialize(vk::Device device, vk::PhysicalDevice physicalDevice, vk::Queue graphicsQueue, uint32_t graphicsQueueFamily);

	/**
	 * @brief Shutdown and release all buffers
	 */
	void shutdown();

	/**
	 * @brief Create a new buffer
	 * @param type Buffer type (Vertex, Index, Uniform)
	 * @param usage Usage hint for memory allocation strategy
	 * @return Handle to the created buffer
	 */
	gr_buffer_handle createBuffer(BufferType type, BufferUsageHint usage);

	/**
	 * @brief Delete a buffer
	 * @param handle Buffer handle to delete
	 */
	void deleteBuffer(gr_buffer_handle handle);

	/**
	 * @brief Update buffer data (full replacement)
	 * @param handle Buffer handle
	 * @param size Data size in bytes
	 * @param data Pointer to data (can be nullptr for allocation only)
	 */
	void updateBufferData(gr_buffer_handle handle, size_t size, const void* data);

	/**
	 * @brief Update buffer data at offset (partial update)
	 * @param handle Buffer handle
	 * @param offset Byte offset into buffer
	 * @param size Data size in bytes
	 * @param data Pointer to data
	 */
	void updateBufferDataOffset(gr_buffer_handle handle, size_t offset, size_t size, const void* data);

	/**
	 * @brief Map buffer for persistent write access
	 * @param handle Buffer handle (must be PersistentMapping usage)
	 * @return Mapped pointer, or nullptr on failure
	 */
	void* mapBuffer(gr_buffer_handle handle);

	/**
	 * @brief Flush a range of a mapped buffer to make writes visible to GPU
	 * @param handle Buffer handle
	 * @param offset Byte offset of range to flush
	 * @param size Size of range to flush
	 */
	void flushMappedBuffer(gr_buffer_handle handle, size_t offset, size_t size);

	/**
	 * @brief Bind a uniform buffer to a binding point
	 * @param bindPoint Uniform block binding point
	 * @param offset Byte offset (must respect minUniformBufferOffsetAlignment)
	 * @param size Size of the range
	 * @param handle Buffer handle
	 */
	void bindUniformBuffer(uniform_block_type bindPoint, size_t offset, size_t size, gr_buffer_handle handle);

	/**
	 * @brief Information about a bound uniform buffer
	 */
	struct BoundUniformBuffer {
		gr_buffer_handle handle;
		size_t offset = 0;
		size_t size = 0;
		vk::Buffer vkBuffer = nullptr;
		vk::DeviceAddress deviceAddress = 0;  // GPU address for BDA

		bool isValid() const {
			return handle.isValid() && vkBuffer != nullptr;
		}

		/**
		 * @brief Get effective GPU address (base + offset)
		 */
		vk::DeviceAddress getEffectiveAddress() const {
			return deviceAddress + offset;
		}
	};

	/**
	 * @brief Get current uniform buffer binding for a binding point
	 * @param bindPoint Uniform block binding point
	 * @return Bound uniform buffer info, or invalid if not bound
	 */
	BoundUniformBuffer getBoundUniformBuffer(uniform_block_type bindPoint) const;

	/**
	 * @brief Get descriptor set for uniform buffers
	 * @return Current uniform buffer descriptor set (may be null)
	 */
	vk::DescriptorSet getUniformDescriptorSet() const { 
		return m_uniformDescriptorSet; 
	}

	/**
	 * @brief Set descriptor manager reference
	 */
	void setDescriptorManager(VulkanDescriptorManager* manager) { 
		m_descriptorManager = manager; 
	}

	/**
	 * @brief Initialize uniform buffer descriptor set
	 * @param layout Descriptor set layout for uniform buffers
	 * @return true on success
	 */
	bool initializeUniformDescriptorSet(vk::DescriptorSetLayout layout);

	/**
	 * @brief Mark uniform descriptor set as bound (permanently)
	 * After this is called, no descriptor updates will EVER occur again.
	 * All uniform data changes must use dynamic offsets only.
	 * This prevents UPDATE_AFTER_BIND violations.
	 */
	void markUniformSetBound() { m_uniformSetEverBound = true; }

	/**
	 * @brief Get the Vulkan buffer handle for a gr_buffer_handle
	 * @param handle Buffer handle
	 * @return Vulkan buffer, or null handle if invalid
	 */
	vk::Buffer getBuffer(gr_buffer_handle handle) const;

	/**
	 * @brief Get buffer data (for internal use)
	 * @param handle Buffer handle
	 * @return Pointer to buffer data, or nullptr if invalid
	 */
	const VulkanBufferData* getBufferData(gr_buffer_handle handle) const;

	/**
	 * @brief Get buffer device address (BDA) for a buffer
	 * @param handle Buffer handle
	 * @return GPU device address, or 0 if invalid or BDA not supported
	 */
	vk::DeviceAddress getBufferDeviceAddress(gr_buffer_handle handle) const;

	/**
	 * @brief Push constant structure for uniform buffer addresses (BDA)
	 *
	 * Contains GPU addresses for all uniform block types.
	 * Used with vkCmdPushConstants to pass addresses to shaders.
	 */
	struct UniformAddressPushConstants {
		vk::DeviceAddress addresses[static_cast<size_t>(uniform_block_type::NUM_BLOCK_TYPES)];

		static constexpr uint32_t size() {
			return static_cast<uint32_t>(sizeof(addresses));
		}
	};

	/**
	 * @brief Get all bound uniform buffer addresses for push constants
	 * @return Push constant structure with effective addresses (base + offset)
	 */
	UniformAddressPushConstants getUniformAddresses() const;

	/**
	 * @brief Called at start of frame AFTER the frame's fence has been waited on
	 *
	 * This is the safe point to process deferred deletions for resources that were
	 * queued when this frame index was last active. Since we've waited on the fence,
	 * we know the GPU is done with all work from that frame.
	 *
	 * @param frameIndex Current frame index (0 to FRAMES_IN_FLIGHT-1)
	 */
	void beginFrame(uint32_t frameIndex);

	/**
	 * @brief Submit all pending transfer commands for the current frame
	 *
	 * Called by VulkanRenderer before graphics work submission. If no transfers
	 * were recorded this frame, this is a no-op.
	 *
	 * @note Transfer commands use the same queue and synchronize with the frame fence,
	 * so staging buffers are safe to delete when beginFrame() processes deferred deletions.
	 */
	void submitTransfers();

	/**
	 * @brief Get minimum UBO offset alignment
	 */
	size_t getMinUniformBufferOffsetAlignment() const { return m_minUboAlignment; }

	/**
	 * @brief Number of frames in flight
	 * Must match VulkanRenderer::MAX_FRAMES_IN_FLIGHT
	 */
	static constexpr uint32_t FRAMES_IN_FLIGHT = 2;

private:
	/**
	 * @brief Queue a buffer for deferred deletion on current frame
	 *
	 * The buffer will be destroyed when beginFrame() is called for this frame index
	 * again, which happens after the GPU fence for this frame has been waited on.
	 *
	 * @param buffer Vulkan buffer handle
	 * @param memory Device memory handle
	 */
	void queueDeferredDeletion(vk::Buffer buffer, vk::DeviceMemory memory);

	/**
	 * @brief Queue a staging buffer for deferred deletion on current frame
	 */
	void queueStagingDeletion(vk::Buffer buffer, vk::DeviceMemory memory);

	/**
	 * @brief Queue a descriptor set for deferred deletion on current frame
	 */
	void queueDescriptorSetDeletion(vk::DescriptorSet set);

	/**
	 * @brief Recreate uniform descriptor set when underlying buffer changes
	 *
	 * Called when a uniform buffer is reallocated (e.g., ring buffer grows) after
	 * the descriptor set has already been bound. Creates a new descriptor set,
	 * re-initializes all bindings, and queues the old set for deferred deletion.
	 *
	 * @return true if recreation was successful
	 */
	bool recreateUniformDescriptorSet();

	/**
	 * @brief Find suitable memory type for buffer allocation
	 * @param typeFilter Bitmask of acceptable memory types
	 * @param properties Required memory properties
	 * @return Memory type index
	 */
	uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties) const;

	/**
	 * @brief Create buffer with given usage and properties
	 */
	void createVkBuffer(VulkanBufferData& bufferData, size_t size, vk::BufferUsageFlags usage,
	                    vk::MemoryPropertyFlags properties);

	/**
	 * @brief Copy data via staging buffer (for device-local buffers)
	 */
	void copyViaStaging(VulkanBufferData& dst, size_t offset, size_t size, const void* data);

	vk::Device m_device;
	vk::PhysicalDevice m_physicalDevice;
	vk::PhysicalDeviceMemoryProperties m_memoryProperties;

	// Buffer storage with free list for slot reuse
	SCP_vector<VulkanBufferData> m_buffers;
	SCP_vector<int> m_freeSlots;

	// Device limits
	size_t m_minUboAlignment = 256;

	// Uniform buffer bindings - tracked per binding point
	std::map<uniform_block_type, BoundUniformBuffer> m_boundUniformBuffers;
	
	// Descriptor set for uniform buffers (allocated per frame)
	vk::DescriptorSet m_uniformDescriptorSet = nullptr;
	vk::DescriptorSetLayout m_uniformDescriptorSetLayout = nullptr;

	// Placeholder buffer for uninitialized uniform bindings (16 bytes, all zeros)
	vk::Buffer m_placeholderUniformBuffer = nullptr;
	vk::DeviceMemory m_placeholderUniformMemory = nullptr;
	static constexpr size_t PLACEHOLDER_BUFFER_SIZE = 256;  // Must be >= largest UBO alignment

	// Reference to descriptor manager (set by VulkanRenderer)
	VulkanDescriptorManager* m_descriptorManager = nullptr;

	// Track if uniform descriptor set has EVER been bound (prevents update-after-bind)
	// Once bound to any command buffer, we can NEVER update it again - only use dynamic offsets
	bool m_uniformSetEverBound = false;

	// Frame tracking - index into frames in flight (0 to FRAMES_IN_FLIGHT-1)
	uint32_t m_currentFrameIndex = 0;

	// Per-frame deferred deletion queues
	// Queue[i] contains resources to delete when frame i's fence is next waited on
	std::array<SCP_vector<PendingBufferDeletion>, FRAMES_IN_FLIGHT> m_pendingDeletions;

	// Per-frame staging buffer deletion queues (separate from regular buffers)
	std::array<SCP_vector<PendingBufferDeletion>, FRAMES_IN_FLIGHT> m_pendingStagingDeletions;

	// Per-frame deferred deletion queue for descriptor sets
	// Used when uniform buffer is reallocated and descriptor set must be recreated
	std::array<SCP_vector<vk::DescriptorSet>, FRAMES_IN_FLIGHT> m_pendingDescriptorSetDeletions;

	// Command pool and queue for transfer operations (uses graphics queue for synchronization)
	vk::CommandPool m_commandPool;
	vk::Queue m_graphicsQueue;
	uint32_t m_graphicsQueueFamily = 0;

	// Per-frame transfer command buffers for batched async transfers
	std::array<vk::CommandBuffer, FRAMES_IN_FLIGHT> m_transferCmds{};
	std::array<bool, FRAMES_IN_FLIGHT> m_transferCmdRecording{};
	std::array<vk::Fence, FRAMES_IN_FLIGHT> m_transferFences{};
	std::array<bool, FRAMES_IN_FLIGHT> m_transferFenceSubmitted{};  // True if fence was submitted and needs wait

	bool m_initialized = false;
};

// Global buffer manager instance (set by VulkanRenderer)
extern VulkanBufferManager* g_vulkanBufferManager;

// Function pointer implementations for gr_screen
gr_buffer_handle gr_vulkan_create_buffer(BufferType type, BufferUsageHint usage);
void gr_vulkan_delete_buffer(gr_buffer_handle handle);
void gr_vulkan_update_buffer_data(gr_buffer_handle handle, size_t size, const void* data);
void gr_vulkan_update_buffer_data_offset(gr_buffer_handle handle, size_t offset, size_t size, const void* data);
void* gr_vulkan_map_buffer(gr_buffer_handle handle);
void gr_vulkan_flush_mapped_buffer(gr_buffer_handle handle, size_t offset, size_t size);
void gr_vulkan_bind_uniform_buffer(uniform_block_type bindPoint, size_t offset, size_t size, gr_buffer_handle handle);

} // namespace vulkan
} // namespace graphics
