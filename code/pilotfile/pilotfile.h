
#ifndef _PILOTFILE_H
#define _PILOTFILE_H

#include "cfile/cfile.h"
#include "globalincs/pstypes.h"
#include "stats/scoring.h"
#include "pilotfile/FileHandler.h"

#include <memory>

class player;
struct sexp_container;


// current pilot constants
static const unsigned int PLR_FILE_ID = 0x5f524c50;	// "PLR_" in file
static const unsigned int CSG_FILE_ID = 0x5f475343;	// "CSG_" in file
// NOTE: Version should be bumped only for adding/removing sections or section
//       content.  It should *NOT* be bumped for limit bumps or anything of
//       that sort!
//   0 - initial version
//   1 - Adding support for the player is multi flag
//   2 - Add language in use when pilot was created
//	     (due to intel entries using translated text as the primary key)
//   3 - Add SEXP containers
//   4   Controls are removed, and instead a preset name is saved/loaded
static const ubyte PLR_VERSION = 4;
//   0 - initial version
//   1 - re-add recent missions
//   2 - separate single/multi squad name & pic
//   3 - remove separate detail settings for campaigns
//   4 - add CPV rollback for Red Alert missions
//   5 - save rank to flags for quick access
//   6 - add SEXP containers
//   7 - Controls are removed, and instead a preset name is saved/loaded.
static const ubyte CSG_VERSION = 7;

typedef struct index_list_t {
	SCP_string name;
	int index{ -1 };
	int val{ 0 };
} index_list_t;

// special stats struct, since our use here is not content specific
typedef struct scoring_special_t {
	int score{ 0 };
	int rank{ RANK_ENSIGN };
	int assists{ 0 };
	int kill_count{ 0 };
	int kill_count_ok{ 0 };
	int bonehead_kills{ 0 };

	unsigned int p_shots_fired{ 0 };
	unsigned int p_shots_hit{ 0 };
	unsigned int p_bonehead_hits{ 0 };

	unsigned int s_shots_fired{ 0 };
	unsigned int s_shots_hit{ 0 };
	unsigned int s_bonehead_hits{ 0 };

	unsigned int missions_flown{ 0 };
	unsigned int flight_time{ 0 };
	_fs_time_t last_flown{ 0 };
	_fs_time_t last_backup{ 0 };

	SCP_vector<index_list_t> ship_kills;
	SCP_vector<index_list_t> medals_earned;
} scoring_special_t;

class pilotfile {
	public:
		pilotfile();
		~pilotfile();

		bool load_player(const char *callsign, player *_p = nullptr, bool force_binary = false);
		bool load_savefile(player *_p, const char *campaign);

		bool save_player(player *_p = nullptr);
		bool save_savefile();

		// updating stats, multi and/or all-time
		void update_stats(scoring_struct *stats, bool training = false);
		void update_stats_backout(scoring_struct *stats, bool training = false);
		void reset_stats();

		// for checking to see if a PLR file is basically valid
		bool verify(const char *fname, int *rank = nullptr, char *valid_language = nullptr);

		// whether current campaign savefile has valid data to work with
		bool is_invalid()
		{
			return m_data_invalid;
		}

	private:
		// --------------------------------------------------------------------
		// info shared between PLR and CSG ...
		// --------------------------------------------------------------------
		CFILE *cfp;
		std::unique_ptr<pilot::FileHandler> handler;
		SCP_string filename;
		player *p;

		int version;
		ubyte csg_ver;

		// some sections are required before others...
		bool m_have_flags;
		bool m_have_info;

		// set in case data appears wrong, so we can avoid loading/saving campaign savefile
		bool m_data_invalid;

		// overall content list, can include reference to more than current
		// mod/campaign provides
		// NOTE:  order of each list **must be preserved**
		SCP_vector<index_list_t> ship_list;
		SCP_vector<index_list_t> weapon_list;
		SCP_vector<index_list_t> intel_list;
		SCP_vector<index_list_t> medals_list;

		scoring_special_t all_time_stats;
		scoring_special_t multi_stats;

		// for writing files, sets/updates section info
		void startSection(Section section_id);
		void endSection();
		// file offset of the size value for the current section (set with startSection())
		size_t m_size_offset;


		// --------------------------------------------------------------------
		// PLR specific
		// --------------------------------------------------------------------
	    void plr_reset_data(bool reset_all);
		void plr_close();

		void plr_read_flags();
		void plr_read_info();
		void plr_read_settings();
		void plr_read_stats();
		void plr_read_stats_multi();
		void plr_read_multiplayer();
		void plr_read_variables();
		void plr_read_containers();

		void plr_read_hud();
		void plr_read_controls();

		void plr_write_flags();
		void plr_write_info();
		void plr_write_settings();
		void plr_write_stats();
		void plr_write_stats_multi();
		void plr_write_multiplayer();
		void plr_write_variables();
		void plr_write_containers();
		void plr_write_hud();
		void plr_write_controls();

		// --------------------------------------------------------------------
		// CSG specific
		// --------------------------------------------------------------------
		void csg_reset_data();
		void csg_close();

		void csg_read_flags();
		void csg_read_info();
		void csg_read_missions();
		void csg_read_techroom();
		void csg_read_loadout();
		void csg_read_stats();
		void csg_read_redalert();
		void csg_read_hud();
		void csg_read_variables();
		void csg_read_settings();
		void csg_read_controls();
		void csg_read_cutscenes();
		void csg_read_lastmissions();
		void csg_read_containers();
		void csg_read_container(sexp_container& container);

		void csg_write_flags();
		void csg_write_info();
		void csg_write_missions();
		void csg_write_techroom();
		void csg_write_loadout();
		void csg_write_stats();
		void csg_write_redalert();
		void csg_write_hud();
		void csg_write_variables();
		void csg_write_settings();
		void csg_write_controls();
		void csg_write_cutscenes();
		void csg_write_lastmissions();
		void csg_write_containers();
		void csg_write_container(const sexp_container& container);

		// similar to PLR verify, except we only want the rank
		bool get_csg_rank(int *rank);

};

extern pilotfile Pilot;

extern void convert_pilot_files();

#endif	// _PILOTFILE_H
