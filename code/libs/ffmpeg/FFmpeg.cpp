
#include "libs/ffmpeg/FFmpeg.h"

#include "FFmpegHeaders.h"

#include "parse/parselo.h"

namespace {
const int MIN_LOG_LEVEL = AV_LOG_ERROR;

bool initialized = false;

void log_callback_report(void* ptr, int level, const char* fmt, va_list vl) {
	if (level > MIN_LOG_LEVEL) {
		return;
	}

	char buffer[1024];
	int print_prefix = 1;
	av_log_format_line(ptr, level, fmt, vl, buffer, sizeof(buffer), &print_prefix);

	mprintf(("FFMPEG Log: %s", buffer)); // no \n, ffmpeg handles that
}

void check_version(const char* libname, uint32_t current, uint32_t compiled)
{
	auto current_major = AV_VERSION_MAJOR(current);
	auto current_minor = AV_VERSION_MINOR(current);

	auto compiled_major = AV_VERSION_MAJOR(compiled);
	auto compiled_minor = AV_VERSION_MINOR(compiled);

	if (current_major != compiled_major)
	{
		Error(LOCATION, "The major version of the %s library is not the same as the one this executable was compiled with!\n"
			"Current major version is %" PRIu32 " but this executable was compiled with major version %" PRIu32 ".\n"
			"This may be caused by using outdated DLLs, if you downloaded these builds then try reextracing the zip file.", libname, current_major, compiled_major);
	}

	if (current_minor < compiled_minor)
	{
		Error(LOCATION, "The minor version of the %s library is not the same as the one this executable was compiled with!\n"
			"Current minor version is %" PRIu32 " but this executable was compiled with minor version %" PRIu32 ".\n"
			"This may be caused by using outdated DLLs, if you downloaded these builds then try reextracing the zip file.", libname, current_minor, compiled_minor);
	}
}
}

namespace libs {
namespace ffmpeg {
void initialize() {
	if (initialized) {
		return;
	}

	av_register_all();

	check_version("libavcodec", avcodec_version(), LIBAVCODEC_VERSION_INT);
	check_version("libavformat", avformat_version(), LIBAVFORMAT_VERSION_INT);
	check_version("libavutil", avutil_version(), LIBAVUTIL_VERSION_INT);
	check_version("libswresample", swresample_version(), LIBSWRESAMPLE_VERSION_INT);
	check_version("libswscale", swscale_version(), LIBSWSCALE_VERSION_INT);

#ifndef NDEBUG
	av_log_set_callback(&log_callback_report);
	av_log_set_level(AV_LOG_ERROR);
#else
	av_log_set_level(AV_LOG_QUIET);
#endif

	mprintf(("FFmpeg library initialized!\n"));
	mprintf(("FFmpeg: License: %s\n", avformat_license()));

	initialized = true;
}
}
}
