

#include "UniformBufferManager.h"
#include "uniform_structs.h"

namespace {

size_t getElementSize(uniform_block_type type) {
	switch (type) {
	case uniform_block_type::Lights:
		return sizeof(graphics::deferred_light_data);
	case uniform_block_type::ModelData:
		return sizeof(graphics::model_uniform_data);
	case uniform_block_type::NanoVGData:
		return sizeof(graphics::nanovg_draw_data);
	case uniform_block_type::DecalInfo:
		return sizeof(graphics::decal_info);
	case uniform_block_type::NUM_BLOCK_TYPES:
	default:
		UNREACHABLE("Invalid block type encountered!");
		return 0;
	}
}

size_t getHeaderSize(uniform_block_type type) {
	switch (type) {
	case uniform_block_type::Lights:
		return sizeof(graphics::deferred_global_data);
	case uniform_block_type::DecalInfo:
		return sizeof(graphics::decal_globals);
	case uniform_block_type::ModelData:
	case uniform_block_type::NanoVGData:
		return 0;
	case uniform_block_type::NUM_BLOCK_TYPES:
	default:
		UNREACHABLE("Invalid block type encountered!");
		return 0;
	}
}

}

namespace graphics {
namespace util {


UniformBufferManager::UniformBufferManager() {
	bool success = gr_get_property(gr_property::UNIFORM_BUFFER_OFFSET_ALIGNMENT, &_offset_alignment);

	Assertion(success, "Uniform buffer usage requires a backend which allows to query the offset alignment!");

	_segment_fences.fill(nullptr);
	changeSegmentSize(4096);
}
UniformBufferManager::~UniformBufferManager() {
	if (_active_uniform_buffer >= 0) {
		gr_delete_buffer(_active_uniform_buffer);
		_active_uniform_buffer = -1;
	}
	for (auto& fence : _segment_fences) {
		if (fence != nullptr) {
			gr_sync_delete(fence);
			fence = nullptr;
		}
	}

	for (auto& buffer : _retired_buffers) {
		gr_sync_delete(buffer.second);
		gr_delete_buffer(buffer.first);
	}
	_retired_buffers.clear();
}
void UniformBufferManager::onFrameEnd() {
	GR_DEBUG_SCOPE("Performing uniform frame end operations");

	if (_segment_offset > _segment_size) {
		// We needed more data than what is available in the segment
		changeSegmentSize(_segment_offset);
	} else {
		// Set up the fence for the currently active segment
		_segment_fences[_active_segment] = gr_sync_fence();

		_active_segment = (_active_segment + 1) % NUM_SEGMENTS;
		_segment_offset = 0;

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
				// I don't know how to handle this properly but this probably means that something went wrong with the GPU
				Error(LOCATION, "Failed to wait until uniform range is available! Get a coder.");
			}
		}

	}

	while (!_retired_buffers.empty()) {
		if (gr_sync_wait(_retired_buffers.front().second, 0)) {
			// Fence was signaled => buffer is not in use anymore
			// Delete the sync object since we don't need it anymore
			gr_sync_delete(_retired_buffers.front().second);
			gr_delete_buffer(_retired_buffers.front().first);

			_retired_buffers.erase(_retired_buffers.begin());
		} else {
			// The first fence element was not signaled yet so all the other fences also haven't been signaled yet
			break;
		}
	}
}
UniformBuffer UniformBufferManager::getUniformBuffer(uniform_block_type type, size_t num_elements) {
	auto size = UniformAligner::getBufferSize(num_elements,
											  (size_t) _offset_alignment,
											  getElementSize(type),
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
		return getUniformBuffer(type, num_elements);
	}

	if (_temp_buffer.size() < size) {
		_temp_buffer.resize(size);
	}

	auto data_offset = _segment_size * _active_segment + _segment_offset;
	_segment_offset = end_offset;

	return UniformBuffer(this,
						 data_offset,
						 _temp_buffer.data(),
						 size,
						 getElementSize(type),
						 getHeaderSize(type),
						 _offset_alignment);
}
void UniformBufferManager::changeSegmentSize(size_t new_size) {
	if (_active_uniform_buffer >= 0) {
		// Retire the old buffer first
		_retired_buffers.emplace_back(_active_uniform_buffer, gr_sync_fence());
	}

	// The current fences are meaningless now so we need to delete them
	for (auto& fence : _segment_fences) {
		if (fence != nullptr) {
			gr_sync_delete(fence);
			fence = nullptr;
		}
	}

	// TODO: Add persistent mapping support here
	_active_uniform_buffer = gr_create_buffer(BufferType::Uniform, BufferUsageHint::Dynamic);
	_active_buffer_size = new_size * NUM_SEGMENTS;
	gr_update_buffer_data(_active_uniform_buffer, _active_buffer_size, nullptr);

	_active_segment = 0;
	_segment_size = new_size;
	_segment_offset = 0;
}
void UniformBufferManager::submitData(void* buffer, size_t data_size, size_t offset) {
	// TODO: Add persistent mapping support here
	gr_update_buffer_data_offset(_active_uniform_buffer, offset, data_size, buffer);
}
int UniformBufferManager::getActiveBufferHandle() {
	return _active_uniform_buffer;
}

}
}
