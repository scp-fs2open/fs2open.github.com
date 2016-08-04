
#include "FFmpegContext.h"

#include <boost/format.hpp>

namespace {
const size_t AVIO_BUFFER_SIZE = 8192;

int cfileRead(void* ptr, uint8_t* buf, int buf_size) {
	auto cfile = reinterpret_cast<CFILE*>(ptr);

	auto numRead = cfread(buf, 1, buf_size, cfile);

	if (numRead == 0) {
		// Read failed
		return -1;
	}

	return numRead;
}

int64_t cfileSeek(void* ptr, int64_t offset, int whence) {
	auto cfile = reinterpret_cast<CFILE*>(ptr);

	int op = CF_SEEK_SET;
	switch (whence) {
		case SEEK_SET:
			op = CF_SEEK_SET;
			break;
		case SEEK_CUR:
			op = CF_SEEK_CUR;
			break;
		case SEEK_END:
			op = CF_SEEK_END;
			break;
		case AVSEEK_SIZE:
			return cfilelength(cfile);
	}

	cfseek(cfile, static_cast<int>(offset), op);

	// cfseek returns the offset in the archive file (who thought that would be a good idea?)
	return cftell(cfile);
}
}

namespace libs {
namespace ffmpeg {

FFmpegContext::FFmpegContext(CFILE* inFile) : m_ctx(nullptr), m_file(inFile) {
	Assertion(inFile != nullptr, "Invalid file pointer passed!");
}

FFmpegContext::~FFmpegContext() {
	if (m_ctx) {
		if (m_ctx->pb) {
			if (m_ctx->pb->buffer) {
				av_free(m_ctx->pb->buffer);
			}
			av_free(m_ctx->pb);
		}

		avformat_close_input(&m_ctx);
		m_ctx = nullptr;
	}

	if (m_file) {
		cfclose(m_file);
		m_file = nullptr;
	}
}

std::unique_ptr<FFmpegContext> FFmpegContext::createContext(CFILE* mediaFile) {
	Assertion(mediaFile != nullptr, "File pointer must be valid!");

	std::unique_ptr<FFmpegContext> instance(new FFmpegContext(mediaFile));

	instance->m_ctx = avformat_alloc_context();

	if (!instance->m_ctx) {
		throw FFmpegException("Failed to allocate context!");
	}

	auto avioBuffer = reinterpret_cast<uint8_t*>(av_malloc(AVIO_BUFFER_SIZE));

	if (!avioBuffer) {
		throw FFmpegException("Failed to allocate IO buffer!");
	}

	auto ioContext =
		avio_alloc_context(avioBuffer, AVIO_BUFFER_SIZE, 0, instance->m_file, cfileRead, nullptr, cfileSeek);

	if (!ioContext) {
		throw FFmpegException("Failed to allocate IO context!");
	}

	instance->m_ctx->pb = ioContext;

	auto probe_ret = av_probe_input_buffer2(instance->m_ctx->pb, &instance->m_ctx->iformat, nullptr, nullptr, 0, 0);
	if (probe_ret < 0) {
		char errorStr[1024];
		av_strerror(probe_ret, errorStr, 1024);
		auto fmt = boost::format("Could not open movie file! Error: %s") % errorStr;

		throw FFmpegException(fmt.str());
	}

	instance->m_ctx->flags |= AVFMT_FLAG_CUSTOM_IO;

	auto ret = avformat_open_input(&instance->m_ctx, nullptr, instance->m_ctx->iformat, nullptr);
	if (ret < 0) {
		char errorStr[1024];
		av_strerror(ret, errorStr, 1024);
		auto fmt = boost::format("Could not open movie file! Error: %s") % errorStr;

		throw FFmpegException(fmt.str());
	}

	ret = avformat_find_stream_info(instance->m_ctx, nullptr);
	if (ret < 0) {
		char errorStr[1024];
		av_strerror(ret, errorStr, 1024);
		auto fmt = boost::format("Failed to get stream information! Error: %s") % errorStr;

		throw FFmpegException(fmt.str());
	}

	return instance;
}

std::unique_ptr<FFmpegContext> FFmpegContext::createContext(const SCP_string& path, int dir_type) {
	CFILE* file = cfopen(path.c_str(), "rb", CFILE_NORMAL, dir_type);

	if (!file) {
		throw FFmpegException("Failed to open file!");
	}

	return createContext(file);
}

}
}
