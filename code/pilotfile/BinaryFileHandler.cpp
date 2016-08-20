
#include "pilotfile/BinaryFileHandler.h"

pilot::BinaryFileHandler::BinaryFileHandler(CFILE* cfp) : _cfp(cfp) {
	Assertion(cfp != nullptr, "File pointer must be valid!");
}

pilot::BinaryFileHandler::~BinaryFileHandler() {
	cfclose(_cfp);
	_cfp = nullptr;
}

void pilot::BinaryFileHandler::writeUByte(const char*, std::uint8_t value) {
	cfwrite_ubyte(value, _cfp);
}
void pilot::BinaryFileHandler::writeShort(const char*, std::int16_t value) {
	cfwrite_short(value, _cfp);
}

void pilot::BinaryFileHandler::writeInt(const char*, std::int32_t value) {
	cfwrite_int(value, _cfp);
}

void pilot::BinaryFileHandler::writeUInt(const char*, std::uint32_t value) {
	cfwrite_uint(value, _cfp);
}

void pilot::BinaryFileHandler::writeFloat(const char*, float value) {
	cfwrite_float(value, _cfp);
}

void pilot::BinaryFileHandler::writeString(const char*, const char* str) {
	cfwrite_string_len(str, _cfp);
}

void pilot::BinaryFileHandler::startSectionWrite(Section id) {
	if (id == Section::Unnamed) {
		_sectionOffsets.push_back({id, 0});
		return;
	}

	cfwrite_ushort((ushort) id, _cfp);

	// to be updated when endSection() is called
	cfwrite_int(0, _cfp);

	// starting offset, for size of section
	_sectionOffsets.push_back({id, (size_t)cftell(_cfp)});
}

void pilot::BinaryFileHandler::endSectionWrite() {
	Assertion(!_sectionOffsets.empty(), "No active section!");
	auto previous_off = _sectionOffsets.back();

	if (previous_off.id == Section::Unnamed) {
		// ignore unnamed sections, these are only needed for JSON
		return;
	}


	size_t cur = (size_t) cftell(_cfp);
	_sectionOffsets.pop_back();

	Assert(cur >= previous_off.offset);

	size_t section_size = cur - previous_off.offset;

	if (section_size) {
		// go back to section size in file and write proper value
		cfseek(_cfp, (int) (cur - section_size - sizeof(int)), CF_SEEK_SET);
		cfwrite_int((int) section_size, _cfp);

		// go back to previous location for next section
		cfseek(_cfp, (int) cur, CF_SEEK_SET);
	}
}
void pilot::BinaryFileHandler::startArrayWrite(const char*, size_t size, bool short_index) {
	if (short_index) {
		cfwrite_ushort((short)size, _cfp);
	} else {
		cfwrite_int((int)size, _cfp);
	}
}
void pilot::BinaryFileHandler::endArrayWrite() {
	// Nothing to do here for the binary version
}
void pilot::BinaryFileHandler::flush() {
	cflush(_cfp);
}
