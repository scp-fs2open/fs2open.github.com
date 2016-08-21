
#ifndef BINARY_FILE_HANDLER_H
#define BINARY_FILE_HANDLER_H
#pragma once

#include "pilotfile/FileHandler.h"

#include "globalincs/pstypes.h"
#include "cfile/cfile.h"

namespace pilot {
class BinaryFileHandler: public FileHandler {
	CFILE* _cfp;

	struct SectionOffset {
		Section id;
		size_t offset;
	};

	SCP_vector<SectionOffset> _sectionOffsets;
 public:
	explicit BinaryFileHandler(CFILE* cfp);

	~BinaryFileHandler() override;

	void writeUByte(const char* name, std::uint8_t value) override;

	void writeShort(const char* name, std::int16_t value) override;

	void writeInt(const char* name, std::int32_t value) override;

	void writeUInt(const char* name, std::uint32_t value) override;

	void writeFloat(const char* name, float value) override;

	void writeString(const char* name, const char* str) override;


	void startSectionWrite(Section id) override;

	void endSectionWrite() override;


	void startArrayWrite(const char* name, size_t size, bool short_length = false) override;

	void endArrayWrite() override;


	virtual void flush() override;
};
}

#endif // BINARY_FILE_HANDLER_H
