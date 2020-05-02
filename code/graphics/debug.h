#pragma once

#include "graphics/2d.h"
#include "math/vecmat.h"

namespace graphics {
namespace debug {

void line_3d(const vec3d* from, const vec3d* to, const color* color);

void text_3d(const vec3d* world_pos, const color* color, SCP_string text);

void render_elements();

void cleanup();

} // namespace debug
} // namespace graphics
