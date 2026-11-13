#include "mission/missionjournal.h"

#include "cmdline/cmdline.h"
#include "iff_defs/iff_defs.h"
#include "libs/jansson.h"
#include "mission/missionparse.h"
#include "network/multi.h"
#include "osapi/osapi.h"
#include "parse/parselo.h"

#include <cstdio>
#include <ctime>

#if defined(_WIN32)
#include <direct.h>
#endif

// defined in freespace.cpp
extern char Game_current_mission_filename[];

namespace {

SCP_string utc_time(const char* format)
{
	const std::time_t now = std::time(nullptr);
	const std::tm* utc = std::gmtime(&now);
	char buffer[64];
	if (utc == nullptr || std::strftime(buffer, sizeof(buffer), format, utc) == 0) {
		return "";
	}
	return buffer;
}

// one file per game launch, named when the first event is written; a numbered suffix keeps another engine
// sharing the folder from appending to the same file
const SCP_string& journal_path()
{
	static SCP_string path;
	if (path.empty()) {
		// fails harmlessly when the folder exists; write_line reports a folder that can't be written
		_mkdir(os_get_config_path("journal").c_str());
		const SCP_string base = "journal/Journal." + utc_time("%Y-%m-%dT%H%M%S");
		for (int n = 1; n <= 9 && path.empty(); ++n) {
			const SCP_string candidate = os_get_config_path(base + (n > 1 ? "-" + std::to_string(n) : "") + ".log");
			FILE* existing = fopen(candidate.c_str(), "r");
			if (existing == nullptr) {
				path = candidate;
			} else {
				fclose(existing);
			}
		}
		if (path.empty()) {
			path = os_get_config_path(base + ".log");
		}
	}
	return path;
}

bool journal_wanted()
{
	if (MULTIPLAYER_CLIENT) {
		return false;
	}
	return Cmdline_event_journal;
}

// whether the current mission's MissionStart has been written, so MissionEnd only follows a start
bool Journal_mission_open = false;

// opened and closed per event, so a tool reading the file sees each line as soon as it is written
void write_line(const SCP_string& line)
{
	static bool warned = false;
	FILE* fp = fopen(journal_path().c_str(), "a");
	bool written = false;
	if (fp != nullptr) {
		written = fputs(line.c_str(), fp) >= 0 && fputc('\n', fp) != EOF;
		// fclose writes out the buffered line, so a full disk usually shows up here; it runs even if a write failed
		written = fclose(fp) == 0 && written;
	}
	if (!written && !warned) {
		ReleaseWarning(LOCATION, "Event journal: cannot write to \"%s\".", journal_path().c_str());
		warned = true;
	}
}

// mission and ship names are not always UTF-8, which JSON requires
json_t* utf8_string(const char* text)
{
	SCP_string buffer;
	coerce_to_utf8(buffer, text);
	return json_string(buffer.c_str());
}

json_t* new_event(const char* event, const char* timestamp)
{
	json_t* obj = json_object();
	json_object_set_new(obj, "timestamp", json_string(timestamp));
	json_object_set_new(obj, "event", json_string(event));
	return obj;
}

void set_team(json_t* obj, const char* key, int team)
{
	if (SCP_vector_inbounds(Iff_info, team)) {
		json_object_set_new(obj, key, utf8_string(Iff_info[team].iff_name));
	}
}

} // namespace

SCP_string mission_journal_format_entry(const log_entry& entry, const char* mission_name, const char* timestamp)
{
	json_t* obj = new_event(mission_log_type_name(entry.type), timestamp);
	json_object_set_new(obj, "Mission", utf8_string(mission_name));
	json_object_set_new(obj, "MissionTime", json_real(f2fl(entry.timestamp)));
	json_object_set_new(obj, "Primary", utf8_string(entry.pname));
	set_team(obj, "PrimaryTeam", entry.primary_team);
	json_object_set_new(obj, "Secondary", utf8_string(entry.sname));
	set_team(obj, "SecondaryTeam", entry.secondary_team);
	json_object_set_new(obj, "Index", json_integer(entry.index));
	json_object_set_new(obj, "Hidden", json_boolean((entry.flags & MLF_HIDDEN) != 0));
	return json_dump_string_new(obj, JSON_COMPACT | JSON_PRESERVE_ORDER | JSON_REAL_PRECISION(7));
}

void mission_journal_mission_start()
{
	if (!journal_wanted() || Journal_mission_open) {
		return;
	}
	Journal_mission_open = true;
	json_t* obj = new_event("MissionStart", utc_time("%Y-%m-%dT%H:%M:%SZ").c_str());
	json_object_set_new(obj, "Mission", utf8_string(Game_current_mission_filename));
	json_object_set_new(obj, "Title", utf8_string(The_mission.name.c_str()));
	write_line(json_dump_string_new(obj, JSON_COMPACT | JSON_PRESERVE_ORDER | JSON_REAL_PRECISION(7)));
}

void mission_journal_log_entry(const log_entry& entry)
{
	if (!journal_wanted()) {
		return;
	}
	// entries can be logged while the mission loads, before the post-load start call
	mission_journal_mission_start();
	write_line(
		mission_journal_format_entry(entry, Game_current_mission_filename, utc_time("%Y-%m-%dT%H:%M:%SZ").c_str()));
}

void mission_journal_mission_end()
{
	if (!journal_wanted() || !Journal_mission_open) {
		return;
	}
	Journal_mission_open = false;
	json_t* obj = new_event("MissionEnd", utc_time("%Y-%m-%dT%H:%M:%SZ").c_str());
	json_object_set_new(obj, "Mission", utf8_string(Game_current_mission_filename));
	json_object_set_new(obj, "MissionTime", json_real(f2fl(Missiontime)));
	write_line(json_dump_string_new(obj, JSON_COMPACT | JSON_PRESERVE_ORDER | JSON_REAL_PRECISION(7)));
}
