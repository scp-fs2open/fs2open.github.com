#pragma once

#include <QHash>
#include <QList>
#include <QObject>
#include <QString>

class QHelpEngine;

namespace fso::fred::dialogs {

struct TutorialEntry {
	QString title;
	QString fullPath;   // absolute path on disk
	QString urlPath;    // e.g. "/getting-started.html" — key in tutorialContent()
};

/**
 * @brief Service model for the HelpTopicsDialog.
 *
 * Manages the singleton QHelpEngine and the list of mod tutorial pages discovered
 * in data/freddocs/.  All state is static so it is shared across the application
 * lifetime regardless of how many times the dialog is opened or closed.
 *
 * Call prewarm() once at startup (after the VFS is ready) to initialise the engine
 * and kick off search indexing in the background before the user opens Help Topics.
 */
class HelpTopicsDialogModel : public QObject {
	Q_OBJECT

public:
	explicit HelpTopicsDialogModel(QObject* parent = nullptr);

	// Call once at startup... initialises the help engine, schedules full-text
	// search indexing, and discovers mod tutorial pages, all before the dialog opens.
	static void prewarm();

	static QHelpEngine*                      helpEngine();
	static const QList<TutorialEntry>&       tutorials();
	static const QHash<QString, QByteArray>& tutorialContent();
	static const TutorialEntry*              sexpOperatorReference();

	// Exposed so HelpTopicsDialog can call it directly and show an error on failure.
	static bool ensureEngineReady();

	// On-demand VFS lookup for assets (CSS, images, scripts) referenced by tutorial pages.
	static QByteArray loadTutorialAsset(const QString& urlPath);

private:
	static QString resolveCollectionFile();
	static QString resolveContentFile();
	static QString extractHtmlTitle(const QString& filePath);
	static QList<TutorialEntry> discoverTutorials();

	static QHelpEngine*               _helpEngine;
	static QList<TutorialEntry>       _tutorials;
	static QHash<QString, QByteArray> _tutorialContent;
	static QHash<QString, QByteArray> _assetCache;
	static TutorialEntry              _sexpOperatorReference;
	static bool                       _hasSexpOperatorReference;
};

} // namespace fso::fred::dialogs
