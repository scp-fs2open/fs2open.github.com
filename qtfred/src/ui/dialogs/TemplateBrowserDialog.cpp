#include "TemplateBrowserDialog.h"

#include "ui_TemplateBrowserDialog.h"

#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QListWidgetItem>
#include <QPushButton>
#include <QTextStream>

namespace fso::fred::dialogs {

TemplateBrowserDialog::TemplateBrowserDialog(QWidget* parent, const QString& templatesDir)
	: QDialog(parent), ui(new Ui::TemplateBrowserDialog())
{
	ui->setupUi(this);

	// Open button starts disabled until a selection is made
	ui->buttonBox->button(QDialogButtonBox::Open)->setEnabled(false);

	_templates = scanTemplates(templatesDir);
	populateList();
	clearDetails();

	connect(ui->searchEdit, &QLineEdit::textChanged, this, &TemplateBrowserDialog::onSearchChanged);
	connect(ui->templateList, &QListWidget::currentRowChanged, this, [this](int) { onSelectionChanged(); });
	connect(ui->templateList, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem*) { onItemDoubleClicked(); });
	connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &TemplateBrowserDialog::accept);
}

TemplateBrowserDialog::~TemplateBrowserDialog() = default;

QString TemplateBrowserDialog::selectedTemplatePath() const
{
	auto* current = ui->templateList->currentItem();
	if (!current)
		return {};
	int idx = current->data(Qt::UserRole).toInt();
	return _templates.at(idx).filepath;
}

void TemplateBrowserDialog::onSearchChanged(const QString& text)
{
	populateList(text);
}

void TemplateBrowserDialog::onSelectionChanged()
{
	auto* current = ui->templateList->currentItem();
	bool valid = current && (current->flags() & Qt::ItemIsSelectable);
	ui->buttonBox->button(QDialogButtonBox::Open)->setEnabled(valid);
	if (valid)
		showDetails(_templates.at(current->data(Qt::UserRole).toInt()));
	else
		clearDetails();
}

void TemplateBrowserDialog::onItemDoubleClicked()
{
	auto* current = ui->templateList->currentItem();
	if (current && (current->flags() & Qt::ItemIsSelectable))
		accept();
}

void TemplateBrowserDialog::populateList(const QString& filter)
{
	ui->templateList->clear();
	ui->buttonBox->button(QDialogButtonBox::Open)->setEnabled(false);
	clearDetails();

	const QString query = filter.trimmed();

	for (int i = 0; i < _templates.size(); ++i) {
		const auto& t = _templates.at(i);
		if (!query.isEmpty() &&
			!t.title.contains(query, Qt::CaseInsensitive) &&
			!t.author.contains(query, Qt::CaseInsensitive) &&
			!t.tags.contains(query, Qt::CaseInsensitive)) {
			continue;
		}
		auto* item = new QListWidgetItem(t.title, ui->templateList);
		item->setData(Qt::UserRole, i);
	}

	if (ui->templateList->count() == 0) {
		const QString msg = query.isEmpty() ? tr("(No templates found)") : tr("(No matches found)");
		auto* item = new QListWidgetItem(msg, ui->templateList);
		item->setFlags(item->flags() & ~Qt::ItemIsSelectable);
	}
}

void TemplateBrowserDialog::showDetails(const TemplateFileInfo& info)
{
	ui->titleValue->setText(info.title);
	ui->authorValue->setText(info.author);
	ui->tagsValue->setText(info.tags);
	ui->descriptionView->setPlainText(info.description);
}

void TemplateBrowserDialog::clearDetails()
{
	ui->titleValue->clear();
	ui->authorValue->clear();
	ui->tagsValue->clear();
	ui->descriptionView->clear();
}

TemplateFileInfo TemplateBrowserDialog::readTemplateInfo(const QString& filepath)
{
	TemplateFileInfo info;
	info.filepath = filepath;
	info.title    = QFileInfo(filepath).baseName(); // default: filename stem

	QFile file(filepath);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
		return info;

	QTextStream in(&file);
	bool inSection = false;
	bool inDesc    = false;
	QStringList descLines;

	while (!in.atEnd()) {
		const QString line    = in.readLine();
		const QString trimmed = line.trimmed();

		if (!inSection) {
			// Section is at the top; if we've reached #Mission Info it isn't here
			if (trimmed == QStringLiteral("#Mission Info"))
				break;
			if (trimmed == QStringLiteral("#Template Info"))
				inSection = true;
			continue;
		}

		if (trimmed == QStringLiteral("#End Template Info"))
			break;

		if (inDesc) {
			if (trimmed == QStringLiteral("$end_template_desc")) {
				inDesc           = false;
				info.description = descLines.join(QStringLiteral("\n"));
			} else {
				descLines << line;
			}
			continue;
		}

		if (trimmed.startsWith(QStringLiteral("$Template Title:")))
			info.title = trimmed.mid(16).trimmed();
		else if (trimmed.startsWith(QStringLiteral("$Template Author:")))
			info.author = trimmed.mid(17).trimmed();
		else if (trimmed.startsWith(QStringLiteral("$Template Tags:")))
			info.tags = trimmed.mid(15).trimmed();
		else if (trimmed == QStringLiteral("$Template Description:"))
			inDesc = true;
	}

	return info;
}

QList<TemplateFileInfo> TemplateBrowserDialog::scanTemplates(const QString& dir)
{
	QList<TemplateFileInfo> results;
	QDirIterator it(dir, QStringList() << QStringLiteral("*.fst"), QDir::Files, QDirIterator::Subdirectories);
	while (it.hasNext())
		results << readTemplateInfo(it.next());

	std::sort(results.begin(), results.end(), [](const TemplateFileInfo& a, const TemplateFileInfo& b) {
		return a.title.compare(b.title, Qt::CaseInsensitive) < 0;
	});

	return results;
}

} // namespace fso::fred::dialogs
