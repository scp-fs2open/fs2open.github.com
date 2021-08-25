
#include "FFmpegContext.h"

#include <cstring>

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

	auto intOff = static_cast<int>(offset);
	if ((int64_t)intOff != offset) {
		// Overflow!!!
		return -1;
	}
	
	auto ret = cfseek(cfile, intOff, op);

	if (ret != 0) {
		// Error
		return -1;
	}

	// cfseek returns the offset in the archive file (who thought that would be a good idea?)
	return cftell(cfile);
}

int memfile_read(void* opaque, uint8_t* buf, int buf_size) {
    auto memsound = reinterpret_cast<libs::ffmpeg::MemFileCursor*>(opaque);
    auto vec_buf_size = static_cast<SCP_vector<uint8_t>::size_type>(buf_size);
    if ((int)vec_buf_size != buf_size) {
        // Overflow!!!
        return -1;
    }
    
    vec_buf_size = std::min(vec_buf_size, memsound->snddata.size() - memsound->cursor_pos);
    
    if (vec_buf_size > 0) {
        std::memcpy(buf, memsound->snddata.data() + memsound->cursor_pos, vec_buf_size);
        
        memsound->cursor_pos += vec_buf_size;
        
        return static_cast<int>(vec_buf_size);
    } else {
        return -1;
    }
}

int64_t memfile_seek(void* opaque, int64_t offset, int whence) {
    auto memsound = reinterpret_cast<libs::ffmpeg::MemFileCursor*>(opaque);

    if (whence == AVSEEK_SIZE) {
        return memsound->snddata.size();
    }
    int64_t cursor_pos = memsound->cursor_pos;

    switch (whence) {
        case SEEK_SET:
            cursor_pos = offset;
            break;
        case SEEK_CUR:
            cursor_pos += offset;
            break;
        case SEEK_END:
            cursor_pos = memsound->snddata.size() + offset;
            break;
    }

    memsound->cursor_pos = std::min(static_cast<SCP_vector<uint8_t>::size_type>(cursor_pos), memsound->snddata.size());

    return static_cast<int64_t>(memsound->cursor_pos);
}

}//namespace

namespace libs {
namespace ffmpeg {

MemFileCursor::MemFileCursor(const uint8_t* data, size_t snd_len) : snddata(data, data + snd_len), cursor_pos(0) {
}

MemFileCursor::MemFileCursor() : snddata() , cursor_pos(0) {}

FFmpegContext::FFmpegContext(CFILE* inFile) : m_ctx(nullptr), m_file(inFile), m_memsound() {
	Assertion(inFile != nullptr, "Invalid file pointer passed!");
}

FFmpegContext::FFmpegContext(const uint8_t* snddata, size_t snd_len) : m_ctx(nullptr), m_file(nullptr), m_memsound(snddata, snd_len) {
}

FFmpegContext::~FFmpegContext() {
	if (m_ctx) {
		if (m_ctx->pb) {
			if (m_ctx->pb->buffer) {
				av_free(m_ctx->pb->buffer);
			}
			av_free(m_ctx->pb);
			m_ctx->pb = nullptr;
		}

		avformat_close_input(&m_ctx);
		m_ctx = nullptr;
	}

	if (m_file) {
		cfclose(m_file);
		m_file = nullptr;
	}
	
}

void FFmpegContext::prepare(void *opaque,
                            int (*read_packet)(void *opaque, uint8_t *buf, int buf_size),
                            int64_t (*seek)(void *opaque, int64_t offset, int whence)) {
    m_ctx = avformat_alloc_context();

    if (!m_ctx) {
        throw FFmpegException("Failed to allocate context!");
    }

    auto avioBuffer = reinterpret_cast<uint8_t*>(av_malloc(AVIO_BUFFER_SIZE));

    if (!avioBuffer) {
        throw FFmpegException("Failed to allocate IO buffer!");
    }

    auto ioContext =
        avio_alloc_context(avioBuffer, AVIO_BUFFER_SIZE, 0, opaque, read_packet, nullptr, seek);

    if (!ioContext) {
        throw FFmpegException("Failed to allocate IO context!");
    }

    m_ctx->pb = ioContext;

    auto probe_ret = av_probe_input_buffer2(m_ctx->pb, &m_ctx->iformat, nullptr, nullptr, 0, 0);
    if (probe_ret < 0) {
        char errorStr[1024];
        av_strerror(probe_ret, errorStr, 1024);

        throw FFmpegException(SCP_string("Could not open movie file! Error: ") + errorStr);
    }

    m_ctx->flags |= AVFMT_FLAG_CUSTOM_IO;

    auto ret = avformat_open_input(&m_ctx, nullptr, m_ctx->iformat, nullptr);
    if (ret < 0) {
        char errorStr[1024];
        av_strerror(ret, errorStr, 1024);

        throw FFmpegException(SCP_string("Could not open movie file! Error: ") + errorStr);
    }

    ret = avformat_find_stream_info(m_ctx, nullptr);
    if (ret < 0) {
        char errorStr[1024];
        av_strerror(ret, errorStr, 1024);

        throw FFmpegException(SCP_string("Failed to get stream information! Error: ") + errorStr);
    }
}

std::unique_ptr<FFmpegContext> FFmpegContext::createContext(CFILE* mediaFile) {
	Assertion(mediaFile != nullptr, "File pointer must be valid!");

	std::unique_ptr<FFmpegContext> instance(new FFmpegContext(mediaFile));

    instance->prepare(instance->m_file, cfileRead, cfileSeek);
    return instance;
}


std::unique_ptr<FFmpegContext> FFmpegContext::createContextMem(const uint8_t* snddata, size_t snd_len) {
	std::unique_ptr<FFmpegContext> instance(new FFmpegContext(snddata, snd_len));

    instance->prepare(&(instance->m_memsound), memfile_read, memfile_seek);
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
