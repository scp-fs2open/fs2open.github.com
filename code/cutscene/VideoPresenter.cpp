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
VideoPresenter::VideoPresenter(const MovieProperties& props) : _properties(props)
{
	GR_DEBUG_SCOPE("Init video");

	_planeTextureHandles.fill(-1);

	auto w = static_cast<int>(props.size.width);
	auto h = static_cast<int>(props.size.height);

	switch (props.pixelFormat) {
	case FramePixelFormat::YUV420:
		_planeTextureBuffers[0].reset(new uint8_t[w * h]);
		_planeTextureHandles[0] = bm_create(8, w, h, _planeTextureBuffers[0].get(), BMP_AABITMAP);

		_planeTextureBuffers[1].reset(new uint8_t[w / 2 * h / 2]);
		_planeTextureHandles[1] = bm_create(8, w / 2, h / 2, _planeTextureBuffers[1].get(), BMP_AABITMAP);

		_planeTextureBuffers[2].reset(new uint8_t[w / 2 * h / 2]);
		_planeTextureHandles[2] = bm_create(8, w / 2, h / 2, _planeTextureBuffers[2].get(), BMP_AABITMAP);

		material_set_movie(&_movie_material, _planeTextureHandles[0], _planeTextureHandles[1], _planeTextureHandles[2]);
		break;
	case FramePixelFormat::BGR:
		_planeTextureBuffers[0].reset(new uint8_t[w * h * 3]);
		_planeTextureHandles[0] = bm_create(24, w, h, _planeTextureBuffers[0].get(), 0);

		material_set_unlit(&_rgb_material, _planeTextureHandles[0], 1.0f, false, false);
		break;
	case FramePixelFormat::BGRA:
		_planeTextureBuffers[0].reset(new uint8_t[w * h * 4]);
		_planeTextureHandles[0] = bm_create(32, w, h, _planeTextureBuffers[0].get(), 0);

		material_set_unlit(&_rgb_material, _planeTextureHandles[0], 1.0f, true, false);
		break;
	default:
		UNREACHABLE("Unhandled enum value!");
		break;
	}
}

VideoPresenter::~VideoPresenter() {
	GR_DEBUG_SCOPE("Deinit video");

	for (auto& handle : _planeTextureHandles) {
		if (handle > 0) {
			bm_release(handle);
			handle = -1;
		}
	}
}

void VideoPresenter::uploadVideoFrame(const VideoFramePtr& frame) {
	GR_DEBUG_SCOPE("Update video frame");

	for (size_t i = 0; i < frame->getPlaneNumber(); ++i) {
		auto size = frame->getPlaneSize(i);
		auto data = frame->getPlaneData(i);

		memcpy(_planeTextureBuffers[i].get(), data, size.stride * size.height);

		int bpp = 0;
		switch (_properties.pixelFormat) {
		case FramePixelFormat::YUV420:
			bpp = 8;
			break;
		case FramePixelFormat::BGR:
			bpp = 24;
			break;
		case FramePixelFormat::BGRA:
			bpp = 32;
			break;
		default:
			UNREACHABLE("Unhandled enum value!");
			break;
		}

		gr_update_texture(_planeTextureHandles[i], bpp, _planeTextureBuffers[i].get(), static_cast<int>(size.width),
		                  static_cast<int>(size.height));
	}
}

void VideoPresenter::displayFrame(float x1, float y1, float x2, float y2, float alpha) {
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
	layout.add_vertex_component(vertex_format_data::POSITION2, sizeof(vertex_data[0]), offsetof(movie_vertex, pos));
	layout.add_vertex_component(vertex_format_data::TEX_COORD2, sizeof(vertex_data[0]), offsetof(movie_vertex, uv));

	switch (_properties.pixelFormat) {
	case FramePixelFormat::YUV420:
		if (alpha < 1.0f)
			material_set_movie(&_movie_material,
				_planeTextureHandles[0],
				_planeTextureHandles[1],
				_planeTextureHandles[2],
				alpha);
		gr_render_movie(&_movie_material, PRIM_TYPE_TRISTRIP, &layout, 4, gr_immediate_buffer_handle, offset);
		break;
	case FramePixelFormat::BGR:
		if (alpha < 1.0f)
			material_set_unlit(&_rgb_material, _planeTextureHandles[0], alpha, false, false);
		gr_render_primitives(&_rgb_material, PRIM_TYPE_TRISTRIP, &layout, 0, 4, gr_immediate_buffer_handle, offset);
		break;
	case FramePixelFormat::BGRA:
		if (alpha < 1.0f)
			material_set_unlit(&_rgb_material, _planeTextureHandles[0], alpha, true, false);
		gr_render_primitives(&_rgb_material, PRIM_TYPE_TRISTRIP, &layout, 0, 4, gr_immediate_buffer_handle, offset);
		break;
	default:
		UNREACHABLE("Unhandled enum value!");
		break;
	}
}

}
}
