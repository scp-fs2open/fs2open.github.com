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

// Globals needed by the engine when built in 'FRED' mode.
int Fred_running = 1;
char Fred_callsigns[MAX_SHIPS][NAME_LENGTH+1];
char Fred_alt_names[MAX_SHIPS][NAME_LENGTH+1];
int Show_cpu = 0;

// Empty functions to make fred link with the sexp_mission_set_subspace
void game_start_subspace_ambient_sound() {}
void game_stop_subspace_ambient_sound() {}

// SDL defines this on windows which causes problems
#ifdef main
#undef main
#endif

int main(int argc, char *argv[])
{

    QApplication app(argc, argv);
    QSplashScreen splash(QPixmap(":/images/splash.png"));
    splash.show();
    app.processEvents();
    std::shared_ptr<fso::fred::Editor> fred = std::make_shared<fso::fred::Editor>();

#ifdef WIN32
    SCP_mspdbcs_Initialise();
#endif

    auto baseDir = QDir::toNativeSeparators(QDir::current().absolutePath());

    std::unordered_map<fso::fred::SubSystem, QString> initializers = {
        {fso::fred::SubSystem::OSRegistry, app.tr("OS registry")},
        {fso::fred::SubSystem::Timer, app.tr("Timer")},
        {fso::fred::SubSystem::CFile, app.tr("CFile")},
        {fso::fred::SubSystem::Locale, app.tr("Initialization locale")},
        {fso::fred::SubSystem::Graphics, app.tr("Initializating graphics")},
        {fso::fred::SubSystem::Fonts, app.tr("Fonts")},
        {fso::fred::SubSystem::Keyboard, app.tr("Initializing keyboard")},
        {fso::fred::SubSystem::Mouse, app.tr("Initializing mouse")},
        {fso::fred::SubSystem::Iff, app.tr("Initializing IFF")},
        {fso::fred::SubSystem::Objects, app.tr("Initializing objects")},
        {fso::fred::SubSystem::Species, app.tr("Initializing species")},
        {fso::fred::SubSystem::MissionBrief, app.tr("Initializing briefings")},
        {fso::fred::SubSystem::AI, app.tr("Initializing AI")},
        {fso::fred::SubSystem::AIProfiles, app.tr("Initializing AI profiles")},
        {fso::fred::SubSystem::Armor, app.tr("Initializing armors")},
        {fso::fred::SubSystem::Weapon, app.tr("Initializing weaponry")},
        {fso::fred::SubSystem::Medals, app.tr("Initializing medals")},
        {fso::fred::SubSystem::Ships, app.tr("Initializing ships")},
        {fso::fred::SubSystem::Nebulas, app.tr("Initializing nebulas")},
        {fso::fred::SubSystem::Stars, app.tr("Initializing stars")},
        {fso::fred::SubSystem::View, app.tr("Setting view")}
    };

    fso::fred::initialize(baseDir.toStdString(), [&](const fso::fred::SubSystem &which) {
        if (initializers.count(which))
            splash.showMessage(initializers.at(which), Qt::AlignHCenter | Qt::AlignBottom, Qt::white);
        app.processEvents();
    });

    fso::fred::MainWindow mw;
    mw.setEditor(fred);
    splash.showMessage(qApp->tr("Switching rendering window"), Qt::AlignHCenter | Qt::AlignBottom, Qt::white);
    app.processEvents();
    fred->setRenderWindow(reinterpret_cast<void *>(mw.effectiveWinId()));
    mw.show();
    splash.finish(&mw);

    auto ret = app.exec();

#ifdef WIN32
    SCP_mspdbcs_Cleanup();
#endif

    return ret;
}
