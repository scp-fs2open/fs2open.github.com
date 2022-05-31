#pragma once

#include "scripting/ade_api.h"

namespace scripting {
namespace api {

DECLARE_ADE_LIB(l_Graphics);
static int pseudo_beam_line_texture = -1;

void graphics_on_frame();

}
}

