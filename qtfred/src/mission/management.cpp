
#include "mission/management.h"

#include "object.h"

#include <localization/localize.h>
#include <ui/QtGraphicsOperations.h>
#include <io/key.h>
#include <io/mouse.h>
#include <iff_defs/iff_defs.h>
#include <weapon/weapon.h>
#include <stats/medals.h>
#include <nebula/neb.h>
#include <starfield/starfield.h>
#include <sound/audiostr.h>
#include <project.h>
#include <scripting/scripting.h>
#include <hud/hudsquadmsg.h>
#include <globalincs/alphacolors.h>

#include <menuui/techmenu.h>
#include <localization/fhash.h>
#include <gamesnd/eventmusic.h>
#include <cutscene/cutscenes.h>
#include <missionui/fictionviewer.h>
#include <menuui/mainhallmenu.h>
#include <stats/scoring.h>
#include <mission/missioncampaign.h>
#include <nebula/neblightning.h>
#include <libs/ffmpeg/FFmpeg.h>
#include <parse/sexp/sexp_lookup.h>
#include <utils/Random.h>

#include <clocale>

extern bool Xstr_inited;

extern void allocate_parse_text(size_t size);

extern void parse_init(bool basic = false);

extern void brief_init_colors();

extern void ssm_init();    // Need this to populate Ssm_info so OPF_SSM_CLASS does something. -MageKing17

extern void cmdline_debug_print_cmdline();

namespace fso {
namespace fred {

void fred_preload_all_briefing_icons() {
	for (auto& ii : Briefing_icon_info) {
		generic_anim_load(&ii.regular);
		hud_anim_load(&ii.fade);
		hud_anim_load(&ii.highlight);
	}
}

bool
initialize(const std::string& cfilepath, int argc, char* argv[], Editor* editor, const InitializerCallback& listener) {
	std::setlocale(LC_ALL, "C");

	Random::seed(static_cast<unsigned int>(time(nullptr)));

	listener(SubSystem::OS);
	os_init(Osreg_class_name, Osreg_app_name);

	listener(SubSystem::Timer);
	timer_init();

	listener(SubSystem::CommandLine);
	// this should enable mods - Kazan
	if (!parse_cmdline(argc, argv)) {
		// Command line contained an option that terminates the program immediately
		return false;
	}

#ifndef NDEBUG
#if FS_VERSION_REVIS == 0
	mprintf(("qtFred Open version: %i.%i.%i\n", FS_VERSION_MAJOR, FS_VERSION_MINOR, FS_VERSION_BUILD));
#else
	mprintf(("qtFred Open version: %i.%i.%i.%i\n", FS_VERSION_MAJOR, FS_VERSION_MINOR, FS_VERSION_BUILD, FS_VERSION_REVIS));
#endif

	cmdline_debug_print_cmdline();
#endif

	// d'oh
	// cfile needs a file path...
	auto file_path = cfilepath + DIR_SEPARATOR_STR + "qtfred.exe";

	listener(SubSystem::CFile);
	if (cfile_init(file_path.c_str())) {
		return false;
	}

	listener(SubSystem::ModTable);
	// Load game_settings.tbl
	mod_table_init();

	listener(SubSystem::Locale);
	// initialize localization module. Make sure this is done AFTER initializing OS.
	// NOTE: Fred should ALWAYS run without localization. Otherwise it might swap in another language
	// when saving - which would cause inconsistencies when externalizing to tstrings.tbl via Exstr
	// trust me on this :)
	lcl_init(LCL_UNTRANSLATED);

	// Goober5000 - force init XSTRs (so they work, but only work untranslated, based on above comment)
	Xstr_inited = true;

#ifndef NDEBUG
	load_filter_info();
#endif

	listener(SubSystem::Sound);
	snd_init();

	listener(SubSystem::Graphics);
	// Not ready for this yet
	//	Cmdline_nospec = 1;
	// 	Cmdline_noglow = 1;
	Cmdline_window = 1;

	std::unique_ptr<QtGraphicsOperations> graphicsOps(new QtGraphicsOperations(editor));
	gr_init(std::move(graphicsOps));

	io::mouse::CursorManager::get()->showCursor(false);

	// To avoid breaking current mods which do not support scripts in FRED we only initialize the scripting
	// system if a special mod_table option is set
	if (Enable_scripts_in_fred) {
		listener(SubSystem::Scripting);
		script_init();            //WMC
	}

	listener(SubSystem::Fonts);
	font::init();                    // loads up all fonts

	gr_set_gamma(3.0f);

	listener(SubSystem::Keyboard);
	key_init();

	listener(SubSystem::Mouse);
	mouse_init();

	listener(SubSystem::Particles);
	particle::ParticleManager::init();

	listener(SubSystem::Iff);
	iff_init();            // Goober5000

	listener(SubSystem::Species);
	species_init();        // Kazan

	listener(SubSystem::BriefingIcons);
	brief_icons_init();

	// for fred specific replacement texture stuff
	Fred_texture_replacements.clear();

	listener(SubSystem::HudCommOrders);
	hud_init_comm_orders();        // Goober5000

	listener(SubSystem::AlphaColors);
	alpha_colors_init();

	listener(SubSystem::GameSound);
	gamesnd_parse_soundstbl();        // needs to be loaded after species stuff but before interface/weapon/ship stuff - taylor

	listener(SubSystem::MissionBrief);
	mission_brief_common_init();

	// Initialize dynamic SEXPs. Must happen before ship init for LuaAI
	listener(SubSystem::DynamicSEXPs);
	sexp::dynamic_sexp_init();

	listener(SubSystem::Objects);
	obj_init();

	listener(SubSystem::Models);
	model_init();
	model_free_all();                // Free all existing models

	listener(SubSystem::AI);
	ai_init();

	listener(SubSystem::AIProfiles);
	ai_profiles_init();

	listener(SubSystem::Armor);
	armor_init();

	listener(SubSystem::Weapon);
	weapon_init();

	listener(SubSystem::Medals);
	parse_medal_tbl();            // get medal names for sexpression usage

	listener(SubSystem::Glowpoints);
	glowpoint_init();

	listener(SubSystem::Ships);
	ship_init();

	listener(SubSystem::Parse);
	parse_init();

	listener(SubSystem::TechroomIntel);
	techroom_intel_init();

	// get fireball IDs for sexpression usage
	// (we don't need to init the entire system via fireball_init, we just need the information)
	fireball_parse_tbl();

	// initialize and activate external string hash table
	// make sure to do here so that we don't parse the table files into the hash table - waste of space
	fhash_init();
	fhash_activate();

	listener(SubSystem::Nebulas);
	neb2_init();                        // fullneb stuff

	listener(SubSystem::Stars);
	stars_init();

	listener(SubSystem::Ssm);
	ssm_init();        // The game calls this after stars_init(), and we need Ssm_info initialized for OPF_SSM_CLASS. -MageKing17

	brief_init_colors();
	fred_preload_all_briefing_icons(); //phreak.  This needs to be done or else the briefing icons won't show up

	listener(SubSystem::EventMusic);
	event_music_init();

	listener(SubSystem::FictionViewer);
	fiction_viewer_reset();

	listener(SubSystem::CommandBriefing);
	cmd_brief_reset();
	Show_waypoints = TRUE;

	listener(SubSystem::Cutscenes);
	cutscene_init();

	listener(SubSystem::Mainhalls);
	main_hall_table_init();

	listener(SubSystem::Ranks);
	parse_rank_tbl();

	listener(SubSystem::Campaign);
	mission_campaign_clear();

	stars_post_level_init();

	// neb lightning
	listener(SubSystem::NebulaLightning);
	nebl_init();

	listener(SubSystem::FFmpeg);
	libs::ffmpeg::initialize();

	// wookieejedi
	// load in the controls and defaults including the controlconfigdefault.tbl
	// this allows the sexp tree in key-pressed to actually match what the game will use
	// especially useful when a custom Controlconfigdefaults.tbl is used
	control_config_common_init();

	listener(SubSystem::ScriptingInitHook);
	Script_system.RunInitFunctions();
	Script_system.RunCondition(CHA_GAMEINIT);

	return true;
}

void shutdown() {
	audiostream_close();
	snd_close();
	gr_close();

	os_cleanup();

	if (LoggingEnabled) {
		outwnd_close();
	}
}


mission_load_error::mission_load_error(const char* msg) : std::runtime_error{ msg } {
}

}
}
