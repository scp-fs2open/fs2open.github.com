#include "HelpTopicsDialogModel.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHelpEngine>
#include <QHelpEngineCore>
#include <QHelpSearchEngine>
#include <QRegularExpression>
#include <QStandardPaths>

#include "cfile/cfile.h"

// Declared in cfile... must be at file scope so the tutorial discovery code can see it.
extern int cfile_inited;

namespace fso::fred::dialogs {

// ---------------------------------------------------------------------------
// Static member definitions
// ---------------------------------------------------------------------------
QHelpEngine*               HelpTopicsDialogModel::_helpEngine    = nullptr;
QList<TutorialEntry>       HelpTopicsDialogModel::_tutorials;
QHash<QString, QByteArray> HelpTopicsDialogModel::_tutorialContent;

// ---------------------------------------------------------------------------
HelpTopicsDialogModel::HelpTopicsDialogModel(QObject* parent) : QObject(parent) {}

// ---------------------------------------------------------------------------
void HelpTopicsDialogModel::prewarm() {
	if (ensureEngineReady())
		_helpEngine->searchEngine()->scheduleIndexDocumentation();
	_tutorials = discoverTutorials();
}

QHelpEngine* HelpTopicsDialogModel::helpEngine() {
	return _helpEngine;
}

const QList<TutorialEntry>& HelpTopicsDialogModel::tutorials() {
	return _tutorials;
}

const QHash<QString, QByteArray>& HelpTopicsDialogModel::tutorialContent() {
	return _tutorialContent;
}

// ---------------------------------------------------------------------------
bool HelpTopicsDialogModel::ensureEngineReady() {
	if (_helpEngine != nullptr)
		return !_helpEngine->registeredDocumentations().isEmpty();

	const QString collectionFile = resolveCollectionFile();
	if (collectionFile.isEmpty()) {
		mprintf(("QtFRED help: could not determine a writable location for the help collection\n"));
		return false;
	}

	// Parent to qApp so Qt manages the lifetime. Destroyed cleanly before the event loop exits.
	_helpEngine = new QHelpEngine(collectionFile, qApp);
	if (!_helpEngine->setupData()) {
		mprintf(("QtFRED help: could not initialize engine: %s\n",
		         _helpEngine->error().toUtf8().constData()));
		delete _helpEngine;
		_helpEngine = nullptr;
		return false;
	}

	const QString contentFile = resolveContentFile();
	if (contentFile.isEmpty()) {
		mprintf(("QtFRED help: could not find qtfred_help.qch\n"));
		delete _helpEngine;
		_helpEngine = nullptr;
		return false;
	}

	const QString ns = QHelpEngineCore::namespaceName(contentFile);
	if (!ns.isEmpty()) {
		// Always unregister and re-register.  Qt copies file content from the .qch into the
		// .qhc SQLite database at registration time.  If the .qch is rebuilt without
		// re-registration the .qhc tables become stale: findFile() succeeds but fileData()
		// returns 0 bytes.  Forcing re-registration each startup is cheap and guarantees the
		// collection database is always in sync with the current .qch.
		if (_helpEngine->registeredDocumentations().contains(ns))
			_helpEngine->unregisterDocumentation(ns);

		if (!_helpEngine->registerDocumentation(contentFile)) {
			mprintf(("QtFRED help: failed to register %s: %s\n",
			         contentFile.toUtf8().constData(),
			         _helpEngine->error().toUtf8().constData()));
		} else {
			mprintf(("QtFRED help: registered namespace '%s' from %s\n",
			         ns.toUtf8().constData(),
			         contentFile.toUtf8().constData()));
		}
	}

	return !_helpEngine->registeredDocumentations().isEmpty();
}

// ---------------------------------------------------------------------------
QString HelpTopicsDialogModel::resolveCollectionFile() {
	const QString appDataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
	if (appDataDir.isEmpty())
		return {};
	QDir(appDataDir).mkpath(QStringLiteral("help"));
	return QDir(appDataDir).filePath(QStringLiteral("help/qtfred.qhc"));
}

QString HelpTopicsDialogModel::resolveContentFile() {
	// Check the FSO VFS for a mod override in data/freddocs/.
	if (cfile_inited) {
		CFileLocation loc = cf_find_file_location("qtfred_help.qch", CF_TYPE_FREDDOCS);
		if (loc.found && !loc.full_name.empty())
			return QString::fromStdString(loc.full_name);
	}

	// Fall back to the built-in .qch deployed next to the executable.
	return QDir(QCoreApplication::applicationDirPath()).filePath(
	    QStringLiteral("help/qtfred_help.qch"));
}

// ---------------------------------------------------------------------------
QString HelpTopicsDialogModel::extractHtmlTitle(const QString& filePath) {
	QFile f(filePath);
	if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
		return {};
	// Titles are always in the <head>, so a small read is enough.
	const QString head = QString::fromUtf8(f.read(4096));
	const QRegularExpression re(QStringLiteral("<title[^>]*>([^<]+)</title>"),
	                            QRegularExpression::CaseInsensitiveOption);
	const auto m = re.match(head);
	if (m.hasMatch())
		return m.captured(1).trimmed();
	return QFileInfo(filePath).completeBaseName();
}

QList<TutorialEntry> HelpTopicsDialogModel::discoverTutorials() {
	if (!cfile_inited)
		return {};

	SCP_vector<SCP_string> filenames;
	cf_get_file_list(filenames, CF_TYPE_FREDDOCS, "*.html");

	QList<TutorialEntry> result;
	for (const auto& filename : filenames) {
		// Skip files in subdirectories. Only surface top-level tutorials.
		// Mods can use subdirectories for assets or linked pages without flooding the list.
		if (filename.find('/') != SCP_string::npos || filename.find('\\') != SCP_string::npos)
			continue;

		// cf_get_file_list strips extensions; re-add it for the lookup and URL.
		const std::string fullFilename = filename + ".html";
		CFileLocation loc = cf_find_file_location(fullFilename.c_str(), CF_TYPE_FREDDOCS);
		if (!loc.found)
			continue;

		const QString fullPath = QString::fromStdString(loc.full_name);
		const QString urlPath  = QStringLiteral("/") + QString::fromStdString(fullFilename);

		QFile f(fullPath);
		if (!f.open(QIODevice::ReadOnly))
			continue;

		_tutorialContent[urlPath] = f.readAll();
		result.append({extractHtmlTitle(fullPath), fullPath, urlPath});
	}
	return result;
}

} // namespace fso::fred::dialogs
