
#pragma once

#include "globalincs/pstypes.h"

#include <exception>
#include <stdexcept>

namespace libs {
namespace ffmpeg {

void initialize();

class FFmpegException: public std::runtime_error {
 public:
	FFmpegException(const std::string& msg) : std::runtime_error(msg) {}
	~FFmpegException() throw() {}
};

}
}
