
#ifndef _FFMPEGAUDIOREADER_H
#define _FFMPEGAUDIOREADER_H
#pragma once

#include "globalincs/pstypes.h"

#include "libs/ffmpeg/FFmpegHeaders.h"

namespace ffmpeg
{

class FFmpegAudioReader
{
	int _stream_idx = -1;
	AVFormatContext* _format_ctx = nullptr;
	AVCodecContext* _codec_ctx = nullptr;

#if LIBAVCODEC_VERSION_INT <= AV_VERSION_INT(57, 24, 255)
    AVPacket* _currentPacket = nullptr;

	void processPendingPacket();
#endif

public:
	FFmpegAudioReader(AVFormatContext* av_format_context, AVCodecContext* codec_ctx, int stream_idx);
	~FFmpegAudioReader();

	bool readFrame(AVFrame* decode_frame);
};

}

#endif // _FFMPEGAUDIOREADER_H