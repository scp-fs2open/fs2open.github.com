
#include "mission/management.h"

#include "object.h"

#include <object/waypoint.h>
#include <object/object.h>
#include <ship/ship.h>
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

#include <clocale>

extern int Xstr_inited;

extern void allocate_mission_text(size_t size);

extern void parse_init(bool basic = false);

namespace fso {
namespace fred {


void initialize(const std::string& cfilepath, int argc, char* argv[], Editor* editor, InitializerCallback listener) {

#ifndef NDEBUG
	outwnd_init();

	load_filter_info();
#endif

	std::setlocale(LC_ALL, "C");

	std::vector<std::pair<std::function<void(void)>, SubSystem>> initializers =
		{{ std::bind(os_init, Osreg_company_name, Osreg_app_name, nullptr, nullptr), SubSystem::OS },
		 { timer_init,                                                               SubSystem::Timer },
		 { [&]() {
			 // this should enable mods - Kazan
			 parse_cmdline(argc, argv);
		 },                                                                          SubSystem::CommandLine },
		 { [&cfilepath](void) {
			 // cfile needs a file path...
			 auto file_path = cfilepath + DIR_SEPARATOR_STR + "qtfred.exe";

			 cfile_init(file_path.c_str());
		 },                                                                          SubSystem::CFile },
		 { []() {
			 lcl_init(FS2_OPEN_DEFAULT_LANGUAGE);

			 // Goober5000 - force init XSTRs (so they work, but only work in English, based on above comment)
			 Xstr_inited = 1;
		 },                                                                          SubSystem::Locale },
		 { [=]() {
			 std::unique_ptr<QtGraphicsOperations> graphicsOps(new QtGraphicsOperations(editor));
			 gr_init(std::move(graphicsOps));
		 },                                                                          SubSystem::Graphics },
		 { font::init,                                                               SubSystem::Fonts },
		 { key_init,                                                                 SubSystem::Keyboard },
		 { mouse_init,                                                               SubSystem::Mouse },
		 { particle::ParticleManager::init,                                          SubSystem::Particles },
		 { iff_init,                                                                 SubSystem::Iff },
		 { obj_init,                                                                 SubSystem::Objects },
		 { species_init,                                                             SubSystem::Species },
		 { mission_brief_common_init,                                                SubSystem::MissionBrief },
		 { ai_init,                                                                  SubSystem::AI },
		 { ai_profiles_init,                                                         SubSystem::AIProfiles },
		 { armor_init,                                                               SubSystem::Armor },
		 { weapon_init,                                                              SubSystem::Weapon },
		 { parse_medal_tbl,                                                          SubSystem::Medals },
		 { ship_init,                                                                SubSystem::Ships },
		 { []() { parse_init(); },                                                   SubSystem::Parse },
		 { neb2_init,                                                                SubSystem::Nebulas },
		 { []() {
			 stars_init();
			 stars_pre_level_init(true);
			 stars_post_level_init();
		 },                                                                          SubSystem::Stars }};

	for (const auto& initializer : initializers) {
		const auto& init_function = initializer.first;
		const auto& which = initializer.second;
		listener(which);
		init_function();
	}
}

void shutdown() {
	audiostream_close();
	snd_close();
	gr_close();

	os_cleanup();

#ifndef NDEBUG
	outwnd_close();
#endif
}


mission_load_error::mission_load_error(const char* msg) : std::runtime_error{ msg } {
}

}
}
