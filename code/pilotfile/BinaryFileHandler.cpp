
#include "pilotfile/BinaryFileHandler.h"

pilot::BinaryFileHandler::BinaryFileHandler(CFILE* cfp) : _cfp(cfp) {
	Assertion(cfp != nullptr, "File pointer must be valid!");
}

pilot::BinaryFileHandler::~BinaryFileHandler() {
	cfclose(_cfp);
	_cfp = nullptr;
}

void pilot::BinaryFileHandler::writeByte(const char*, std::int8_t value) {
	cfwrite_char(value, _cfp);
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

void pilot::BinaryFileHandler::beginWritingSections() {
	Assertion(!_writingSections, "Section writing nesting is not supported!");
	_writingSections = true;
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
	_sectionOffsets.pop_back();

	if (previous_off.id == Section::Unnamed) {
		// ignore unnamed sections, these are only needed for JSON
		return;
	}


	size_t cur = (size_t) cftell(_cfp);

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
void pilot::BinaryFileHandler::endWritingSections() {
	Assertion(_writingSections, "Section writing ended while not writing sections!");
	_writingSections = false;
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
std::int8_t pilot::BinaryFileHandler::readByte(const char*) {
	return cfread_char(_cfp);
}
std::uint8_t pilot::BinaryFileHandler::readUByte(const char*) {
	return cfread_ubyte(_cfp);
}
std::int16_t pilot::BinaryFileHandler::readShort(const char*) {
	return cfread_short(_cfp);
}
std::int32_t pilot::BinaryFileHandler::readInt(const char*) {
	return cfread_int(_cfp);
}
std::uint32_t pilot::BinaryFileHandler::readUInt(const char*) {
	return cfread_uint(_cfp);
}
float pilot::BinaryFileHandler::readFloat(const char*) {
	return cfread_float(_cfp);
}
SCP_string pilot::BinaryFileHandler::readString(const char*) {
	return cfread_string_len(_cfp);
}
void pilot::BinaryFileHandler::readString(const char*, char* dest, size_t max_size) {
	cfread_string_len(dest, (int) max_size, _cfp);
}
void pilot::BinaryFileHandler::beginSectionRead() {
	// Everything is handled in nextSection
}
bool pilot::BinaryFileHandler::hasMoreSections() {
	return !cfeof(_cfp);
}
Section pilot::BinaryFileHandler::nextSection() {
	Assertion(!_in_array, "nextSection() may not be called in an array!");

	if (_section_start_pos != INVALID_SIZE && _section_end_pos != INVALID_SIZE) {
		cf_set_max_read_len(_cfp, 0);

		// There was a previous section
		auto current = (size_t)cftell(_cfp);
		if (current != _section_end_pos) {
			mprintf(("PLR => WARNING: Advancing to the next section. " SIZE_T_ARG " bytes were skipped!\n", _section_end_pos - current));
			cfseek(_cfp, (int)_section_end_pos, CF_SEEK_SET);
		}

		_section_start_pos = INVALID_SIZE;
		_section_end_pos = INVALID_SIZE;
	}

	auto section_id = cfread_ushort(_cfp);
	auto size = cfread_uint(_cfp);

	if (size == 0) {
		return Section::Invalid;
	}

	_section_start_pos = (size_t)cftell(_cfp);
	_section_end_pos = _section_start_pos + size;

	cf_set_max_read_len(_cfp, size);

	return static_cast<Section>(section_id);
}
void pilot::BinaryFileHandler::endSectionRead() {
	_section_start_pos = INVALID_SIZE;
	_section_end_pos = INVALID_SIZE;
}
size_t pilot::BinaryFileHandler::startArrayRead(const char*, bool short_index) {
	Assertion(!_in_array, "Array nesting is not supported!");

	_in_array = true;
	if (short_index) {
		return (size_t) cfread_short(_cfp);
	} else {
		return (size_t) cfread_int(_cfp);
	}
}
void pilot::BinaryFileHandler::nextArraySection() {
	Assertion(_in_array, "nextArraySection() may only be called in an array!");
	// Nothing to do here in the binary version
}
void pilot::BinaryFileHandler::endArrayRead() {
	Assertion(_in_array, "Array ended while not reading array!");

	_in_array = false;
}
