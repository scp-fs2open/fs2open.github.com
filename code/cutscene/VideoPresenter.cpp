#include "VideoPresenter.h"


using namespace cutscene;
using namespace cutscene::player;

namespace cutscene {
namespace player {
VideoPresenter::VideoPresenter(const MovieProperties& props) : _scaleVideo(false) {
	GR_DEBUG_SCOPE("Init video");

	_vertexBuffer = gr_create_vertex_buffer(true);

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

	gr_set_lighting(false, false);

	float screen_ratio = (float) gr_screen.center_w / (float) gr_screen.center_h;
	float movie_ratio = (float) props.size.width / (float) props.size.height;

	float scale_by;
	if (screen_ratio > movie_ratio) {
		scale_by = (float) gr_screen.center_h / (float) props.size.height;
	} else {
		scale_by = (float) gr_screen.center_w / (float) props.size.width;
	}

	// don't bother setting anything if we aren't going to need it
	if (!Cmdline_noscalevid && (scale_by != 1.0f)) {
		vec3d scale;

		scale.xyz.x = scale_by;
		scale.xyz.y = scale_by;
		scale.xyz.z = -1.0f;

		gr_push_scale_matrix(&scale);
		_scaleVideo = true;
	}

	int screenX;
	int screenY;

	if (_scaleVideo) {
		screenX = fl2i(((gr_screen.center_w / 2.0f + gr_screen.center_offset_x) / scale_by)
						   - (static_cast<int>(props.size.width) / 2.0f) + 0.5f);
		screenY = fl2i(((gr_screen.center_h / 2.0f + gr_screen.center_offset_y) / scale_by)
						   - (static_cast<int>(props.size.height) / 2.0f) + 0.5f);
	} else {
		// centers on 1024x768, fills on 640x480
		screenX = ((gr_screen.center_w - static_cast<int>(props.size.width)) / 2) + gr_screen.center_offset_x;
		screenY = ((gr_screen.center_h - static_cast<int>(props.size.height)) / 2) + gr_screen.center_offset_y;
	}

	// set additional values for screen width/height and UV coords
	int screenXW = screenX + static_cast<int>(props.size.width);
	int screenYH = screenY + static_cast<int>(props.size.height);

	float glVertices[4][4] = {{ 0 }};
	glVertices[0][0] = (float) screenX;
	glVertices[0][1] = (float) screenY;
	glVertices[0][2] = 0.0f;
	glVertices[0][3] = 0.0f;

	glVertices[1][0] = (float) screenX;
	glVertices[1][1] = (float) screenYH;
	glVertices[1][2] = 0.0f;
	glVertices[1][3] = 1.0f;

	glVertices[2][0] = (float) screenXW;
	glVertices[2][1] = (float) screenY;
	glVertices[2][2] = 1.0f;
	glVertices[2][3] = 0.0f;

	glVertices[3][0] = (float) screenXW;
	glVertices[3][1] = (float) screenYH;
	glVertices[3][2] = 1.0f;
	glVertices[3][3] = 1.0f;

	_vertexLayout.add_vertex_component(vertex_format_data::POSITION2, sizeof(glVertices[0]), 0);
	_vertexLayout.add_vertex_component(vertex_format_data::TEX_COORD2, sizeof(glVertices[0]), sizeof(float) * 2);

	gr_update_buffer_data(_vertexBuffer, sizeof(glVertices[0]) * 4, glVertices);
}

VideoPresenter::~VideoPresenter() {
	GR_DEBUG_SCOPE("Deinit video");

	if (_scaleVideo) {
		gr_pop_scale_matrix();
	}

	gr_delete_buffer(_vertexBuffer);
	_vertexBuffer = -1;

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

void VideoPresenter::displayFrame() {
	GR_DEBUG_SCOPE("Draw video frame");

	gr_render_movie(&_render_material, PRIM_TYPE_TRISTRIP, &_vertexLayout, 4, _vertexBuffer);
}

}
}
