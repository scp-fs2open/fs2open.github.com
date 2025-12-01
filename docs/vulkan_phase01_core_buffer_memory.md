# Phase 1: Core Buffer & Memory

## Implementation Status: Complete (Functional)

### VulkanBufferManager (VulkanBuffer.h/cpp)

**Architecture:**
- Slot allocator using free list + dense array for handle management
- Per-buffer frame tracking for GPU synchronization
- Deferred deletion queue for in-flight resources
- Staging buffer system for device-local uploads

**Buffer Lifecycle:**
- `createBuffer(type, usage)` - Creates handle, allocates on first data upload
- `updateBufferData(handle, size, data)` - Full buffer replacement, auto-resizes
- `updateBufferDataOffset(handle, offset, size, data)` - Partial update
- `deleteBuffer(handle)` - Immediate or deferred deletion based on frame tracking
- `beginFrame(frameIndex)` - Processes deferred deletions after fence wait

**Memory Strategy:**
- **Static**: Device-local (VRAM) with staging copy via transfer queue
  - Falls back to host-visible if no transfer queue available
- **Dynamic/Streaming**: Host-visible + coherent for frequent CPU updates
- **PersistentMapping**: Host-visible + coherent, persistently mapped pointer

**Persistent Mapping:**
- `mapBuffer(handle)` - Returns persistent mapped pointer
- `flushMappedBuffer(handle, offset, size)` - No-op for coherent memory
- Used for streaming data (particles, dynamic text, etc.)

**Staging Uploads:**
- Creates temporary staging buffer per device-local upload
- Records vkCmdCopyBuffer on transfer queue
- Synchronous: waits on fence before cleanup
- Location: `copyViaStaging()` in VulkanBuffer.cpp:416-525

**Frame Synchronization:**
- `FRAMES_IN_FLIGHT = 2` (must match VulkanRenderer)
- Tracks `lastUsedFrame` per buffer for informational purposes
- Per-frame deletion queues: `m_pendingDeletions[FRAMES_IN_FLIGHT]`
- Resources queued on frame N are destroyed when frame N's fence is next waited on
- `beginFrame(frameIndex)` processes deletion queue for that frame after fence wait
- Prevents GPU/CPU race conditions on resize/delete operations

**Device Limits:**
- Queries `minUniformBufferOffsetAlignment` from VkPhysicalDeviceLimits
- Validates UBO offsets in `bindUniformBuffer()`
- Exposes via `getMinUniformBufferOffsetAlignment()`

**Uniform Buffer Binding:**
- `bindUniformBuffer()` (VulkanBuffer.cpp:344-366) currently validates alignment but does not bind
- Descriptor set binding deferred until VulkanDescriptorManager integration
- See comment at lines 361-363 for implementation notes

**Integration:**
- Global `g_vulkanBufferManager` instance
- Function pointers for gr_screen: `gr_vulkan_create_buffer()`, etc.
- Supports BufferType: Vertex, Index, Uniform
- Supports BufferUsageHint: Static, Dynamic, Streaming, PersistentMapping

### Completion Checklist

**Completed:**
- [x] Fix member variable name inconsistency
  - All instances now correctly use `m_currentFrameIndex`
  - No compilation errors

**Deferred (Requires Integration):**
- [ ] Implement descriptor set binding in `bindUniformBuffer()`
  - Currently only validates alignment, does not actually bind
  - Requires VulkanDescriptorManager integration (separate phase)
  - See VulkanBuffer.cpp:344-366 for placeholder comment
  - This does not block basic buffer functionality

### Testing Coverage

**Status:** No unit tests currently exist for VulkanBufferManager.

**Test file location:** `test/src/graphics/test_vulkan.cpp` (currently contains only VulkanShader tests)

**Recommended test coverage:**
- Buffer creation/deletion with frame sync
- Staging upload validation (device-local buffers)
- Persistent mapping lifecycle (map/flush operations)
- Deferred deletion queue behavior across frame boundaries
- Memory type selection logic (static vs dynamic vs streaming)
- Alignment validation for UBO offsets
