//
//

#include "SubtitleDecoder.h"

namespace {

double getFrameTime(int64_t pts, AVRational time_base) {
	return pts * av_q2d(time_base);
}

}

namespace cutscene {
namespace ffmpeg {

SubtitleDecoder::SubtitleDecoder(cutscene::ffmpeg::DecoderStatus* status) : FFMPEGStreamDecoder(status) {
}
SubtitleDecoder::~SubtitleDecoder() {

}
void SubtitleDecoder::decodePacket(AVPacket* packet) {
	int finishedFrame = 0;
	AVSubtitle subtitle;
	auto result = avcodec_decode_subtitle2(m_status->subtitleCodecCtx, &subtitle, &finishedFrame, packet);

	if (result >= 0 && finishedFrame) {
		pushSubtitleFrame(packet, &subtitle);

		avsubtitle_free(&subtitle);
	}
}
void SubtitleDecoder::finishDecoding() {

}
void SubtitleDecoder::pushSubtitleFrame(AVPacket* packet, AVSubtitle* subtitle) {
	if (subtitle->format != 1) {
		// Non-text subtitles are not supported yet.
		mprintf(("FFmpeg: Detected a non-text subtitle! This is not supported yet!\n"));
		return;
	}
	if (subtitle->num_rects < 1) {
		return;
	}

	int64_t pts = subtitle->pts;
	if (pts == AV_NOPTS_VALUE) {
		pts = packet->pts;
	}
	if (pts == AV_NOPTS_VALUE) {
		// Still no valid timestamp...
		return;
	}
	auto packet_time = getFrameTime(pts, m_status->subtitleStream->time_base);

	auto start_time = packet_time + (subtitle->start_display_time / 1000.0);
	auto end_time = packet_time + (subtitle->end_display_time / 1000.0);
	if (subtitle->end_display_time == 0 && m_status->subtitleStream->time_base.num != 0) {
		end_time = packet_time + getFrameTime(packet->duration, m_status->subtitleStream->time_base);
	}

	SCP_string processed_text;
	// For now we only use the first subtitle rectangle
	auto subtitle_rect = subtitle->rects[0];
	if (subtitle_rect->type == SUBTITLE_BITMAP) {
		// Same as above, non-text subtitles are not supported yet.
		mprintf(("FFmpeg: Detected a non-text subtitle! This is not supported yet!\n"));
		return;
	} else if (subtitle_rect->type == SUBTITLE_TEXT) {
		// Subtitle does not need to be processed any further
		processed_text = subtitle_rect->text;
	} else if (subtitle_rect->type == SUBTITLE_ASS) {
		SCP_string ass_text = subtitle_rect->ass;
		
		// We are not interested in any of the other information contained in the ASS line but we still need to figure
		// out where our text starts
		auto comma_pos = ass_text.find(',');
		for (auto i = 0; i < 8 && comma_pos != SCP_string::npos; ++i) {
			comma_pos = ass_text.find(',', comma_pos + 1);
		}
		Assertion(comma_pos != SCP_string::npos, "Received an ill-formed ASS line from FFmpeg! Text was '%s'.", subtitle_rect->ass);

		// This + 1 is safe since comma_pos points to a valid character so the next character is at worst the end of the string
		processed_text = ass_text.substr(comma_pos + 1);

		// ASS has a special sequence for new lines that needs to be converted to an actual new line
		auto newline_pos = processed_text.find("\\N");
		while(newline_pos != SCP_string::npos) {
			processed_text.replace(newline_pos, 2, "\n");

			newline_pos = processed_text.find("\\N");
		}
	} else {
		mprintf(("FFmpeg: Detected unknown subtitle name in movie!\n"));
		return;
	}

	// Remove \r from string. Some subtitles use \r\n for line breaks and we only support \n
	auto ret_pos = processed_text.find('\r');
	while(ret_pos != SCP_string::npos) {
		processed_text.erase(ret_pos, 1);

		ret_pos = processed_text.find('\r');
	}

	SubtitleFramePtr frame(new SubtitleFrame());
	frame->text = processed_text;

	frame->displayStartTime = start_time;
	frame->displayEndTime = end_time;

	pushFrame(std::move(frame));
}

}
}

