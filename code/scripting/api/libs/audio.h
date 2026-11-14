#pragma once

#include "scripting/ade_api.h"

namespace scripting {
namespace api {

DECLARE_ADE_LIB(l_Audio);

// Remember a stream opened by a script, so it can be closed at the end of the mission if the script doesn't close it
void script_music_track(int signature);

// Close script streams from this mission that are no longer playing
void script_music_close();

}
}

