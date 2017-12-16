#include "VideoPresenter.h"

#include "graphics/matrix.h"

using namespace cutscene;
using namespace cutscene::player;

namespace {
struct movie_vertex {
	vec2d pos;
	vec2d uv;
};
}

namespace cutscene {
namespace player {
VideoPresenter::VideoPresenter(const MovieProperties& props) {
	GR_DEBUG_SCOPE("Init video");

	auto w = static_cast<int>(props.size.width);
	auto h = static_cast<int>(props.size.height);

	// Create buffer textures
	_yTexBuffer.reset(new uint8_t[w * h]);
	_ytex = bm_create(8, w, h, _yTexBuffer.get(), BMP_AABITMAP);

	_uTexBuffer.reset(new uint8_t[w / 2 * h / 2]);
	_utex = bm_create(8, w / 2, h / 2, _uTexBuffer.get(), BMP_AABITMAP);

	_vTexBuffer.reset(new uint8_t[w / 2 * h / 2]);
	_vtex = bm_create(8, w / 2, h / 2, _vTexBuffer.get(), BMP_AABITMAP);

	material_set_movie(&_render_material, _ytex, _utex, _vtex);
}

VideoPresenter::~VideoPresenter() {
	GR_DEBUG_SCOPE("Deinit video");

	bm_release(_ytex);
	bm_release(_utex);
	bm_release(_vtex);

	_ytex = _utex = _vtex = -1;
}

void VideoPresenter::uploadVideoFrame(const VideoFramePtr& frame) {
	GR_DEBUG_SCOPE("Update video frame");
	auto ptrs = frame->getDataPointers();

	memcpy(_yTexBuffer.get(), ptrs.y, frame->ySize.stride * frame->ySize.height);
	memcpy(_uTexBuffer.get(), ptrs.u, frame->uvSize.stride * frame->uvSize.height);
	memcpy(_vTexBuffer.get(), ptrs.v, frame->uvSize.stride * frame->uvSize.height);

	gr_update_texture(_ytex, 8, _yTexBuffer.get(), (int) frame->ySize.width, (int) frame->ySize.height);
	gr_update_texture(_utex, 8, _uTexBuffer.get(), (int) frame->uvSize.width, (int) frame->uvSize.height);
	gr_update_texture(_vtex, 8, _vTexBuffer.get(), (int) frame->uvSize.width, (int) frame->uvSize.height);
}

void VideoPresenter::displayFrame(float x1, float y1, float x2, float y2) {
	GR_DEBUG_SCOPE("Draw video frame");

	movie_vertex vertex_data[4];

	vertex_data[0].pos.x = x1;
	vertex_data[0].pos.y = y1;
	vertex_data[0].uv.x = 0.0f;
	vertex_data[0].uv.y = 0.0f;

	vertex_data[1].pos.x = x1;
	vertex_data[1].pos.y = y2;
	vertex_data[1].uv.x = 0.0f;
	vertex_data[1].uv.y = 1.0f;

	vertex_data[2].pos.x = x2;
	vertex_data[2].pos.y = y1;
	vertex_data[2].uv.x = 1.0f;
	vertex_data[2].uv.y = 0.0f;

	vertex_data[3].pos.x = x2;
	vertex_data[3].pos.y = y2;
	vertex_data[3].uv.x = 1.0f;
	vertex_data[3].uv.y = 1.0f;

	// Use the immediate buffer for storing our data since that is exactly what we need
	auto offset = gr_add_to_immediate_buffer(sizeof(vertex_data), vertex_data);

	vertex_layout layout;
	layout.add_vertex_component(vertex_format_data::POSITION2, sizeof(vertex_data[0]), offset + offsetof(movie_vertex, pos));
	layout.add_vertex_component(vertex_format_data::TEX_COORD2, sizeof(vertex_data[0]), offset + offsetof(movie_vertex, uv));

	gr_render_movie(&_render_material, PRIM_TYPE_TRISTRIP, &layout, 4, gr_immediate_buffer_handle);
}

}
}
