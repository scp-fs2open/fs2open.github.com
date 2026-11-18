#include "checkpoint/store.h"

#include "cfile/cfile.h"
#include "freespace.h"
#include "playerman/player.h"

namespace checkpoint {

namespace {

constexpr uint32_t FILE_MAGIC = 0x4b435346; // "FSCK"
constexpr uint32_t FILE_VERSION = 1;
constexpr uint32_t LOCATIONS = CF_LOCATION_ROOT_USER | CF_LOCATION_ROOT_GAME | CF_LOCATION_TYPE_ROOT;

using Slots = SCP_map<int, Buffer>;

SCP_string file_name(const char* mission)
{
	SCP_string name = (mission != nullptr) ? mission : Game_current_mission_filename;
	auto dot = name.rfind('.');
	if (dot != SCP_string::npos)
		name.resize(dot);
	SCP_tolower(name);
	return SCP_string(Player != nullptr ? Player->callsign : "") + "." + name + ".chk";
}

// Reads all slots of a file.  A missing or damaged file has none.
Slots read_file(const char* mission = nullptr)
{
	Slots slots;
	auto name = file_name(mission);
	auto fp = cfopen(name.c_str(), "rb", CF_TYPE_PLAYERS, false, LOCATIONS);
	if (fp == nullptr)
		return slots;

	Buffer data(static_cast<size_t>(cfilelength(fp)));
	bool complete = data.empty() || cfread(data.data(), 1, static_cast<int>(data.size()), fp) == static_cast<int>(data.size());
	cfclose(fp);
	if (!complete)
		return slots;

	Reader r(data);
	if (r.u32() != FILE_MAGIC || r.u32() != FILE_VERSION) {
		mprintf(("Checkpoint: %s is not a checkpoint file of this version; ignoring it\n", name.c_str()));
		return slots;
	}
	auto count = r.u32();
	for (uint32_t i = 0; i < count && !r.failed(); ++i) {
		int32_t index = 0;
		r.value(index);
		auto block = r.block();
		if (!r.failed())
			slots[index].assign(block.position(), block.position() + block.remaining());
	}
	if (r.failed()) {
		mprintf(("Checkpoint: %s is damaged; ignoring it\n", name.c_str()));
		slots.clear();
	}
	return slots;
}

// Writes the slots to a new file first and replaces the old one afterwards, so a failed write keeps the old file.
bool write_file(const Slots& slots)
{
	auto name = file_name(nullptr);
	if (slots.empty()) {
		cf_delete(name.c_str(), CF_TYPE_PLAYERS, LOCATIONS);
		return true;
	}

	Buffer data;
	Writer w(data);
	w.u32(FILE_MAGIC);
	w.u32(FILE_VERSION);
	w.u32(static_cast<uint32_t>(slots.size()));
	for (const auto& slot : slots) {
		w.value(static_cast<int32_t>(slot.first));
		auto at = w.begin_block();
		w.bytes(slot.second.data(), slot.second.size());
		w.end_block(at);
	}

	auto temp = name + ".tmp";
	auto fp = cfopen(temp.c_str(), "wb", CF_TYPE_PLAYERS, false, LOCATIONS);
	if (fp == nullptr)
		return false;
	bool written = cfwrite(data.data(), 1, static_cast<int>(data.size()), fp) == static_cast<int>(data.size());
	cfclose(fp);
	if (!written) {
		cf_delete(temp.c_str(), CF_TYPE_PLAYERS, LOCATIONS);
		return false;
	}

	// renaming doesn't replace an existing file everywhere
	cf_delete(name.c_str(), CF_TYPE_PLAYERS, LOCATIONS);
	return cf_rename(temp.c_str(), name.c_str(), CF_TYPE_PLAYERS) == 0;
}

} // namespace

bool store_save(int index, const Buffer& buffer)
{
	auto slots = read_file();
	slots[index] = buffer;
	bool written = write_file(slots);
	if (!written)
		mprintf(("Checkpoint: could not write %s\n", file_name(nullptr).c_str()));
	return written;
}

bool store_load(int index, Buffer& buffer)
{
	auto slots = read_file();
	auto it = slots.find(index);
	if (it == slots.end())
		return false;
	buffer = std::move(it->second);
	return true;
}

bool store_has(int index, const char* mission)
{
	auto slots = read_file(mission);
	return (index < 0) ? !slots.empty() : slots.count(index) > 0;
}

int store_next_free_index()
{
	auto slots = read_file();
	int index = 1;
	while (slots.count(index) > 0)
		++index;
	return index;
}

void store_clear(int index)
{
	if (index < 0) {
		write_file({});
		return;
	}
	auto slots = read_file();
	if (slots.erase(index) > 0)
		write_file(slots);
}

void store_delete_pilot_files(const char* callsign)
{
	SCP_vector<SCP_string> names;
	SCP_string spec = SCP_string(callsign) + ".*.chk";
	// cf_get_file_list isn't reentrant, so a filter set by the caller is put aside
	auto filter = Get_file_list_filter;
	Get_file_list_filter = nullptr;
	cf_get_file_list(names, CF_TYPE_PLAYERS, spec.c_str(), CF_SORT_NONE, nullptr, LOCATIONS);
	Get_file_list_filter = filter;
	for (const auto& name : names)
		cf_delete((name + ".chk").c_str(), CF_TYPE_PLAYERS, LOCATIONS);
}

} // namespace checkpoint
