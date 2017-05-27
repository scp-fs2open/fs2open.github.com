#include <unordered_map>
#include <memory>
#include <utility>
#include <functional>

#include <stdlib.h>

#include <QApplication>
#include <QDir>
#include <QSplashScreen>

#ifdef _WIN32
#include "globalincs/mspdb_callstack.h"
#ifndef _MINGW
#include <crtdbg.h>
#endif // !_MINGW
#endif

#include "globalincs/globals.h" // MAX_SHIPS, NAME_LENGTH

#include "editor.h"
#include "mainwindow.h"

#include "fredGlobals.h"

// Globals needed by the engine when built in 'FRED' mode.
int Fred_running = 1;
char Fred_callsigns[MAX_SHIPS][NAME_LENGTH + 1];
char Fred_alt_names[MAX_SHIPS][NAME_LENGTH + 1];
int Show_cpu = 0;

// Empty functions to make fred link with the sexp_mission_set_subspace
void game_start_subspace_ambient_sound() {
}
void game_stop_subspace_ambient_sound() {
}

// SDL defines this on windows which causes problems
#ifdef main
#undef main
#endif

int main(int argc, char* argv[]) {
	using namespace fso::fred;

	SDL_SetMainReady();

	QApplication app(argc, argv);

	// Init Qt parts of the global object
	fredGlobals->qtInit();

	QSplashScreen splash(QPixmap(":/images/splash.png"));
	splash.show();
	app.processEvents();
	std::unique_ptr<Editor> fred(new Editor());

	// Initialize renderer once initialize is complete
	// TODO: Decide how to handle this. It could be done inside Editor but that is currently free of Qt code
	QObject::connect(fredGlobals, &QtFredGlobals::initializeComplete, [&fred]() { fred->initializeRenderer(); });

#ifdef WIN32
	SCP_mspdbcs_Initialise();
#endif

	auto baseDir = QDir::toNativeSeparators(QDir::current().absolutePath());

	std::unordered_map<SubSystem, QString> initializers = {{ SubSystem::OS,           app.tr("Initializing OS interface") },
														   { SubSystem::CommandLine,  app.tr("Parsing command line") },
														   { SubSystem::Timer,        app.tr("Initializing Timer") },
														   { SubSystem::CFile,        app.tr("Initializing CFile") },
														   { SubSystem::Locale,       app.tr("Initialization locale") },
														   { SubSystem::Graphics,     app.tr("Initializating graphics") },
														   { SubSystem::Fonts,        app.tr("Initializing Fonts") },
														   { SubSystem::Keyboard,     app.tr("Initializing keyboard") },
														   { SubSystem::Mouse,        app.tr("Initializing mouse") },
														   { SubSystem::Particles,    app.tr("Initializing particles") },
														   { SubSystem::Iff,          app.tr("Initializing IFF") },
														   { SubSystem::Objects,      app.tr("Initializing objects") },
														   { SubSystem::Species,      app.tr("Initializing species") },
														   { SubSystem::MissionBrief, app.tr("Initializing briefings") },
														   { SubSystem::AI,           app.tr("Initializing AI") },
														   { SubSystem::AIProfiles,   app.tr("Initializing AI profiles") },
														   { SubSystem::Armor,        app.tr("Initializing armors") },
														   { SubSystem::Weapon,       app.tr("Initializing weaponry") },
														   { SubSystem::Medals,       app.tr("Initializing medals") },
														   { SubSystem::Ships,        app.tr("Initializing ships") },
														   { SubSystem::Nebulas,      app.tr("Initializing nebulas") },
														   { SubSystem::Stars,        app.tr("Initializing stars") },
														   { SubSystem::View,         app.tr("Setting view") }};

	fso::fred::initialize(baseDir.toStdString(), argc, argv, fred.get(), [&](const SubSystem& which) {
		if (initializers.count(which)) {
			splash.showMessage(initializers.at(which), Qt::AlignHCenter | Qt::AlignBottom, Qt::white);
		}
		app.processEvents();
	});

	// Allow other parts of the code to execute code that needs to run after everything has been set up
	emit fredGlobals->initializeComplete();

	app.processEvents();
	splash.close();

	auto ret = app.exec();

	// Clean up resources after we are done
	fso::fred::shutdown();

#ifdef WIN32
	SCP_mspdbcs_Cleanup();
#endif

	return ret;
}
