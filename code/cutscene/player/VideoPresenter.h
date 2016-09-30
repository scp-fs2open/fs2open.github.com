
#ifndef FS2_OPEN_VIDEOPRESENTER_H
#define FS2_OPEN_VIDEOPRESENTER_H

#include "globalincs/pstypes.h"
#include "cutscene/Decoder.h"

namespace cutscene {
namespace player {
class VideoPresenter {
 public:
	virtual ~VideoPresenter() {}

	virtual void uploadVideoFrame(const VideoFramePtr& frame) = 0;

	virtual void displayFrame() = 0;
};
}
}

#endif //FS2_OPEN_VIDEOPRESENTER_H
