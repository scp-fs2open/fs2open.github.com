#include "grinternal.h"

bool Rendering_to_shadow_map = false;
bool Scene_framebuffer_in_frame = false;

namespace graphics {

std::unique_ptr<PostProcessingManager> Post_processing_manager;

}
