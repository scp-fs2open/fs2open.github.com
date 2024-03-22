#pragma once
#include "options/manager/ingame_options_manager.h"
#include "imconfig.h"
#include "imgui.h"
#include "backends/imgui_impl_opengl3.h"
#include "backends/imgui_impl_sdl.h"
#include "extensions/imgui_sugar.hpp"

const std::unique_ptr<OptConfigurator>& getOptConfigurator();
