#pragma once

// Disable a few warnings that happen in the ffmpeg headers
// TODO: Also do this for other compilers
#pragma warning(push)
#pragma warning(disable: 4244) // conversion from 'int' to '*'

extern "C" {
#include <libavcodec/version.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>

#ifdef WITH_LIBAV
#include <libavresample/avresample.h>
#else
#include <libswresample/swresample.h>
#endif

#include <libavutil/imgutils.h>
#include <libavutil/error.h>
#include <libavutil/opt.h>
}

#ifdef WITH_LIBAV
	#include "LibAVCompatibility.h"
#endif

#pragma warning(pop)
