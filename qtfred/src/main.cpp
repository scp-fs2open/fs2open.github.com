#include <unordered_map>
#include <memory>
#include <utility>
#include <functional>

#include <stdlib.h>

#include <QApplication>
#include <QDir>
#include <QSplashScreen>
#include <QtCore/QLoggingCategory>

#ifdef _WIN32
#include "globalincs/mspdb_callstack.h"
#endif

#include "globalincs/globals.h" // MAX_SHIPS, NAME_LENGTH

#include "mission/editor.h"

#include "ui/FredView.h"
#include "FredApplication.h"
#include "FredApplication.h"

#include <signal.h>

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

int main(int argc, char* argv[]) {
	signal( SIGSEGV, handler );

	using namespace fso::fred;

#ifdef WIN32
	SCP_mspdbcs_Initialise();
#endif

	Q_INIT_RESOURCE(resources);
	qInstallMessageHandler(fsoMessageOutput);

	SDL_SetMainReady();

	QCoreApplication::setOrganizationName("HardLightProductions");
	QCoreApplication::setOrganizationDomain("hard-light.net");
	QCoreApplication::setApplicationName("qtFRED");

	QApplication app(argc, argv);

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

	std::unordered_map<SubSystem, QString> initializers = {{ SubSystem::OS,           app.tr("Initializing OS interface") },
														   { SubSystem::CommandLine,  app.tr("Parsing command line") },
														   { SubSystem::Timer,        app.tr("Initializing Timer") },
														   { SubSystem::CFile,        app.tr("Initializing CFile") },
														   { SubSystem::Locale,       app.tr("Initialization locale") },
														   { SubSystem::Graphics,     app.tr("Initializing graphics") },
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
														   { SubSystem::Parse,        app.tr("Initializing parser") },
														   { SubSystem::Nebulas,      app.tr("Initializing nebulas") },
														   { SubSystem::Stars,        app.tr("Initializing stars") },
														   { SubSystem::View,         app.tr("Setting view") }};

	fso::fred::initialize(baseDir.toStdString(), argc, argv, fred.get(), [&](const SubSystem& which) {
		if (initializers.count(which)) {
			splash.showMessage(initializers.at(which), Qt::AlignHCenter | Qt::AlignBottom, Qt::white);
		}
		qGuiApp->processEvents();
	});

	splash.showMessage(app.tr("Showing editor window"), Qt::AlignHCenter | Qt::AlignBottom, Qt::white);
	splash.finish(qApp->activeWindow());

	for (auto& window : qApp->topLevelWidgets()) {
		// Show all top level windows that are our window
		if (qobject_cast<FredView*>(window) != nullptr) {
			window->showMaximized();
		}
	}

	// Allow other parts of the code to execute code that needs to run after everything has been set up
	fredApp->initializeComplete();

	return QGuiApplication::exec();
}
