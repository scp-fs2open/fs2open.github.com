
#ifndef FS2_OPEN_OPENGLVIDEOPRESENTER_H
#define FS2_OPEN_OPENGLVIDEOPRESENTER_H

#include "globalincs/pstypes.h"
#include "cutscene/player/VideoPresenter.h"

#include <glad/glad.h>

#include <memory>

namespace cutscene {
namespace player {
class OpenGLVideoPresenter: public VideoPresenter {
	int _vertexBuffer;

	bool _scaleVideo;

	GLuint _ytex;
	GLuint _utex;
	GLuint _vtex;
 public:
	OpenGLVideoPresenter(const MovieProperties& props);

	virtual ~OpenGLVideoPresenter();

	virtual void uploadVideoFrame(const VideoFramePtr& frame) SCP_OVERRIDE;

	virtual void displayFrame() SCP_OVERRIDE;
};
}
}


#endif //FS2_OPEN_OPENGLVIDEOPRESENTER_H
