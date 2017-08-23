#ifndef FS2_OPEN_OPENGLVIDEOPRESENTER_H
#define FS2_OPEN_OPENGLVIDEOPRESENTER_H

#include "globalincs/pstypes.h"
#include "graphics/2d.h"
#include "graphics/material.h"
#include "cutscene/Decoder.h"

#include <memory>

namespace cutscene {
namespace player {

class VideoPresenter {
	int _vertexBuffer = -1;
	vertex_layout _vertexLayout;

	bool _scaleVideo = false;

	int _ytex = -1;
	std::unique_ptr<uint8_t[]> _yTexBuffer;

	int _utex = -1;
	std::unique_ptr<uint8_t[]> _uTexBuffer;

	int _vtex = -1;
	std::unique_ptr<uint8_t[]> _vTexBuffer;

	movie_material _render_material;
 public:
	explicit VideoPresenter(const MovieProperties& props);

	// Disallow copying
	VideoPresenter(const VideoPresenter&) = delete;
	VideoPresenter& operator=(const VideoPresenter&) = delete;

	virtual ~VideoPresenter();

	void uploadVideoFrame(const VideoFramePtr& frame);

	void displayFrame();
};
}
}


#endif //FS2_OPEN_OPENGLVIDEOPRESENTER_H
