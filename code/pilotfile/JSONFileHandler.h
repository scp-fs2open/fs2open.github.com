
#ifndef JSON_FILE_HANDLER_H
#define JSON_FILE_HANDLER_H
#pragma once

#include "pilotfile/FileHandler.h"

#include "globalincs/pstypes.h"
#include "cfile/cfile.h"

#include <jansson.h>

namespace pilot {
class JSONFileHandler: public FileHandler {
	CFILE* _cfp;

	json_t* _rootObj;

	json_t* _currentEl;

	SCP_vector<json_t*> _elementStack;

	void ensureNotExists(const char* name);
	void writeInteger(const char* name, json_int_t value);
 public:
	explicit JSONFileHandler(CFILE* cfp);

	~JSONFileHandler() override;

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

	void flush() override;
};
}

#endif // JSON_FILE_HANDLER_H
