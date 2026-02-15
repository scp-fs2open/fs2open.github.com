#pragma once

#include "ai/ai.h"
#include "cfile/cfile.h"
#include "mission/missionparse.h"
#include "object/waypoint.h"
#include "parse/parselo.h"
#include "ship/ship.h"
#include "ship/shipfx.h"

#define MISSION_BACKUP_NAME "Backup"
inline constexpr int MISSION_BACKUP_DEPTH = 9; // TODO make user configurable in QtFRED's future settings menu

struct sexp_container;

enum class MissionFormat {
	RETAIL = 0,
	STANDARD = 1,
	COMPATIBILITY_MODE = 2
};

struct FredSaveConfig {

	vec3d view_pos{};
	matrix view_orient{};

	bool always_save_display_names = false;

	// These are a little strange since mission saving and campaign saving use the same class here
	// which may be worth splitting up in the future. For now these will assert if not set when saving
	// a mission but are ignored when saving a campaign.
	const char (*fred_alt_names)[NAME_LENGTH + 1] = nullptr;
	const char (*fred_callsigns)[NAME_LENGTH + 1] = nullptr;

	MissionFormat save_format = MissionFormat::STANDARD;

	int mission_backup_depth = MISSION_BACKUP_DEPTH; // TODO make user configurable
	SCP_string mission_backup_name = MISSION_BACKUP_NAME; // TODO make user configurable
};

/**
 * @class Fred_mission_save
 *
 * @brief Class which saves a mission file
 *
 * @details Most of the private save functions are an API which are used similarly to parselo.  Many operate on
 *   sections of the save file, and a few are general purpose (the later take in arguments).
 */
class Fred_mission_save {
  public:
	Fred_mission_save() = default;

	void set_save_format(MissionFormat fmt) { save_config.save_format = fmt; }
	void set_view_pos(const vec3d& pos) { save_config.view_pos = pos; }
	void set_view_orient(const matrix& orient) { save_config.view_orient = orient; }
	void set_fred_alt_names(const char (*names)[NAME_LENGTH + 1]) { save_config.fred_alt_names = names; }
	void set_fred_callsigns(const char (*callsigns)[NAME_LENGTH + 1]) { save_config.fred_callsigns = callsigns; }
	void set_always_save_display_names(bool always) { save_config.always_save_display_names = always; }
	void set_mission_backup_depth(int depth) { save_config.mission_backup_depth = depth; }
	void set_mission_backup_name(const SCP_string& name) { save_config.mission_backup_name = name; }


	/**
	 * @brief Saves the mission onto the undo stack
	 *
	 * @param[in] pathname The full pathname
	 *
	 * @details Returns the value of CFred_mission_save::err, which is:
	 *
	 * @returns 0 for no error, or
	 * @returns A negative value if an error occured.
	 *
	 * @see save_mission_internal()
	 */
	int autosave_mission_file(char* pathname);

	/**
	 * @brief Saves the mission file to the given full pathname
	 *
	 * @param[in] pathname The full pathname to save to
	 *
	 * @details Returns the value of CFred_mission_save::err, which is:
	 *
	 * @returns 0 for no error, or
	 * @returns A negative value if an error occurred
	 *
	 * @see save_mission_internal()
	 */
	int save_mission_file(const char* pathname);

  protected:

	FredSaveConfig save_config{};
	char* raw_ptr = nullptr;
	SCP_vector<SCP_string> fso_ver_comment;
	int err = 0;
	CFILE* fp = nullptr;

	/**
	 * @brief Move past the comment without copying it to the output file. Used for special FSO comment tags
	 * @author Goober5000
	 *
	 * @param comment The comment to skip
	 * @param end     String that marks the end of our editable area.
	 *
	 * @details Searches for the comment to bypass. If found, and is before the end marker, this function will delete
	 *  the comment from the internal buffer
	 */
	void bypass_comment(const char* comment, const char* end = NULL);

	/**
	 * @brief Pops a comment off of the fso_ver_comment stack, deleting it
	 *
	 * @param[in] pop_all If true, clears the entire stack
	 */
	void fso_comment_pop(bool pop_all = false);

	/**
	 * @brief Pushes an FSO version comment
	 *
	 * @brief ver The version string to push onto the stack
	 *
	 * @see scan_fso_version_string()
	 */
	void fso_comment_push(const char* ver);

	/**
	 * @brief Saves comments from previous campaign/mission file
	 *
	 * @param[in] newlines Number of padding lines to add to the comment. If negative, inserts a tab before the fso
	 *  version string
	 */
	void parse_comments(int newlines = 1);

	/**
	 * @brief Puts the given string into the file
	 *
	 * @see printf() for formatting
	 */
	int fout(const char* format, ...);

	/**
	 * @brief Puts the given string as an XSTR() into the file
	 *
	 * @param[in] pre_str String to XSTR()
	 * @param[in] format
	 * @param[in] ...
	 *
	 * @TODO verify
	 */
	int fout_ext(const char* pre_str, const char* format, ...);

	/**
	 * @brief Puts the given version string into the file
	 *
	 * @param[in] format
	 * @param[in] ...
	 */
	int fout_version(const char* format, ...);

  private:

	/**
	 * @brief Saves the ai_goals to file
	 *
	 * @param[in] goalp
	 * @param[in] ship
	 */
	void save_ai_goals(ai_goal* goalp, int ship);

	/**
	 * @brief Saves the skybox bitmaps
	 *
	 * @details Returns the value of CFred_mission_save::err, which is:
	 *
	 * @returns 0 for no error, or
	 * @returns A negative value if an error occured.
	 *
	 * @see save_mission_internal()
	 */
	int save_bitmaps();

	/**
	 * @brief Save the reinforcements to file
	 *
	 * @details Returns the value of CFred_mission_save::err, which is:
	 *
	 * @returns 0 for no error, or
	 * @returns A negative value if an error occurred
	 */
	int save_reinforcements();

	/**
	 * @brief Save the info for the given turret to file
	 *
	 * @param[in] ptr  Subsystem (turret) to save
	 * @param[in] ship Index of the parent ship
	 *
	 * @TODO This makes a number of assumptions, needs a few debug asserts
	 */
	void save_turret_info(ship_subsys* ptr, int ship);

	/**
	 * @brief Converts $escaped tags into their retail equivalent
	 * @author Goober5000
	 */
	void convert_special_tags_to_retail() const;

	/**
	 * @brief Converts $escaped tags in the given cstring
	 *
	 * @param[in,out] text    Text to check for tags
	 * @param[in] max_len size of text
	 */
	static void convert_special_tags_to_retail(char* text, int max_len);

	/**
	 * @brief Converts $escaped tags in the given SCP_string
	 *
	 * @param[in,out] text Text to check for tags
	 */
	static void convert_special_tags_to_retail(SCP_string& text);

	/**
	 * @brief Save asteroid field (singular) to file
	 *
	 * @details Returns the value of CFred_mission_save::err, which is:
	 *
	 * @returns 0 for no error, or
	 * @returns A negative value if an error occurred
	 */
	int save_asteroid_fields();

	//	int save_briefing_info();

	/**
	 * @brief Save the briefing to file
	 *
	 * @details Returns the value of CFred_mission_save::err, which is:
	 *
	 * @returns 0 for no error, or
	 * @returns A negative value if an error occurred
	 */
	int save_briefing();

	/**
	 * @brief Save the command briefing to file
	 *
	 * @details Returns the value of CFred_mission_save::err, which is:
	 *
	 * @returns 0 for no error, or
	 * @returns A negative value if an error occurred
	 */
	int save_cmd_brief();

	/**
	 * @brief Save all command briefings to file
	 *
	 * @details Returns the value of CFred_mission_save::err, which is:
	 *
	 * @returns 0 for no error, or
	 * @returns A negative value if an error occurred
	 */
	int save_cmd_briefs();

	/**
	 * @brife Save "common" object data
	 *
	 * @param[in] objp
	 * @param[in] shipp
	 *
	 * @details Returns the value of CFred_mission_save::err, which is:
	 *
	 * @returns 0 for no error, or
	 * @returns A negative value if an error occurred
	 */
	int save_common_object_data(object* objp, ship* shipp);

	/**
	 * @brief Saves "custom" bitmaps to file
	 *
	 * @param[in] expected_string_640  Optional. "$Background 640:"
	 * @param[in] expected_string_1024 Optional. "$Background 1024:"
	 * @param[in] string_field_640     (Required if expected_string_640 defined)  Name of the background for 640
	 * resolution
	 * @param[in] string_field_1024    (Required if expected_string_1024 defined) Name of the background for 1024
	 * resolution
	 * @param[in] blank_lines          Optional. Pads the bitmap entry by this many blank lines
	 */
	void save_custom_bitmap(const char* expected_string_640,
		const char* expected_string_1024,
		const char* string_field_640,
		const char* string_field_1024,
		int blank_lines = 0);

	/**
	 * @brief Saves cutscenes to file
	 *
	 * @details Returns the value of CFred_mission_save::err, which is:
	 *
	 * @returns 0 for no error, or
	 * @returns A negative value if an error occurred
	 */
	int save_cutscenes();

	/**
	 * @brief Saves debriefing to a file
	 *
	 * @details Returns the value of CFred_mission_save::err, which is:
	 *
	 * @returns 0 for no error, or
	 * @returns A negative value if an error occurred
	 */
	int save_debriefing();

	/**
	 * @brief Saves events to a file
	 *
	 * @details Returns the value of CFred_mission_save::err, which is:
	 *
	 * @returns 0 for no error, or
	 * @returns A negative value if an error occurred
	 */
	int save_events();

	/**
	 * @brief Saves fiction to a file
	 *
	 * @details Returns the value of CFred_mission_save::err, which is:
	 *
	 * @returns 0 for no error, or
	 * @returns A negative value if an error occurred
	 */
	int save_fiction();

	/**
	 * @brief Saves goals to a file
	 *
	 * @details Returns the value of CFred_mission_save::err, which is:
	 *
	 * @returns 0 for no error, or
	 * @returns A negative value if an error occurred
	 */
	int save_goals();

	/**
	 * @brief Saves the given matrix to file
	 *
	 * @param[in] m Matrix to save
	 *
	 * @details Returns the value of CFred_mission_save::err, which is:
	 *
	 * @returns 0 for no error, or
	 * @returns A negative value if an error occurred
	 */
	int save_matrix(matrix& m);

	/**
	 * @brief Saves the messages/in-mission dialog to file
	 *
	 * @details Returns the value of CFred_mission_save::err, which is:
	 *
	 * @returns 0 for no error, or
	 * @returns A negative value if an error occurred
	 */
	int save_messages();

	/**
	 * @brief Saves mission info to file
	 *
	 * @details Returns the value of CFred_mission_save::err, which is:
	 *
	 * @returns 0 for no error, or
	 * @returns A negative value if an error occurred
	 */
	int save_mission_info();

	/**
	 * @brief Saves debriefing to a file
	 *
	 * @details Returns the value of CFred_mission_save::err, which is:
	 *
	 * @returns 0 for no error, or
	 * @returns A negative value if an error occurred
	 */
	void save_mission_internal(const char* pathname);

	/**
	 * @brief Saves music entries to file
	 *
	 * @details Returns the value of CFred_mission_save::err, which is:
	 *
	 * @returns 0 for no error, or
	 * @returns A negative value if an error occurred
	 */
	int save_music();

	/**
	 * @brief Saves custom entries to file
	 *
	 * @details Returns the value of CFred_mission_save::err, which is:
	 *
	 * @returns 0 for no error, or
	 * @returns A negative value if an error occurred
	 */
	int save_custom_data();

	/**
	 * Helper function for save_objects().
	 */
	int save_warp_params(WarpDirection direction, ship* shipp);

	/**
	 * @brief Saves object entries to a file
	 *
	 * @details Returns the value of CFred_mission_save::err, which is:
	 *
	 * @returns 0 for no error, or
	 * @returns A negative value if an error occurred
	 */
	int save_objects();

	/**
	 * @brief Saves player entries to a file
	 *
	 * @details Returns the value of CFred_mission_save::err, which is:
	 *
	 * @returns 0 for no error, or
	 * @returns A negative value if an error occurred
	 */
	int save_players();

	/**
	 * @brief Saves plot info to a file
	 *
	 * @details Returns the value of CFred_mission_save::err, which is:
	 *
	 * @returns 0 for no error, or
	 * @returns A negative value if an error occurred
	 *
	 * @TODO This is possibly an unimplemented feature. "Blah" does not seem to be informative
	 */
	int save_plot_info();

	/**
	 * @brief Saves a docking instance
	 * @author Goober5000
	 *
	 * @param[in] shipp    Reference to docker ship (?)
	 * @param[in] dock_ptr Reference to dock pair
	 */
	void save_single_dock_instance(ship* shipp, dock_instance* dock_ptr);

	/**
	 * @brief Saves sexp variables to a file
	 *
	 * @details Returns the value of CFred_mission_save::err, which is:
	 *
	 * @returns 0 for no error, or
	 * @returns A negative value if an error occurred
	 */
	int save_variables();

	/**
	 * @brief Saves sexp containers to a file
	 *
	 * @details Returns the value of CFred_mission_save::err, which is:
	 *
	 * @returns 0 for no error, or
	 * @returns A negative value if an error occurred
	 */
	int save_containers();
	// helper function for non-type options, called only by save_containers()
	void save_container_options(const sexp_container& container);

	/**
	 * @brief Saves the given vector to file
	 *
	 * @param[in] v Vector to save
	 *
	 * @details Returns the value of CFred_mission_save::err, which is:
	 *
	 * @returns 0 for no error, or
	 * @returns A negative value if an error occurred
	 */
	int save_vector(const vec3d& v);

	/**
	 * @brief Saves waypoints to file
	 *
	 * @details Returns the value of CFred_mission_save::err, which is:
	 *
	 * @returns 0 for no error, or
	 * @returns A negative value if an error occurred
	 */
	int save_waypoints();

	/**
	 * @brief Saves the given waypoint list to file
	 *
	 * @param[in] w waypoint list to save
	 *
	 * @details Returns the value of CFred_mission_save::err, which is:
	 *
	 * @returns 0 for no error, or
	 * @returns A negative value if an error occurred
	 */
	int save_waypoint_list(const waypoint_list* wp_list);

	/**
	 * @brief Saves the wing entries to file
	 *
	 * @details Returns the value of CFred_mission_save::err, which is:
	 *
	 * @returns 0 for no error, or
	 * @returns A negative value if an error occurred
	 */
	int save_wings();

	/**
	 * @brief Utility function to save a raw comment, the start of which precedes the current raw_ptr, to a file while
	 * handling newlines properly
	 */
	void fout_raw_comment(const char* comment_start);
};