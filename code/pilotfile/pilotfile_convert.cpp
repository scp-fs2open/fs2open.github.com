
#include "pilotfile/pilotfile_convert.h"
#include "playerman/managepilot.h"
#include "playerman/player.h"


pilotfile_convert::pilotfile_convert()
{
	fver = 0;

	m_size_offset = 0;

	cfp = NULL;
	plr = NULL;
	csg = NULL;
}

pilotfile_convert::~pilotfile_convert()
{
	if (cfp) {
		cfclose(cfp);
	}

	if (plr) {
		delete plr;
	}

	if (csg) {
		delete csg;
	}
}

void pilotfile_convert::startSection(Section::id section_id)
{
	Assert( cfp );

	const int zero = 0;

	cfwrite_ushort( (ushort)section_id, cfp );

	// to be updated when endSection() is called
	cfwrite_int(zero, cfp);

	// starting offset, for size of section
	m_size_offset = cftell(cfp);
}

void pilotfile_convert::endSection()
{
	Assert( cfp );
	Assert( m_size_offset > 0 );

	size_t cur = cftell(cfp);

	Assert( cur >= m_size_offset );

	size_t section_size = cur - m_size_offset;

	if (section_size) {
		// go back to section size in file and write proper value
		cfseek(cfp, (int)(cur - section_size - sizeof(int)), CF_SEEK_SET);
		cfwrite_int((int)section_size, cfp);

		// go back to previous location for next section
		cfseek(cfp, (int)cur, CF_SEEK_SET);
	}
}

/**
 * @brief Converts retail pilots to the "new" binary style
 */
static void convert_retail_pilot_files()
{
	size_t idx, j, i;
	size_t count, inf_count;
	int max_convert, num_converted = 0;
	SCP_vector<SCP_string> existing;
	SCP_vector<SCP_string> old_files;

	// get list of pilots which already exist (new or converted already)
	cf_get_file_list(existing, CF_TYPE_PLAYERS, "*.plr", CF_SORT_NONE, nullptr,
	                 CF_LOCATION_ROOT_USER | CF_LOCATION_ROOT_GAME | CF_LOCATION_TYPE_ROOT);

	// get list of old pilots which may need converting, starting with inferno pilots
	Get_file_list_child = "inferno";
	cf_get_file_list(old_files, CF_TYPE_SINGLE_PLAYERS, "*.pl2", CF_SORT_NONE, nullptr,
	                 CF_LOCATION_ROOT_USER | CF_LOCATION_ROOT_GAME | CF_LOCATION_TYPE_ROOT);
	inf_count = old_files.size();
	cf_get_file_list(old_files, CF_TYPE_SINGLE_PLAYERS, "*.pl2", CF_SORT_NONE, nullptr,
	                 CF_LOCATION_ROOT_USER | CF_LOCATION_ROOT_GAME | CF_LOCATION_TYPE_ROOT);

	if ( old_files.empty() ) {
		return;
	}

	// rip out all files which already exist
	i = 0;
	count = old_files.size();
	for (idx = 0; idx < existing.size(); idx++) {
		const char *fname = existing[idx].c_str();

		for (j = 0; j < count; j++) {
			if ( !stricmp(fname, old_files[j].c_str()) ) {
				// NOTE: we just clear the name here to avoid the fragmentation
				//       from resizing the vector
				old_files[j] = "";
				++i;
			}
		}
	}

	// don't convert enough pilots to exceed the pilot limit
	max_convert = MAX_PILOTS - (int)existing.size();

	// if everything is already converted then bail
	// also bail if MAX_PILOTS (or more!) already exist
	if (i == count || max_convert <= 0) {
		return;
	}

	mprintf(("PILOT: Beginning pilot file conversion...\n"));

	// now proceed to convert all of the old files
	count = old_files.size();
	for (idx = 0; idx < count; idx++) {
		if ( old_files[idx].empty() ) {
			continue;
		}

		pilotfile_convert *pcon = new pilotfile_convert;

		bool inferno = (idx < inf_count);

		if ( pcon->plr_convert(old_files[idx].c_str(), inferno) ) {
			SCP_vector<SCP_string> savefiles;

			SCP_string wildcard(old_files[idx]);
			wildcard.append("*.cs2");

			if (inferno) {
				Get_file_list_child = "inferno";
			}

			cf_get_file_list(savefiles, CF_TYPE_SINGLE_PLAYERS, wildcard.c_str(), CF_SORT_NONE, nullptr,
			                 CF_LOCATION_ROOT_USER | CF_LOCATION_ROOT_GAME | CF_LOCATION_TYPE_ROOT);

			for (j = 0; j < savefiles.size(); j++) {
				pcon->csg_convert(savefiles[j].c_str(), inferno);
			}

			++num_converted;
		}

		delete pcon;

		if (num_converted >= max_convert) {
			break;
		}
	}

	mprintf(("PILOT: Pilot file conversion complete!\n"));
}

/**
 * @brief Converts the binary pilots into the JSON representation
 *
 * This only converts the main pilot file to JSON at the moment to make debugging easier if something is wrong
 */
static void convert_binary_pilot_files() {
	SCP_vector<SCP_string> binary_pilots;
	SCP_vector<SCP_string> json_pilots;

	// get list of binary pilot files
	cf_get_file_list(binary_pilots, CF_TYPE_PLAYERS, "*.plr");

	// get list of existing json pilot files
	cf_get_file_list(binary_pilots, CF_TYPE_PLAYERS, "*.json");

	for (auto binary_it = binary_pilots.begin(); binary_it != binary_pilots.end(); ++binary_it) {
		for (auto json_it = json_pilots.begin(); json_it != json_pilots.end(); ++json_it) {
			// The file extensions are already removed so we can just compare the iterator values
			if (*binary_it == *json_it) {
				// pilot was already converted. We just set this to the empty string to remove it from the list
				*binary_it = "";
			}
		}
	}

	// binary_pilots now contains all unconverted pilots
	for (auto& callsign : binary_pilots) {
		if (callsign.empty()) {
			// This pilot already has a json pilot
			continue;
		}

		// This is simple, read the binary pilot and then write it again as JSON
		pilotfile file;
		player plr; // Player struct to save the loaded values in
		plr.reset();

		if (!file.load_player(callsign.c_str(), &plr, true)) {
			mprintf(("Failed to load binary pilot '%s'!\n", callsign.c_str()));
			continue;
		}

		if (!file.save_player(&plr)) {
			mprintf(("Failed to save JSON pilot '%s'!\n", callsign.c_str()));
			continue;
		}
		mprintf(("Pilot '%s' was successfully converted to JSON.\n", callsign.c_str()));
	}
}

void convert_pilot_files() {
	convert_retail_pilot_files();

	convert_binary_pilot_files();
}
