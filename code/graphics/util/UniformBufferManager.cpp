

#include "UniformBufferManager.h"
#include "uniform_structs.h"

#include "tracing/tracing.h"

namespace {

size_t getElementSize(uniform_block_type type)
{
	switch (type) {
	case uniform_block_type::Lights:
		return sizeof(graphics::deferred_light_data);
	case uniform_block_type::ModelData:
		return sizeof(graphics::model_uniform_data);
	case uniform_block_type::NanoVGData:
		return sizeof(graphics::nanovg_draw_data);
	case uniform_block_type::DecalInfo:
		return sizeof(graphics::decal_info);
	case uniform_block_type::Matrices:
		return sizeof(graphics::matrix_uniforms);
	case uniform_block_type::MovieData:
		return sizeof(graphics::movie_uniforms);
	case uniform_block_type::NUM_BLOCK_TYPES:
	default:
		UNREACHABLE("Invalid block type encountered!");
		return 0;
	}
}

size_t getHeaderSize(uniform_block_type type)
{
	switch (type) {
	case uniform_block_type::Lights:
		return sizeof(graphics::deferred_global_data);
	case uniform_block_type::DecalInfo:
		return sizeof(graphics::decal_globals);
	case uniform_block_type::ModelData:
	case uniform_block_type::NanoVGData:
	case uniform_block_type::Matrices:
	case uniform_block_type::MovieData:
	case uniform_block_type::GenericData:
		return 0;
	case uniform_block_type::NUM_BLOCK_TYPES:
	default:
		UNREACHABLE("Invalid block type encountered!");
		return 0;
	}
}

} // namespace

namespace graphics {
namespace util {

UniformBufferManager::UniformBufferManager()
{
	bool success = gr_get_property(gr_property::UNIFORM_BUFFER_OFFSET_ALIGNMENT, &_offset_alignment);
	Assertion(success, "Uniform buffer usage requires a backend which allows to query the offset alignment!");

	_use_persistent_mapping = gr_is_capable(CAPABILITY_PERSISTENT_BUFFER_MAPPING);

	_segment_fences.fill(nullptr);
	changeSegmentSize(4096);
}
UniformBufferManager::~UniformBufferManager()
{
	if (_active_uniform_buffer.isValid()) {
		gr_delete_buffer(_active_uniform_buffer);
		_active_uniform_buffer = gr_buffer_handle();
	}
	for (auto& fence : _segment_fences) {
		if (fence != nullptr) {
			gr_sync_delete(fence);
			fence = nullptr;
		}
	}

	for (auto& buffer : _retired_buffers) {
		gr_sync_delete(std::get<0>(buffer));
		gr_delete_buffer(std::get<1>(buffer));
		// The shadow buffer pointer will be deleted automatically
	}
	_retired_buffers.clear();
}
void UniformBufferManager::onFrameEnd()
{
	GR_DEBUG_SCOPE("Performing uniform frame end operations");

	if (_segment_offset > _segment_size) {
		// We needed more data than what is available in the segment
		changeSegmentSize(_segment_offset);
	} else {
		// Set up the fence for the currently active segment
		_segment_fences[_active_segment] = gr_sync_fence();

		// Move the current segment to the next one
		_active_segment = (_active_segment + 1) % NUM_SEGMENTS;
		_segment_offset = 0;

		// Now we need to wait until the segment is available again. In most cases this should succeed immediately.
		if (_segment_fences[_active_segment] != nullptr) {
			int i = 0;
			while (i < 10 && !gr_sync_wait(_segment_fences[_active_segment], 500000000)) {
				// This isn't good!
				mprintf(("Missed uniform fence deadline!!\n"));
				++i;
			}
			gr_sync_delete(_segment_fences[_active_segment]);
			_segment_fences[_active_segment] = nullptr;

			if (i == 10) {
				// I don't know how to handle this properly but this probably means that something went wrong with the
				// GPU
				Error(LOCATION, "Failed to wait until uniform range is available! Get a coder.");
			}
		}
	}

	while (!_retired_buffers.empty()) {
		if (gr_sync_wait(std::get<0>(_retired_buffers.front()), 0)) {
			// Fence was signaled => buffer is not in use anymore
			// Delete the sync object since we don't need it anymore
			gr_sync_delete(std::get<0>(_retired_buffers.front()));
			gr_delete_buffer(std::get<1>(_retired_buffers.front()));

			// This will automatically clean up the shadow buffer pointer
			_retired_buffers.erase(_retired_buffers.begin());
		} else {
			// The first fence element was not signaled yet so all the other fences also haven't been signaled yet
			break;
		}
	}
}
UniformBuffer UniformBufferManager::getUniformBuffer(uniform_block_type type, size_t num_elements,
                                                     size_t element_size_override)
{
	if (element_size_override == 0) {
		element_size_override = getElementSize(type);
	}
	auto size = UniformAligner::getBufferSize(num_elements, (size_t)_offset_alignment, element_size_override,
	                                          getHeaderSize(type));

	auto end_offset = _segment_offset + size;

	auto absolute_end = _segment_size * _active_segment + end_offset;

	if (absolute_end >= _active_buffer_size) {
		// This new element uses too much memory to fit into the active buffer so we need to allocate a new one right
		// now. It may happen that we use more than the segment size but that is not an issue since the frame-end code
		// will reallocate the buffer if that happens.
		// We don't really know how much we are going to need here so 2 times the current amount seems like a good idea
		changeSegmentSize(_segment_size * 2);

		// Try the stuff above again.
		return getUniformBuffer(type, num_elements, element_size_override);
	}

	auto data_offset = _segment_size * _active_segment + _segment_offset;
	_segment_offset  = end_offset;

	// Even in the persistent mapping case we still use a temporary buffer since writing to GPU memory is not very fast
	// when doing a lot of small writes (e.g. when building model uniform data). Instead we use a shadow buffer and
	// do a single memcpy when we are done
	return UniformBuffer(this, data_offset, _shadow_uniform_buffer.get() + data_offset, size, element_size_override,
	                     getHeaderSize(type), static_cast<size_t>(_offset_alignment));
}
void UniformBufferManager::changeSegmentSize(size_t new_size)
{
	if (_active_uniform_buffer.isValid()) {
		// Retire the old buffer first
		_retired_buffers.emplace_back(gr_sync_fence(), _active_uniform_buffer, std::move(_shadow_uniform_buffer));
	}

	// The current fences are meaningless now so we need to delete them
	for (auto& fence : _segment_fences) {
		if (fence != nullptr) {
			gr_sync_delete(fence);
			fence = nullptr;
		}
	}

	_active_buffer_size = new_size * NUM_SEGMENTS;
	_shadow_uniform_buffer.reset(new uint8_t[_active_buffer_size]);
	_active_uniform_buffer = gr_create_buffer(
	    BufferType::Uniform, _use_persistent_mapping ? BufferUsageHint::PersistentMapping : BufferUsageHint::Dynamic);

	gr_update_buffer_data(_active_uniform_buffer, _active_buffer_size, nullptr);
	if (_use_persistent_mapping) {
		_buffer_ptr = gr_map_buffer(_active_uniform_buffer);
	}

	_active_segment = 0;
	_segment_size   = new_size;
	_segment_offset = 0;
}
void UniformBufferManager::submitData(void* buffer, size_t data_size, size_t offset)
{
	if (_use_persistent_mapping) {
		auto buffer_dest = reinterpret_cast<void*>(reinterpret_cast<uint8_t*>(_buffer_ptr) + offset);
		memcpy(buffer_dest, buffer, data_size);
		// The data is already in the buffer but we still need to flush the memory range
		gr_flush_mapped_buffer(_active_uniform_buffer, offset, data_size);
	} else {
		gr_update_buffer_data_offset(_active_uniform_buffer, offset, data_size, buffer);
	}
}
gr_buffer_handle UniformBufferManager::getActiveBufferHandle() { return _active_uniform_buffer; }
size_t UniformBufferManager::getBufferSize() { return _active_buffer_size; }
size_t UniformBufferManager::getCurrentlyUsedSize() { return _segment_offset; }
} // namespace util
} // namespace graphics
