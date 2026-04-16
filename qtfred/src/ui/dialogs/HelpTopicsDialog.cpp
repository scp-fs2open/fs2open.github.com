#include "HelpTopicsDialog.h"
#include "ui_HelpTopicsDialog.h"

#include <QApplication>
#include <QDesktopServices>
#include <QHelpContentWidget>
#include <QHelpEngine>
#include <QHelpIndexWidget>
#include <QHelpSearchEngine>
#include <QHelpSearchQueryWidget>
#include <QHelpSearchResultWidget>
#include <QLabel>
#include <QListWidget>
#include <QMessageBox>
#include <QSplitter>
#include <QTabWidget>
#include <QTreeView>
#include <QVBoxLayout>

#include "mission/dialogs/HelpTopicsDialogModel.h"
#include "ui/Theme.h"

namespace fso::fred::dialogs {

// ---------------------------------------------------------------------------
// HelpBrowser
// ---------------------------------------------------------------------------
HelpBrowser::HelpBrowser(QWidget* parent) : QTextBrowser(parent) {
	setOpenLinks(false);
}

void HelpBrowser::changeEvent(QEvent* e) {
	QTextBrowser::changeEvent(e);
	if (e->type() == QEvent::PaletteChange)
		reload();
}

QVariant HelpBrowser::loadResource(int type, const QUrl& name) {
	if (name.scheme() == QStringLiteral("fredtutorial")) {
		// Serve pre-loaded HTML tutorial pages.
		const auto& content = HelpTopicsDialogModel::tutorialContent();
		const auto  it      = content.constFind(name.path());
		if (it != content.constEnd()) {
			QByteArray tutorialBytes = *it;
			if (name.path().endsWith(QStringLiteral(".html")) && isDarkMode()) {
				const int insertAt = tutorialBytes.indexOf("</head>");
				if (insertAt != -1)
					tutorialBytes.insert(insertAt, darkModeStyleBlock());
			}
			return tutorialBytes;
		}
		// On-demand loading for assets (images, CSS, etc.) referenced by tutorial pages.
		// Falls through to the VFS lookup in cfile, handled in the model's resolveContentFile
		// path, so we just return empty here for unknown assets.
		return {};
	}

	if (name.scheme() == QStringLiteral("qthelp")) {
		QHelpEngine* engine = HelpTopicsDialogModel::helpEngine();
		if (engine == nullptr)
			return {};
		const QUrl canonical = engine->findFile(name);
		if (!canonical.isValid())
			return {};
		QByteArray helpBytes = engine->fileData(canonical);
		if (canonical.path().endsWith(QStringLiteral(".html")) && isDarkMode()) {
			const int insertAt = helpBytes.indexOf("</head>");
			if (insertAt != -1)
				helpBytes.insert(insertAt, darkModeStyleBlock());
		}
		return helpBytes;
	}

	return QTextBrowser::loadResource(type, name);
}

bool HelpBrowser::isDarkMode() const {
	return palette().color(QPalette::Window).lightness() < 128;
}

QByteArray HelpBrowser::darkModeStyleBlock() {
	return QByteArrayLiteral(
	    "<style>"
	    "body{background:#1e1e1e;color:#ccc}"
	    "h1{border-bottom-color:#4a90d9}"
	    "h2{border-bottom-color:#444}"
	    "a,nav.breadcrumb a{color:#6ab0f5}"
	    "code,kbd{background:#2d2d2d}"
	    "pre{background:#2d2d2d;border-color:#444}"
	    "th{background:#2a2d3a}"
	    "th,td{border-color:#444}"
	    ".note{background:#1a2a3a;border-left-color:#4a90d9}"
	    ".stub-notice{background:#2a2200;border-left-color:#c08000}"
	    "nav.breadcrumb{color:#999}"
	    "</style>");
}

// ---------------------------------------------------------------------------
// HelpTopicsDialog
// ---------------------------------------------------------------------------
HelpTopicsDialog::HelpTopicsDialog(QWidget* parent)
    : QDialog(parent), ui(new Ui::HelpTopicsDialog) {
	ui->setupUi(this);

	bindStandardIcon(ui->backButton,    QStyle::SP_ArrowLeft);
	bindStandardIcon(ui->forwardButton, QStyle::SP_ArrowRight);

	if (!HelpTopicsDialogModel::ensureEngineReady()) {
		ui->splitter->setDisabled(true);
		QMessageBox::warning(this, tr("Help error"), tr("Could not load QtFRED help content."));
		return;
	}

	QHelpEngine* engine = HelpTopicsDialogModel::helpEngine();

	ui->navigationTabs->addTab(engine->contentWidget(), tr("Contents"));
	ui->navigationTabs->addTab(engine->indexWidget(),   tr("Index"));

	// Search tab: full-text search of built-in help, plus tutorial matches below.
	{
		QHelpSearchEngine* searchEngine = engine->searchEngine();
		auto* container = new QWidget(ui->navigationTabs);
		auto* layout    = new QVBoxLayout(container);
		layout->setContentsMargins(4, 4, 4, 4);
		layout->setSpacing(4);
		layout->addWidget(searchEngine->queryWidget());
		layout->addWidget(searchEngine->resultWidget(), 1);

		_tutorialSearchLabel = new QLabel(tr("Also found in tutorials:"), container);
		_tutorialSearchLabel->setVisible(false);
		layout->addWidget(_tutorialSearchLabel);

		_tutorialSearchResults = new QListWidget(container);
		_tutorialSearchResults->setVisible(false);
		layout->addWidget(_tutorialSearchResults, 1);

		ui->navigationTabs->addTab(container, tr("Search"));

		connect(searchEngine->queryWidget(), &QHelpSearchQueryWidget::search,
		        this, [this, searchEngine]() {
			const QString query = searchEngine->queryWidget()->searchInput();
			searchEngine->search(query);
			searchTutorials(query);
		});
		connect(searchEngine->resultWidget(), &QHelpSearchResultWidget::requestShowLink,
		        this, &HelpTopicsDialog::loadHelpPage);
		connect(_tutorialSearchResults, &QListWidget::itemClicked,
		        this, [this](QListWidgetItem* item) {
			const QString urlPath = item->data(Qt::UserRole).toString();
			if (!urlPath.isEmpty())
				loadHelpPage(QUrl(QStringLiteral("fredtutorial://") + urlPath));
		});

		searchEngine->scheduleIndexDocumentation();
	}

	// Tutorials tab: only shown when mod tutorial pages exist in data/freddocs/.
	if (!HelpTopicsDialogModel::tutorials().isEmpty())
		buildTutorialsTab();

	ui->splitter->setStretchFactor(0, 1);
	ui->splitter->setStretchFactor(1, 3);
	ui->splitter->setSizes({380, 820});

	connect(ui->backButton,    &QToolButton::clicked,         ui->helpBrowser, &QTextBrowser::backward);
	connect(ui->forwardButton, &QToolButton::clicked,         ui->helpBrowser, &QTextBrowser::forward);
	connect(ui->helpBrowser,   &QTextBrowser::backwardAvailable, ui->backButton,    &QToolButton::setEnabled);
	connect(ui->helpBrowser,   &QTextBrowser::forwardAvailable,  ui->forwardButton, &QToolButton::setEnabled);

	connect(engine->contentWidget(), &QTreeView::clicked,
	        this, [this](const QModelIndex& index) {
		auto* model = qobject_cast<QHelpContentModel*>(
		    HelpTopicsDialogModel::helpEngine()->contentWidget()->model());
		if (!model) return;
		QHelpContentItem* item = model->contentItemAt(index);
		if (item && item->url().isValid())
			loadHelpPage(item->url());
	});
	connect(engine->indexWidget(), &QHelpIndexWidget::linkActivated,
	        this, &HelpTopicsDialog::loadHelpPage);
	connect(ui->helpBrowser, &QTextBrowser::anchorClicked,
	        this, &HelpTopicsDialog::loadHelpPage);

	// Load the help home page.
	const auto registeredDocuments = engine->registeredDocumentations();
	if (!registeredDocuments.isEmpty()) {
		const auto homePage = engine->findFile(
		    QUrl(QStringLiteral("qthelp://%1/doc/index.html").arg(registeredDocuments.front())));
		if (homePage.isValid())
			loadHelpPage(homePage);
	}
}

HelpTopicsDialog::~HelpTopicsDialog() = default;

// ---------------------------------------------------------------------------
void HelpTopicsDialog::prewarm() {
	HelpTopicsDialogModel::prewarm();
}

void HelpTopicsDialog::buildTutorialsTab() {
	_tutorialsWidget = new QListWidget(ui->navigationTabs);
	for (const auto& t : HelpTopicsDialogModel::tutorials()) {
		auto* item = new QListWidgetItem(t.title, _tutorialsWidget);
		item->setData(Qt::UserRole, t.urlPath);
		item->setToolTip(t.fullPath);
	}
	ui->navigationTabs->addTab(_tutorialsWidget, tr("Tutorials"));

	connect(_tutorialsWidget, &QListWidget::itemClicked, this, [this](QListWidgetItem* item) {
		const QString urlPath = item->data(Qt::UserRole).toString();
		if (!urlPath.isEmpty())
			loadHelpPage(QUrl(QStringLiteral("fredtutorial://") + urlPath));
	});
}

void HelpTopicsDialog::searchTutorials(const QString& query) {
	_tutorialSearchResults->clear();

	const QString trimmed = query.trimmed();
	if (trimmed.isEmpty() || HelpTopicsDialogModel::tutorials().isEmpty()) {
		_tutorialSearchLabel->setVisible(false);
		_tutorialSearchResults->setVisible(false);
		return;
	}

	static const QRegularExpression tagRe(QStringLiteral("<[^>]+>"));
	const QStringList terms = trimmed.split(QLatin1Char(' '), Qt::SkipEmptyParts);

	for (const auto& t : HelpTopicsDialogModel::tutorials()) {
		const auto& content = HelpTopicsDialogModel::tutorialContent();
		const auto  it      = content.constFind(t.urlPath);
		if (it == content.constEnd())
			continue;

		// Strip tags for plain-text matching.
		QString text = QString::fromUtf8(*it);
		text.remove(tagRe);
		text = text.simplified();

		// All terms must be present.
		bool allFound = true;
		int  firstPos = -1;
		for (const auto& term : qAsConst(terms)) {
			const int pos = text.indexOf(term, 0, Qt::CaseInsensitive);
			if (pos < 0) { allFound = false; break; }
			if (firstPos < 0 || pos < firstPos) firstPos = pos;
		}
		if (!allFound)
			continue;

		// Build a short context snippet around the first match.
		const int    start   = qMax(0, firstPos - 60);
		const int    len     = qMin(160, text.length() - start);
		QString      snippet = text.mid(start, len).simplified();
		if (start > 0)               snippet.prepend(QStringLiteral("\u2026 "));
		if (start + len < text.length()) snippet.append(QStringLiteral(" \u2026"));

		auto* item = new QListWidgetItem(t.title, _tutorialSearchResults);
		item->setData(Qt::UserRole, t.urlPath);
		item->setToolTip(snippet);
	}

	const bool hasResults = _tutorialSearchResults->count() > 0;
	_tutorialSearchLabel->setVisible(hasResults);
	_tutorialSearchResults->setVisible(hasResults);
}

void HelpTopicsDialog::loadHelpPage(const QUrl& url) {
	if (!url.isValid())
		return;
	// Open http/https links in the system browser rather than the help viewer.
	const QString scheme = url.scheme();
	if (scheme == QStringLiteral("http") || scheme == QStringLiteral("https")) {
		QDesktopServices::openUrl(url);
		return;
	}
	ui->helpBrowser->setSource(url);
}

void HelpTopicsDialog::navigateTo(const QString& keywordId) {
	QHelpEngine* engine = HelpTopicsDialogModel::helpEngine();
	if (engine == nullptr)
		return;
	const auto links = engine->linksForIdentifier(keywordId);
	if (!links.isEmpty())
		loadHelpPage(links.constBegin().value());
}

} // namespace fso::fred::dialogs
