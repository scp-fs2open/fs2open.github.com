
#ifndef FS2_OPEN_OPENGLVIDEOPRESENTER_H
#define FS2_OPEN_OPENGLVIDEOPRESENTER_H

#include "globalincs/pstypes.h"
#include "cutscene/player/VideoPresenter.h"

#include <glad/glad.h>

#include <memory>
#include <graphics/2d.h>

namespace cutscene {
namespace player {
class OpenGLVideoPresenter: public VideoPresenter {
	int _vertexBuffer = -1;
	vertex_layout _vertexLayout;
	int _sdr_handle = -1;

	bool _scaleVideo = false;

	GLuint _ytex = 0;
	GLuint _utex = 0;
	GLuint _vtex = 0;
 public:
	OpenGLVideoPresenter(const MovieProperties& props);

	virtual ~OpenGLVideoPresenter();

	virtual void uploadVideoFrame(const VideoFramePtr& frame) SCP_OVERRIDE;

	virtual void displayFrame() SCP_OVERRIDE;
};
}
}


#endif //FS2_OPEN_OPENGLVIDEOPRESENTER_H
