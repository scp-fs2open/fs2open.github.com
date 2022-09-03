
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
	bool _writingSections = false;


	size_t _section_start_pos = INVALID_SIZE;
	size_t _section_end_pos = INVALID_SIZE;

	bool _in_array = false;
 public:
	explicit BinaryFileHandler(CFILE* cfp);

	~BinaryFileHandler() override;

	void writeByte(const char* name, std::int8_t value) override;

	void writeUByte(const char* name, std::uint8_t value) override;

	void writeShort(const char* name, std::int16_t value) override;

	void writeInt(const char* name, std::int32_t value) override;

	void writeUInt(const char* name, std::uint32_t value) override;

	void writeFloat(const char* name, float value) override;

	void writeString(const char* name, const char* str) override;


	void beginWritingSections() override;

	void startSectionWrite(Section id) override;

	void endSectionWrite() override;

	void endWritingSections() override;


	void startArrayWrite(const char* name, size_t size, bool short_length) override;

	void endArrayWrite() override;


	void flush() override;

	std::int8_t readByte(const char* name) override;

	std::uint8_t readUByte(const char* name) override;

	std::int16_t readShort(const char* name) override;

	std::int32_t readInt(const char* name) override;

	std::uint32_t readUInt(const char* name) override;

	float readFloat(const char* name) override;

	SCP_string readString(const char* name) override;

	void readString(const char* name, char* dest, size_t max_size) override;

	void beginSectionRead() override;

	bool hasMoreSections() override;

	Section nextSection() override;

	void endSectionRead() override;

	size_t startArrayRead(const char* name, bool short_index) override;

	void nextArraySection() override;

	void endArrayRead() override;
};
}

#endif // BINARY_FILE_HANDLER_H
