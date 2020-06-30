#pragma once

#include "osapi/osapi.h"

namespace graphics {
namespace vulkan {

bool initialize(std::unique_ptr<os::GraphicsOperations>&& graphicsOps);

}
} // namespace graphics
