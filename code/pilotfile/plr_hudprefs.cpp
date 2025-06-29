#include "plr_hudprefs.h"

#include "globalincs/pstypes.h"

#include "cfile/cfile.h"
#include "hud/hudconfig.h"
#include <parse/parselo.h>

// Saves the player's current HUD preferences (visibility, popup, color for all gauges)
// to a specific file (<callsign>.hdp) in the player directory.
void hud_config_save_player_prefs(const char* callsign)
{
	if (callsign == nullptr || *callsign == '\0') {
		mprintf(("HUDPREFS: Cannot save preferences - invalid callsign provided.\n"));
		return;
	}

	SCP_string filename = callsign;
	filename += ".hdp";

	CFILE* file = cfopen(filename.c_str(), "wt", CF_TYPE_DATA);

	if (file == nullptr) {
		mprintf(("HUDPREFS: Unable to open file '%s' for writing player HUD preferences.\n", filename.c_str()));
		return;
	}

	mprintf(("HUDPREFS: Saving player HUD preferences to '%s'...\n", filename.c_str()));

	char format_buffer[256];

	cfputs("+Version: 1\n\n", file);

	// Iterate ALL gauges
	for (const auto& pair : HUD_config.gauge_colors) {
		const SCP_string& gauge_id = pair.first;

		if (gauge_id.empty()) {
			continue;
		}

		// Get current state
		bool is_visible = HUD_config.is_gauge_visible(gauge_id);
		bool is_popup = HUD_config.is_gauge_popup(gauge_id);

		// Get color to save. If gauge is hidden in configure screen, then player cannot set it,
		// so look up the parent gauge type and save/set it. --wookeejedi
		color clr;
		if (HUD_config.is_gauge_shown_in_config(gauge_id)) {
			clr = pair.second;
		} else {
			clr = HUD_config.get_gauge_color(gauge_id, false);
			HUD_config.set_gauge_color(gauge_id, clr);
		}

		// +Gauge: <id>
		sprintf(format_buffer, "+Gauge: %s\n", gauge_id.c_str());
		cfputs(format_buffer, file);

		// +Visible: <0_or_1>
		sprintf(format_buffer, "+Visible: %d\n", is_visible ? 1 : 0);
		cfputs(format_buffer, file);

		// +Popup: <0_or_1>
		sprintf(format_buffer, "+Popup: %d\n", is_popup ? 1 : 0);
		cfputs(format_buffer, file);

		// +Color: <r> <g> <b> <a>
		sprintf(format_buffer, "+Color: %d %d %d %d\n\n", clr.red, clr.green, clr.blue, clr.alpha);
		cfputs(format_buffer, file);
	}

	cfclose(file);

	mprintf(("HUDPREFS: Finished saving player HUD preferences.\n"));
}

// Loads the player's HUD preferences (visibility, popup, color for all gauges)
// from the specific file (<callsign>.hdp) in the player directory, if it exists.
void hud_config_load_player_prefs(const char* callsign)
{
	if (callsign == nullptr || *callsign == '\0') {
		mprintf(("HUDPREFS: Cannot load preferences - invalid callsign provided.\n"));
		return;
	}

	// Construct the filename: <callsign>.hdp
	SCP_string filename = callsign;
	filename += ".hdp";

	// Check if the file exists in the player directories
	int file_location = cf_exists(filename.c_str(), CF_TYPE_DATA);

	if (file_location == -1) {
		// File doesn't exist, which is fine - means no custom prefs saved yet.
		mprintf(("HUDPREFS: No player HUD preferences file found ('%s'). Using defaults/player file settings.\n", filename.c_str()));
		return;
	}

	mprintf(("HUDPREFS: Loading player HUD preferences from '%s'...\n", filename.c_str()));

	try {

		read_file_text(filename.c_str());
		reset_parse();

		if (optional_string("+Version:")) {
			int version = 1;
			stuff_int(&version);
			// Can add version checks here later if format changes
			if (version != 1) {
				mprintf(("HUDPREFS: Warning - Unknown version %d in '%s'. Trying to parse anyway.\n",
					version,
					filename.c_str()));
			}
		}

		// Loop through all gauge blocks in the file
		while (optional_string("+Gauge:")) {
			SCP_string gauge_id;
			int visible_int = 0;
			int popup_int = 0;
			ubyte r = 0, g = 0, b = 0, a = 255; // Default alpha to opaque

			// Parse gauge ID
			stuff_string(gauge_id, F_NAME);

			// Parse visibility
			required_string("+Visible:");
			stuff_int(&visible_int);
			bool is_visible = (visible_int != 0);

			// Parse popup
			required_string("+Popup:");
			stuff_int(&popup_int);
			bool is_popup = (popup_int != 0);

			// Parse color
			required_string("+Color:");
			stuff_ubyte(&r);
			stuff_ubyte(&g);
			stuff_ubyte(&b);
			stuff_ubyte(&a);

			// Validate gauge ID before applying settings
			if (!gauge_id.empty()) {
				color clr;
				gr_init_alphacolor(&clr, r, g, b, a);

				HUD_config.set_gauge_visibility(gauge_id, is_visible);
				HUD_config.set_gauge_popup(gauge_id, is_popup);
				HUD_config.set_gauge_color(gauge_id, clr);
			} else {
				mprintf(("HUDPREFS: Warning - Encountered empty gauge ID while parsing '%s'. Skipping entry.\n",filename.c_str()));
			}
		}

		mprintf(("HUDPREFS: Finished loading player HUD preferences.\n"));

	} catch (const parse::ParseException& e) {
		mprintf(("HUDPREFS: Error parsing player HUD preferences file '%s'! Error: %s\n", filename.c_str(), e.what()));
	} catch (...) {
		mprintf(("HUDPREFS: Unknown error parsing player HUD preferences file '%s'!\n", filename.c_str()));
	}
}
