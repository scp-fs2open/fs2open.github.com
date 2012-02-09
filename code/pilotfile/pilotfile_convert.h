
/* WARNING:
 *    The conversion code is for converting pl2/plr versions 242 & 142 and cs2
 *    version 15 ONLY!!!
 *
 *    Those versions are the only ones that allow for the conversion process to
 *    run without being directly tied to the active game data.  Attempting
 *    conversion on older versions will almost certainly result in data
 *    corruption in the converted file!
 */

/* NOTE:
 *    Lots of duplicate info here for the old pilot file format.  This is done
 *    so that any future engine changes won't break the conversion process
 *    or worrying about whether said changes would break anything.  It also
 *    allows for better conversion sanity checks, and allows for the conversion
 *    process to run totally independently of current game data.
 */

#define REDALERT_INTERNAL

#include "globalincs/pstypes.h"
#include "cfile/cfile.h"
#include "stats/scoring.h"
#include "parse/sexp.h"
#include "mission/missioncampaign.h"
#include "controlconfig/controlsconfig.h"
#include "missionui/redalert.h"



typedef struct index_list_t {
	SCP_string name;
	int index;
	int val;

	index_list_t() :
		index(-1), val(0)
	{
	}
} index_list_t;

// special stats struct, since our use here is not content specific
typedef struct scoring_special_t {
	int score;
	int rank;
	int assists;
	int kill_count;
	int kill_count_ok;
	int bonehead_kills;

	unsigned int p_shots_fired;
	unsigned int p_shots_hit;
	unsigned int p_bonehead_hits;

	unsigned int s_shots_fired;
	unsigned int s_shots_hit;
	unsigned int s_bonehead_hits;

	unsigned int missions_flown;
	unsigned int flight_time;
	_fs_time_t last_flown;
	_fs_time_t last_backup;

	SCP_vector<index_list_t> ship_kills;
	SCP_vector<index_list_t> medals_earned;

	scoring_special_t() :
		score(0), rank(RANK_ENSIGN), assists(0), kill_count(0), kill_count_ok(0),
		bonehead_kills(0), p_shots_fired(0), p_shots_hit(0), p_bonehead_hits(0),
		s_shots_fired(0), s_shots_hit(0), s_bonehead_hits(0), missions_flown(0),
		flight_time(0), last_flown(0), last_backup(0)
	{
	}
} scoring_special_t;

typedef struct cmission_conv_t {
	int index;			// index into Campaign.missions[]
	int flags;

	SCP_vector<mgoal>	goals;
	SCP_vector<mevent>	events;
	SCP_vector<sexp_variable>	variables;

	scoring_special_t	stats;
} cmission_conv_t;

typedef struct wss_unit_conv_t {
	int ship_index;
	int wep[12];
	int wep_count[12];

	wss_unit_conv_t() :
		ship_index(-1)
	{
	}
} wss_unit_conv_t;

typedef struct loadout_conv_t {
	SCP_string filename;
	SCP_string last_modified;

	wss_unit_conv_t slot[12];

	SCP_vector<int> weapon_pool;
	SCP_vector<int> ship_pool;
} loadout_conv_t;


struct plr_data {
	plr_data();
	~plr_data();

	// -------------------------------------------------------------------------
	// NOTE: we *do not* carry over the following items because they are either
	//       obsolete, removed functionality, or just not needed.  But we still
	//       do have to take them into account during read ...
	//
	//     - stats (ver 142 only)
	//     - loadout (ver 142 only)
    //     - techroom (ver 142 only)
    //     - recent missions (ver 142 & 242)
	//     - red alert (ver 142 only)
	// -------------------------------------------------------------------------

	// not carried over, just for reference during conversion process
	int version;
	int is_multi;


	// basic flags and settings
	int tips;
	int rank;
	int skill_level;
	int save_flags;
	int readyroom_listing_mode;
	int voice_enabled;
	int auto_advance;
	int Use_mouse_to_fly;
	int Mouse_sensitivity;
	int Joy_sensitivity;
	int Dead_zone_size;

	// multiplayer settings/options
	int net_protocol;

	unsigned char multi_squad_set;
	unsigned char multi_endgame_set;
	int multi_flags;
	unsigned int multi_respawn;
	unsigned char multi_max_observers;
	unsigned char multi_skill_level;
	unsigned char multi_voice_qos;
	int multi_voice_token_wait;
	int multi_voice_record_time;
	int multi_time_limit;
	int multi_kill_limit;

	int multi_local_flags;
	int multi_local_update_level;

	// pilot info stuff
	char image_filename[35];
	char squad_name[35];
	char squad_filename[35];
	char current_campaign[35];
	char last_ship_flown[35];

	// HUD config
	int hud_show_flags;
	int hud_show_flags2;
	int hud_popup_flags;
	int hud_popup_flags2;
	unsigned char hud_num_lines;
	int hud_rp_flags;
	int hud_rp_dist;
	unsigned char hud_colors[39][4];

	// control setup
	SCP_vector<config_item> controls;

	int joy_axis_map_to[5];
	int joy_invert_axis[5];

	// audio
	float sound_volume;
	float music_volume;
	float voice_volume;

	// detail settings
	int detail_setting;
	int detail_nebula;
	int detail_distance;
	int detail_hardware_textures;
	int detail_num_debris;
	int detail_num_particles;
	int detail_num_stars;
	int detail_shield_effects;
	int detail_lighting;
	int detail_targetview_model;
	int detail_planets_suns;
	int detail_weapon_extras;

	// variables
	SCP_vector<sexp_variable> variables;
};

struct csg_data {
	csg_data();
	~csg_data();

	int sig;
	int cutscenes;

	SCP_string main_hall;
	int prev_mission;
	int next_mission;
	int loop_reentry;
	int loop_enabled;
	int num_completed;

	int last_ship_flown_index;

	SCP_vector<bool> ships_allowed;
	SCP_vector<bool> weapons_allowed;

	SCP_vector<bool> ships_techroom;
	SCP_vector<bool> weapons_techroom;
	SCP_vector<bool> intel_techroom;

	SCP_vector<index_list_t> ship_list;
	SCP_vector<index_list_t> weapon_list;
	SCP_vector<index_list_t> intel_list;
	SCP_vector<index_list_t> medals_list;

	SCP_vector<cmission_conv_t> missions;

	SCP_vector<sexp_variable> variables;

	loadout_conv_t loadout;

	scoring_special_t stats;

	SCP_vector<red_alert_ship_status> wingman_status;
	SCP_string precursor_mission;
};

class pilotfile_convert {
	public:
		pilotfile_convert();
		~pilotfile_convert();

		bool load();
		void save();

		bool plr_convert(const char *fname, bool inferno);
		bool csg_convert(const char *fname, bool inferno);

	private:
		CFILE *cfp;
		unsigned int fver;

		scoring_special_t all_time_stats;
		scoring_special_t multi_stats;

		// sections of a pilot file. includes both plr and csg sections
		struct Section {
			enum id {
				Flags			= 0x0001,
				Info			= 0x0002,
				Loadout			= 0x0003,
				Controls		= 0x0004,
				Multiplayer		= 0x0005,
				Scoring			= 0x0006,
				ScoringMulti	= 0x0007,
				Techroom		= 0x0008,
				HUD				= 0x0009,
				Settings		= 0x0010,
				RedAlert		= 0x0011,
				Variables		= 0x0012,
				Missions		= 0x0013
			};
		};

		// for writing files, sets/updates section info
		void startSection(Section::id section_id);
		void endSection();
		// file offset of the size value for the current section (set with startSection())
		size_t m_size_offset;


		// --------------------------------------------------------------------
		// PLR specific
		// --------------------------------------------------------------------
		plr_data *plr;

		void plr_import();
		void plr_import_controls();
		void plr_import_hud();
		void plr_import_detail();
		void plr_import_stats();
		void plr_import_loadout();
		void plr_import_multiplayer();
		void plr_import_red_alert();
		void plr_import_variables();

		void plr_export();
		void plr_export_flags();
		void plr_export_info();
		void plr_export_stats();
		void plr_export_stats_multi();
		void plr_export_hud();
		void plr_export_variables();
		void plr_export_multiplayer();
		void plr_export_controls();
		void plr_export_settings();

		// --------------------------------------------------------------------
		// CSG specific
		// --------------------------------------------------------------------
		csg_data *csg;

		void csg_import(bool inferno);
		void csg_import_loadout();
		void csg_import_stats();
		void csg_import_techroom();
		void csg_import_red_alert();
		void csg_import_missions(bool inferno);
		void csg_import_ships_weapons();

		void csg_export();
		void csg_export_flags();
		void csg_export_info();
		void csg_export_missions();
		void csg_export_techroom();
		void csg_export_loadout();
		void csg_export_stats();
		void csg_export_redalert();
		void csg_export_hud();
		void csg_export_variables();
};

