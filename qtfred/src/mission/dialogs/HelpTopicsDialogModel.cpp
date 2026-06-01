#include "HelpTopicsDialogModel.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QtHelp/QHelpEngine>
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
QHash<QString, QByteArray> HelpTopicsDialogModel::_assetCache;
TutorialEntry              HelpTopicsDialogModel::_sexpOperatorReference;
bool                       HelpTopicsDialogModel::_hasSexpOperatorReference = false;

// ---------------------------------------------------------------------------
HelpTopicsDialogModel::HelpTopicsDialogModel(QObject* parent) : QObject(parent) {}

// ---------------------------------------------------------------------------
void HelpTopicsDialogModel::prewarm() {
	if (ensureEngineReady())
		_helpEngine->searchEngine()->scheduleIndexDocumentation();
	_tutorialContent.clear();
	_assetCache.clear();
	_hasSexpOperatorReference = false;
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

const TutorialEntry* HelpTopicsDialogModel::sexpOperatorReference() {
	return _hasSexpOperatorReference ? &_sexpOperatorReference : nullptr;
}

// ---------------------------------------------------------------------------
bool HelpTopicsDialogModel::ensureEngineReady() {
	if (_helpEngine != nullptr)
		return !_helpEngine->registeredDocumentations().isEmpty();

	const QString bundleQhcPath = resolveBundleFile(QStringLiteral("help/qtfred_help.qhc"));
	const QString bundleQchPath = resolveBundleFile(QStringLiteral("help/qtfred_help.qch"));

	if (!QFileInfo::exists(bundleQhcPath)) {
		mprintf(("QtFRED help: collection file not found at %s\n",
		         bundleQhcPath.toUtf8().constData()));
		return false;
	}
	if (!QFileInfo::exists(bundleQchPath)) {
		mprintf(("QtFRED help: content file not found at %s\n",
		         bundleQchPath.toUtf8().constData()));
		return false;
	}

	// The collection must live in a writable location so Qt can write the search index
	// alongside it. On macOS the app bundle is read-only. We keep an up-to-date copy in
	// AppDataLocation and update it from the bundle when it's missing or newer.
	// We use a bundle-side pre-built .qhc (not a fresh one) to avoid the Qt6 issue where
	// setupData()+registerDocumentation() bootstrapping on an empty collection is unreliable.
	const QString helpDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
	                        + QStringLiteral("/help");
	QDir().mkpath(helpDir);
	const QString writableQhcPath = helpDir + QStringLiteral("/qtfred_help.qhc");

	// We need to know whether the AppDataLocation copy is already for this exact build
	// before opening the engine.  A small marker file stores the bundle .qch path that
	// was used when the copy was made.  This correctly handles switching between any build
	// versions (older or newer) because the comparison is path-based, not time-based.
	const QString markerPath   = helpDir + QStringLiteral("/qtfred_help_build.txt");
	QString       storedBundle;
	{
		QFile f(markerPath);
		if (f.open(QIODevice::ReadOnly))
			storedBundle = QString::fromUtf8(f.readAll()).trimmed();
	}

	if (storedBundle != bundleQchPath) {
		QFile::remove(writableQhcPath);
		if (!QFile::copy(bundleQhcPath, writableQhcPath)) {
			mprintf(("QtFRED help: could not copy collection to %s\n",
			         writableQhcPath.toUtf8().constData()));
			return false;
		}
	}

	// Parent to qApp so Qt manages the lifetime. Destroyed cleanly before the event loop exits.
	_helpEngine = new QHelpEngine(writableQhcPath, qApp);
	if (!_helpEngine->setupData()) {
		mprintf(("QtFRED help: could not initialize engine: %s\n",
		         _helpEngine->error().toUtf8().constData()));
		delete _helpEngine;
		_helpEngine = nullptr;
		return false;
	}

	// The copied .qhc stores a relative path to the .qch that won't resolve from
	// AppDataLocation. Re-register using the absolute bundle path.
	const QStringList docs = _helpEngine->registeredDocumentations();
	if (!docs.isEmpty()) {
		const QString registeredQch = _helpEngine->documentationFileName(docs.first());
		if (registeredQch != bundleQchPath) {
			_helpEngine->unregisterDocumentation(docs.first());
			if (!_helpEngine->registerDocumentation(bundleQchPath)) {
				mprintf(("QtFRED help: could not re-register documentation: %s\n",
				         _helpEngine->error().toUtf8().constData()));
				delete _helpEngine;
				_helpEngine = nullptr;
				return false;
			}
			// Update the marker to record which build this collection now points to.
			QFile f(markerPath);
			if (f.open(QIODevice::WriteOnly))
				f.write(bundleQchPath.toUtf8());
		}
	}

	return !_helpEngine->registeredDocumentations().isEmpty();
}

// ---------------------------------------------------------------------------
QString HelpTopicsDialogModel::resolveBundleFile(const QString& relativePath) {
	QDir dir(QCoreApplication::applicationDirPath());

#ifdef Q_OS_MACOS
	dir.cdUp();
	dir.cd("Resources");
#endif

	return dir.filePath(relativePath);
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
	QList<TutorialEntry> result;
	if (cfile_inited) {
		SCP_vector<SCP_string> filenames;
		cf_get_file_list(filenames, CF_TYPE_FREDDOCS, "*.html");
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

			if (QString::compare(QString::fromStdString(fullFilename),
			                     QStringLiteral("sexps.html"),
			                     Qt::CaseInsensitive) == 0) {
				_sexpOperatorReference = {QStringLiteral("SEXP Operator Reference"), fullPath, urlPath};
				_hasSexpOperatorReference = true;
				continue;
			}

			result.append({extractHtmlTitle(fullPath), fullPath, urlPath});
		}
	}

	// Optional fallback for developers: also accept a loose sexps.html shipped
	// next to the QtFRED executable (or in its help/ subdirectory).
	if (!_hasSexpOperatorReference) {
		const QDir appDir(QCoreApplication::applicationDirPath());
		const QStringList fallbackPaths = {
		    appDir.filePath(QStringLiteral("sexps.html")),
		    appDir.filePath(QStringLiteral("help/sexps.html"))
		};

		for (const auto& candidate : fallbackPaths) {
			QFile f(candidate);
			if (!f.open(QIODevice::ReadOnly))
				continue;

			const QString urlPath = QStringLiteral("/sexps.html");
			_tutorialContent[urlPath] = f.readAll();
			_sexpOperatorReference = {QStringLiteral("SEXP Operator Reference"), candidate, urlPath};
			_hasSexpOperatorReference = true;
			break;
		}
	}

	return result;
}

QByteArray HelpTopicsDialogModel::loadTutorialAsset(const QString& urlPath) {
	if (!cfile_inited)
		return {};

	// Reject empty, root-only, non-rooted, or path-traversal requests.
	if (urlPath.isEmpty() || urlPath == QStringLiteral("/"))
		return {};
	if (!urlPath.startsWith(QLatin1Char('/')))
		return {};
	if (urlPath.contains(QStringLiteral("..")))
		return {};

	// Cache both hits and misses to avoid repeated VFS lookups for the same asset.
	const auto cached = _assetCache.constFind(urlPath);
	if (cached != _assetCache.constEnd())
		return *cached;

	// Strip the leading '/' before the VFS lookup (e.g. "/images/logo.png" -> "images/logo.png").
	// QUrl::path() already decodes percent-encoded characters, so no extra decoding needed.
	const std::string relPath = urlPath.mid(1).toStdString();
	CFileLocation loc = cf_find_file_location(relPath.c_str(), CF_TYPE_FREDDOCS);
	if (!loc.found || loc.full_name.empty()) {
		_assetCache.insert(urlPath, {});
		return {};
	}
	QFile f(QString::fromStdString(loc.full_name));
	if (!f.open(QIODevice::ReadOnly)) {
		_assetCache.insert(urlPath, {});
		return {};
	}
	QByteArray data = f.readAll();
	_assetCache.insert(urlPath, data);
	return data;
}

} // namespace fso::fred::dialogs
