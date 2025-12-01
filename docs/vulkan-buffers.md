# Vulkan Buffer Management

## Overview

Buffer management for vertex, index, and uniform data. Located in `code/graphics/vulkan/VulkanBuffer.cpp/h`.

## VulkanBufferManager

### Initialization
```cpp
VulkanBufferManager(
    vk::Device device,
    vk::PhysicalDevice physicalDevice,
    vk::Queue transferQueue,
    uint32_t transferQueueFamilyIndex
);
```

Global instance: `g_vulkanBufferManager`

### Configuration
```cpp
static constexpr vk::DeviceSize STAGING_BUFFER_SIZE = 64 * 1024 * 1024;  // 64MB
static constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;
```

## Buffer Types

### Vertex Buffers
- Usage: `eVertexBuffer | eTransferDst`
- Memory: `eDeviceLocal` (GPU-only)
- Uploaded via staging buffer

### Index Buffers
- Usage: `eIndexBuffer | eTransferDst`
- Memory: `eDeviceLocal` (GPU-only)
- Uploaded via staging buffer

### Uniform Buffers
- Usage: `eUniformBuffer`
- Memory: `eHostVisible | eHostCoherent` (CPU-mapped)
- Direct write, no staging needed

### Staging Buffers
- Usage: `eTransferSrc`
- Memory: `eHostVisible | eHostCoherent`
- Ring buffer for uploads

## Staging Buffer Strategy

### Ring Buffer Layout
```
|--- Frame 0 ---|--- Frame 1 ---|
|    32 MB      |    32 MB      |
```

### Per-Frame Partitioning
Each frame-in-flight gets its own partition:
```cpp
vk::DeviceSize framePartitionSize = STAGING_BUFFER_SIZE / MAX_FRAMES_IN_FLIGHT;
vk::DeviceSize frameOffset = currentFrame * framePartitionSize;
```

### Allocation
```cpp
void* allocateStagingMemory(vk::DeviceSize size, vk::DeviceSize& outOffset) {
    // Check if fits in current frame's partition
    if (m_stagingOffset + size > framePartitionEnd) {
        // Wait for this frame's previous upload to complete
        waitForFence(m_frameFences[m_currentFrame]);
        m_stagingOffset = frameOffset;  // Reset to frame start
    }

    outOffset = m_stagingOffset;  // Capture BEFORE alignment
    void* ptr = mappedMemory + m_stagingOffset;
    m_stagingOffset += size;
    m_stagingOffset = align(m_stagingOffset, 4);  // 4-byte alignment

    return ptr;
}
```

## Memory Type Selection

```cpp
uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties) {
    auto memProps = physicalDevice.getMemoryProperties();
    for (uint32_t i = 0; i < memProps.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) &&
            (memProps.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }
    throw std::runtime_error("Failed to find suitable memory type");
}
```

### Common Memory Configurations

| Use Case | Properties |
|----------|------------|
| GPU-only (vertex/index) | `eDeviceLocal` |
| CPU-visible (uniform) | `eHostVisible \| eHostCoherent` |
| Staging | `eHostVisible \| eHostCoherent` |
| Readback | `eHostVisible \| eHostCached` |

## Upload Commands

### Single Buffer Upload
```cpp
void uploadToBuffer(vk::Buffer dst, const void* data, vk::DeviceSize size) {
    vk::DeviceSize offset;
    void* staging = allocateStagingMemory(size, offset);
    memcpy(staging, data, size);

    vk::BufferCopy region{offset, 0, size};
    cmd.copyBuffer(stagingBuffer, dst, region);
}
```

### Batched Uploads
```cpp
void beginUploadBatch();
void queueUpload(vk::Buffer dst, const void* data, vk::DeviceSize size);
void endUploadBatch();  // Submits all queued copies
```

## Synchronization

### Fence Management
```cpp
// Per-frame fences for upload synchronization
std::array<vk::UniqueFence, MAX_FRAMES_IN_FLIGHT> m_uploadFences;

// Wait before reusing staging memory
void waitForFrame(uint32_t frameIndex) {
    device.waitForFences(m_uploadFences[frameIndex].get(), VK_TRUE, UINT64_MAX);
    device.resetFences(m_uploadFences[frameIndex].get());
}
```

### Command Buffer Reset
Command pool created with `eResetCommandBuffer` flag:
```cpp
poolCreate.flags |= vk::CommandPoolCreateFlagBits::eTransient |
                    vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
```

Reset before each recording:
```cpp
cmd.reset(vk::CommandBufferResetFlags{});
cmd.begin(vk::CommandBufferBeginInfo{vk::CommandBufferUsageFlagBits::eOneTimeSubmit});
```

## Alignment Requirements

### Buffer Offsets
- Uniform buffer: `minUniformBufferOffsetAlignment` (typically 256)
- Storage buffer: `minStorageBufferOffsetAlignment` (typically 256)
- Vertex buffer: No alignment required for offset
- Index buffer: Element size alignment (2 or 4 bytes)

### Memory Allocation
- General staging: 4-byte alignment
- Texture data: Format-specific (4 bytes for RGBA8, etc.)

## Dynamic Uniform Buffers

For per-draw uniforms:
```cpp
// Allocate aligned slots in uniform buffer
size_t alignedSize = align(sizeof(UniformData), minUniformBufferOffsetAlignment);
uint32_t dynamicOffset = currentSlot * alignedSize;

// Bind with dynamic offset
cmd.bindDescriptorSets(bindPoint, layout, 0, descriptorSet, dynamicOffset);
```

## Buffer Destruction

RAII via `vk::UniqueBuffer` and `vk::UniqueDeviceMemory`:
```cpp
struct ManagedBuffer {
    vk::UniqueBuffer buffer;
    vk::UniqueDeviceMemory memory;
    vk::DeviceSize size;
    void* mapped = nullptr;  // For host-visible buffers
};
```

## Performance Tips

1. **Batch uploads** - Minimize command buffer submissions
2. **Per-frame partitioning** - Avoid mid-frame stalls
3. **Persistent mapping** - Map uniform buffers once at creation
4. **Alignment padding** - Pre-calculate to avoid waste
5. **Transfer queue** - Use dedicated queue if available
