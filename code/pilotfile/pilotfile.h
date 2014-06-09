
#ifndef _PILOTFILE_H
#define _PILOTFILE_H

#include "globalincs/pstypes.h"
#include "cfile/cfile.h"
#include "stats/scoring.h"


class player;


// current pilot constants
static const unsigned int PLR_FILE_ID = 0x5f524c50;	// "PLR_" in file
static const unsigned int CSG_FILE_ID = 0x5f475343;	// "CSG_" in file
// NOTE: Version should be bumped only for adding/removing sections or section
//       content.  It should *NOT* be bumped for limit bumps or anything of
//       that sort!
//   0 - initial version
//   1 - Adding support for the player is multi flag
static const ubyte PLR_VERSION = 1;
//   0 - initial version
//   1 - re-add recent missions
//   2 - separate single/multi squad name & pic
//   3 - remove separate detail settings for campaigns
//   4 - add CPV rollback for Red Alert missions
static const ubyte CSG_VERSION = 4;


class pilotfile {
	public:
		pilotfile();
		~pilotfile();

		bool load_player(const char *callsign, player *_p = NULL);
		bool load_savefile(const char *campaign);

		bool save_player(player *_p = NULL);
		bool save_savefile();

		// updating stats, multi and/or all-time
		void update_stats(scoring_struct *stats, bool training = false);
		void update_stats_backout(scoring_struct *stats, bool training = false);
		void reset_stats();

		// for checking to see if a PLR file is basically valid
		bool verify(const char *fname, int *rank = NULL);

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
		SCP_string filename;
		player *p;

		int version;
		ubyte csg_ver;

		// some sections are required before others...
		bool m_have_flags;
		bool m_have_info;

		// set in case data appears wrong, so we can avoid loading/saving campaign savefile
		bool m_data_invalid;

		typedef struct index_list_t {
			SCP_string name;
			int index;
			int val;

			index_list_t() :
				index(-1), val(0)
			{
			}
		} index_list_t;

		// overall content list, can include reference to more than current
		// mod/campaign provides
		// NOTE:  order of each list **must be preserved**
		SCP_vector<index_list_t> ship_list;
		SCP_vector<index_list_t> weapon_list;
		SCP_vector<index_list_t> intel_list;
		SCP_vector<index_list_t> medals_list;

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
				Missions		= 0x0013,
				Cutscenes		= 0x0014,
				LastMissions	= 0x0015
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
		void plr_reset_data();
		void plr_close();

		void plr_read_flags();
		void plr_read_info();
		void plr_read_settings();
		void plr_read_stats();
		void plr_read_stats_multi();
		void plr_read_multiplayer();
		void plr_read_variables();
		void plr_read_hud();
		void plr_read_controls();

		void plr_write_flags();
		void plr_write_info();
		void plr_write_settings();
		void plr_write_stats();
		void plr_write_stats_multi();
		void plr_write_multiplayer();
		void plr_write_variables();
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
};

extern pilotfile Pilot;

extern void convert_pilot_files();

#endif	// _PILOTFILE_H
