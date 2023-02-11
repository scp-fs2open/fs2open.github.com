#include <unordered_map>
#include <memory>
#include <functional>

#include <QApplication>
#include <QDir>
#include <QSplashScreen>
#include <QTimer>
#include <QtCore/QLoggingCategory>

#ifdef _WIN32
#include "globalincs/mspdb_callstack.h"
#endif

#include "mission/Editor.h"
#include "mission/management.h"
#include "ui/widgets/renderwidget.h"
#include "globalincs/pstypes.h"

#include "ui/FredView.h"
#include "FredApplication.h"

#include <csignal>
#include <project.h>

// Globals needed by the engine when built in 'FRED' mode.
int Fred_running = 1;
int Show_cpu = 0;

// Empty functions to make fred link with the sexp_mission_set_subspace
void game_start_subspace_ambient_sound() {
}
void game_stop_subspace_ambient_sound() {
}

void fsoMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
	auto errorMsg = qFormatLogMessage(type, context, msg).toUtf8();
	switch (type) {
	case QtDebugMsg:
		mprintf(("Qt Debug: %s\n", errorMsg.constData()));
		fprintf(stderr, "Qt Debug: %s\n", errorMsg.constData());
		break;
	case QtInfoMsg:
		mprintf(("Qt Info: %s\n", errorMsg.constData()));
		fprintf(stderr, "Qt Info: %s\n", errorMsg.constData());
		break;
	case QtWarningMsg:
		mprintf(("Qt Warning: %s\n", errorMsg.constData()));
		fprintf(stderr, "Qt Warning: %s\n", errorMsg.constData());
		break;
	case QtCriticalMsg:
		mprintf(("Qt Critical: %s\n", errorMsg.constData()));
		fprintf(stderr, "Qt Critical: %s\n", errorMsg.constData());
		break;
	case QtFatalMsg:
		Error(context.file == nullptr ? "Unknown" : context.file, context.line, "Qt Critical: %s", errorMsg.constData());
		break;
	}
}

// SDL defines this on windows which causes problems
#ifdef main
#undef main
#endif

void handler(int signal) {
	auto stacktrace = dump_stacktrace();

	fprintf(stderr, "Stack: %s\n", stacktrace.c_str());
	exit( signal );
}

// Our callback to keep the window responsive while loading
void game_busy_callback(int  /*count*/) {
	qGuiApp->processEvents();
}

int main(int argc, char* argv[]) {
	signal( SIGSEGV, handler );

	using namespace fso::fred;

#ifdef WIN32
	SCP_mspdbcs_Initialise();
#endif

	if (LoggingEnabled) {
		outwnd_init();
	}

	qInstallMessageHandler(fsoMessageOutput);

	SDL_SetMainReady();

	QCoreApplication::setOrganizationName("HardLightProductions");
	QCoreApplication::setOrganizationDomain("hard-light.net");
	QCoreApplication::setApplicationName("qtFRED");

	QApplication app(argc, argv);

	// Expect that the platform library is in the same directory
	QCoreApplication::addLibraryPath(QCoreApplication::applicationDirPath());	

	QGuiApplication::setApplicationDisplayName(app.tr("qtFRED v%1").arg(FS_VERSION_FULL));

#ifndef NDEBUG
	QLoggingCategory::defaultCategory()->setEnabled(QtDebugMsg, true);
	QLoggingCategory::defaultCategory()->setEnabled(QtInfoMsg, true);
#endif

	QGuiApplication::setWindowIcon(QIcon(":/images/V_fred.ico"));

	// This will be available as the global instance
	FredApplication localFredApp;

	QSplashScreen splash(QPixmap(":/images/splash.png"));
	splash.show();
	qGuiApp->processEvents();
	std::unique_ptr<Editor> fred(new Editor());

	auto baseDir = QDir::toNativeSeparators(QDir::current().absolutePath());

	typedef std::unordered_map<SubSystem, QString> SubsystemMap;
	SubsystemMap initializers = {{ SubSystem::OS,                app.tr("Initializing OS interface") },
								 { SubSystem::CommandLine,       app.tr("Parsing command line") },
								 { SubSystem::Timer,             app.tr("Initializing Timer") },
								 { SubSystem::CFile,             app.tr("Initializing CFile") },
								 { SubSystem::Locale,            app.tr("Initializing locale") },
								 { SubSystem::Sound,             app.tr("Initializing sound") },
								 { SubSystem::Graphics,          app.tr("Initializing graphics") },
								 { SubSystem::Scripting,         app.tr("Initializing scripting") },
								 { SubSystem::Fonts,             app.tr("Initializing Fonts") },
								 { SubSystem::Keyboard,          app.tr("Initializing keyboard") },
								 { SubSystem::Mouse,             app.tr("Initializing mouse") },
								 { SubSystem::Particles,         app.tr("Initializing particles") },
								 { SubSystem::Iff,               app.tr("Initializing IFF") },
								 { SubSystem::Objects,           app.tr("Initializing objects") },
								 { SubSystem::Models,            app.tr("Initializing model system") },
								 { SubSystem::Species,           app.tr("Initializing species") },
								 { SubSystem::BriefingIcons,     app.tr("Initializing briefing icons") },
								 { SubSystem::HudCommOrders,     app.tr("Initializing HUD comm orders") },
								 { SubSystem::AlphaColors,       app.tr("Initializing alpha colors") },
								 { SubSystem::GameSound,         app.tr("Initializing briefing icons") },
								 { SubSystem::MissionBrief,      app.tr("Initializing briefings") },
								 { SubSystem::AI,                app.tr("Initializing AI") },
								 { SubSystem::AIProfiles,        app.tr("Initializing AI profiles") },
								 { SubSystem::Armor,             app.tr("Initializing armors") },
								 { SubSystem::Weapon,            app.tr("Initializing weaponry") },
								 { SubSystem::Medals,            app.tr("Initializing medals") },
								 { SubSystem::Glowpoints,        app.tr("Initializing glow points") },
								 { SubSystem::Ships,             app.tr("Initializing ships") },
								 { SubSystem::Parse,             app.tr("Initializing parser") },
								 { SubSystem::TechroomIntel,     app.tr("Initializing techroom intel") },
								 { SubSystem::Nebulas,           app.tr("Initializing nebulas") },
								 { SubSystem::Stars,             app.tr("Initializing stars") },
								 { SubSystem::Ssm,               app.tr("Initializing SSMs") },
								 { SubSystem::EventMusic,        app.tr("Initializing event music") },
								 { SubSystem::FictionViewer,     app.tr("Initializing fiction viewer") },
								 { SubSystem::CommandBriefing,   app.tr("Initializing command briefing") },
								 { SubSystem::Campaign,          app.tr("Initializing campaign system") },
								 { SubSystem::NebulaLightning,   app.tr("Initializing nebula lightning") },
								 { SubSystem::FFmpeg,            app.tr("Initializing FFmpeg") },
								 { SubSystem::SEXPs,             app.tr("Initializing SEXP system") },
								 { SubSystem::ScriptingInitHook, app.tr("Running game init scripting hook") },
	};

	auto initSuccess = fso::fred::initialize(baseDir.toStdString(), argc, argv, fred.get(), [&](const SubSystem& which) {
		if (initializers.count(which)) {
			splash.showMessage(initializers.at(which), Qt::AlignHCenter | Qt::AlignBottom, Qt::white);
		}
		qGuiApp->processEvents();
	});

	if (!initSuccess) {
		return -1;
	}

	splash.showMessage(app.tr("Showing editor window"), Qt::AlignHCenter | Qt::AlignBottom, Qt::white);
	splash.finish(qApp->activeWindow());

	// Use this to keep the app responsive
	game_busy_callback(game_busy_callback);

	// Find and show our window from the top level windows
	FredView* fredview(nullptr);
	for (auto& window : qApp->topLevelWidgets()) {
		fredview = qobject_cast<FredView*>(window);
		if (fredview != nullptr) break;
	}
	Assert(fredview != nullptr);
	fredview->showMaximized();

	// Allow other parts of the code to execute code that needs to run after everything has been set up
	fredApp->initializeComplete();

	if (Cmdline_start_mission) {
		// Automatically load a mission if specified on the command line
		QTimer::singleShot(500, [&]() {
			fred->loadMission(Cmdline_start_mission);
		});
	}

	// Render first correct frame
	QTimer::singleShot(50, [=]{
		Assert(fredview != nullptr);
		fredview->getRenderWidget()->renderFrame();
	});

	return QGuiApplication::exec();
}
