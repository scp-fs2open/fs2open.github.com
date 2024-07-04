#pragma once

// Disable a few warnings that happen in the ffmpeg headers
// TODO: Also do this for other compilers
#pragma warning(push)
#pragma warning(disable: 4244) // conversion from 'int' to '*'

extern "C" {
#include <libavcodec/avcodec.h>
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

/**
 * Extract version components from the full ::AV_VERSION_INT int as returned
 * by functions like ::avformat_version() and ::avcodec_version()
 * Brought in from a newer FFMpeg for compat with older library versions
 */
#ifndef AV_VERSION_MAJOR
#define AV_VERSION_MAJOR(a) ((a) >> 16)
#endif
#ifndef AV_VERSION_MINOR
#define AV_VERSION_MINOR(a) (((a) & 0x00FF00) >> 8)
#endif
#ifndef AV_VERSION_MICRO
#define AV_VERSION_MICRO(a) ((a) & 0xFF)
#endif
