
#ifndef JSON_FILE_HANDLER_H
#define JSON_FILE_HANDLER_H
#pragma once

#include "pilotfile/FileHandler.h"

#include "globalincs/pstypes.h"
#include "cfile/cfile.h"

#include <jansson.h>

namespace pilot {
class JSONFileHandler: public FileHandler {
	CFILE* _cfp = nullptr;

	json_t* _rootObj = nullptr;

	json_t* _currentEl = nullptr;

	SCP_vector<json_t*> _elementStack;
	void pushElement(json_t* el);
	void popElement();

	void* _sectionIterator = nullptr;
	bool _startingSectionIteration = false;
	size_t _arrayIndex = INVALID_SIZE;

	void ensureNotExists(const char* name);
	void writeInteger(const char* name, json_int_t value);

	json_int_t readInteger(const char* name);
	void ensureExists(const char* name);

	void nextArraySection(bool in_section);
 public:
	JSONFileHandler(CFILE* cfp, bool reading);

	~JSONFileHandler() override;

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


	void startArrayWrite(const char* name, size_t size, bool short_length = false) override;

	void endArrayWrite() override;

	void flush() override;

	std::int8_t readByte(const char* name) override;

	std::uint8_t readUByte(const char* name) override;

	std::int16_t readShort(const char* name) override;

	std::int32_t readInt(const char* name) override;

	std::uint32_t readUInt(const char* name) override;

	float readFloat(const char* name) override;

	SCP_string readString(const char* name) override;

	void beginSectionRead() override;

	bool hasMoreSections() override;

	Section nextSection() override;

	void endSectionRead() override;

	size_t startArrayRead(const char* name, bool short_index) override;

	void nextArraySection() override;

	void endArrayRead() override;
};
}

#endif // JSON_FILE_HANDLER_H
