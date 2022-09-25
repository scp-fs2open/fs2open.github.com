#pragma once
#include "lab/manager/lab_manager.h"
#include "imgui.h"
#include "backends/imgui_impl_opengl3.h"
#include "backends/imgui_impl_sdl.h"


const std::unique_ptr<LabManager> &getLabManager();
