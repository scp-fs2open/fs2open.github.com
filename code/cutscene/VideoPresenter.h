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
	MovieProperties _properties;

	std::array<int, 3> _planeTextureHandles;
	std::array<std::unique_ptr<uint8_t[]>, 3> _planeTextureBuffers;

	movie_material _movie_material;
	material _rgb_material; // Material used when a RGB/RGBA movie is played
 public:
	explicit VideoPresenter(const MovieProperties& props);

	// Disallow copying
	VideoPresenter(const VideoPresenter&) = delete;
	VideoPresenter& operator=(const VideoPresenter&) = delete;

	virtual ~VideoPresenter();

	void uploadVideoFrame(const VideoFramePtr& frame);

	void displayFrame(float x1, float y1, float x2, float y2, float alpha = 1.0f);
};
}
}


#endif //FS2_OPEN_OPENGLVIDEOPRESENTER_H
