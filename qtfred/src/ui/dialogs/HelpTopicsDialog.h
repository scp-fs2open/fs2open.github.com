#pragma once

#include <QDialog>
#include <QTextBrowser>
#include <memory>

class QLabel;
class QListWidget;
class QListWidgetItem;
class QHelpContentItem;
class QStandardItem;
class QStandardItemModel;
class QTreeView;

namespace fso::fred::dialogs {

namespace Ui {
class HelpTopicsDialog;
}

/**
 * @brief Custom text browser that serves qthelp:// and fredtutorial:// URLs.
 *
 * Declared here (rather than as a private inner class) so that the .ui file can
 * promote a QTextBrowser to this type via Qt Designer's custom-widget mechanism.
 */
class HelpBrowser : public QTextBrowser {
	Q_OBJECT

public:
	explicit HelpBrowser(QWidget* parent = nullptr);

protected:
	void     changeEvent(QEvent* e) override;
	QVariant loadResource(int type, const QUrl& name) override;

private:
	bool isDarkMode() const;
	static QByteArray darkModeStyleBlock();
};

// ---------------------------------------------------------------------------

class HelpTopicsDialog : public QDialog {
	Q_OBJECT

public:
	explicit HelpTopicsDialog(QWidget* parent = nullptr);
	~HelpTopicsDialog() override;

	// Call once at startup to initialise the help engine, schedule search indexing,
	// and discover mod tutorial pages... all before the user opens Help Topics.
	static void prewarm();

public slots: // NOLINT(readability-redundant-access-specifiers)
	void navigateTo(const QString& keywordId);

private:
	void loadHelpPage(const QUrl& url);
	void findInPage(bool backward = false);
	void buildContentsTab();
	void addHelpContentItem(QStandardItem* parent, QHelpContentItem* contentItem);
	QStandardItem* findContentItemByTitle(QStandardItem* root, const QString& title) const;
	void buildTutorialsTab();
	void searchTutorials(const QString& query);

	std::unique_ptr<Ui::HelpTopicsDialog> ui;

	// Widgets created dynamically (not in .ui) for the Search and Tutorials tabs.
	QLabel*      _tutorialSearchLabel   = nullptr;
	QListWidget* _tutorialSearchResults = nullptr;
	QListWidget* _tutorialsWidget       = nullptr;
	QTreeView*   _contentsTree          = nullptr;
	QStandardItemModel* _contentsModel  = nullptr;
};

} // namespace fso::fred::dialogs
