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

	QGuiApplication::setApplicationDisplayName(QApplication::tr("qtFRED v%1").arg(FS_VERSION_FULL));

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
	SubsystemMap initializers = {{ SubSystem::OS,                QApplication::tr("Initializing OS interface") },
								 { SubSystem::CommandLine,       QApplication::tr("Parsing command line") },
								 { SubSystem::Timer,             QApplication::tr("Initializing Timer") },
								 { SubSystem::CFile,             QApplication::tr("Initializing CFile") },
								 { SubSystem::Locale,            QApplication::tr("Initializing locale") },
								 { SubSystem::Sound,             QApplication::tr("Initializing sound") },
								 { SubSystem::Graphics,          QApplication::tr("Initializing graphics") },
								 { SubSystem::Scripting,         QApplication::tr("Initializing scripting") },
								 { SubSystem::Fonts,             QApplication::tr("Initializing Fonts") },
								 { SubSystem::Keyboard,          QApplication::tr("Initializing keyboard") },
								 { SubSystem::Mouse,             QApplication::tr("Initializing mouse") },
								 { SubSystem::Particles,         QApplication::tr("Initializing particles") },
								 { SubSystem::Iff,               QApplication::tr("Initializing IFF") },
								 { SubSystem::Objects,           QApplication::tr("Initializing objects") },
								 { SubSystem::Models,            QApplication::tr("Initializing model system") },
								 { SubSystem::Species,           QApplication::tr("Initializing species") },
								 { SubSystem::BriefingIcons,     QApplication::tr("Initializing briefing icons") },
								 { SubSystem::HudCommOrders,     QApplication::tr("Initializing HUD comm orders") },
								 { SubSystem::AlphaColors,       QApplication::tr("Initializing alpha colors") },
								 { SubSystem::GameSound,         QApplication::tr("Initializing briefing icons") },
								 { SubSystem::MissionBrief,      QApplication::tr("Initializing briefings") },
								 { SubSystem::AI,                QApplication::tr("Initializing AI") },
								 { SubSystem::AIProfiles,        QApplication::tr("Initializing AI profiles") },
								 { SubSystem::Armor,             QApplication::tr("Initializing armors") },
								 { SubSystem::Weapon,            QApplication::tr("Initializing weaponry") },
								 { SubSystem::Medals,            QApplication::tr("Initializing medals") },
								 { SubSystem::Glowpoints,        QApplication::tr("Initializing glow points") },
								 { SubSystem::Ships,             QApplication::tr("Initializing ships") },
								 { SubSystem::Parse,             QApplication::tr("Initializing parser") },
								 { SubSystem::TechroomIntel,     QApplication::tr("Initializing techroom intel") },
								 { SubSystem::Nebulas,           QApplication::tr("Initializing nebulas") },
								 { SubSystem::Stars,             QApplication::tr("Initializing stars") },
								 { SubSystem::Ssm,               QApplication::tr("Initializing SSMs") },
								 { SubSystem::EventMusic,        QApplication::tr("Initializing event music") },
								 { SubSystem::FictionViewer,     QApplication::tr("Initializing fiction viewer") },
								 { SubSystem::CommandBriefing,   QApplication::tr("Initializing command briefing") },
								 { SubSystem::Cutscenes,         QApplication::tr("Initializing cutscenes")},
								 { SubSystem::Mainhalls,         QApplication::tr("Initializing mainhalls")},
								 { SubSystem::Ranks,             QApplication::tr("Initializing ranks")},
								 { SubSystem::Campaign,          QApplication::tr("Initializing campaign system") },
								 { SubSystem::NebulaLightning,   QApplication::tr("Initializing nebula lightning") },
								 { SubSystem::FFmpeg,            QApplication::tr("Initializing FFmpeg") },
								 { SubSystem::DynamicSEXPs,      QApplication::tr("Initializing dynamic SEXP system") },
								 { SubSystem::ScriptingInitHook, QApplication::tr("Running game init scripting hook") },
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

	splash.showMessage(QApplication::tr("Showing editor window"), Qt::AlignHCenter | Qt::AlignBottom, Qt::white);
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
