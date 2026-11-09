#include "SaveAsTemplateDialog.h"

#include "ui_SaveAsTemplateDialog.h"

#include <QPushButton>

namespace fso::fred::dialogs {

SaveAsTemplateDialog::SaveAsTemplateDialog(QWidget* parent, const SCP_string& defaultAuthor)
	: QDialog(parent), ui(new Ui::SaveAsTemplateDialog())
{
	ui->setupUi(this);

	ui->authorEdit->setText(QString::fromStdString(defaultAuthor));

	// Save button starts disabled until a title is entered
	ui->buttonBox->button(QDialogButtonBox::Save)->setEnabled(false);

	connect(ui->titleEdit, &QLineEdit::textChanged, this, &SaveAsTemplateDialog::onTitleChanged);
	connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &SaveAsTemplateDialog::onAccepted);
}

SaveAsTemplateDialog::~SaveAsTemplateDialog() = default;

MissionTemplateInfo SaveAsTemplateDialog::templateInfo() const
{
	MissionTemplateInfo info;
	info.title       = ui->titleEdit->text().trimmed().toUtf8().constData();
	info.author      = ui->authorEdit->text().trimmed().toUtf8().constData();
	info.tags        = ui->tagsEdit->text().trimmed().toUtf8().constData();
	info.description = ui->descriptionEdit->toPlainText().toUtf8().constData();
	return info;
}

void SaveAsTemplateDialog::onTitleChanged(const QString& text)
{
	ui->buttonBox->button(QDialogButtonBox::Save)->setEnabled(!text.trimmed().isEmpty());
}

void SaveAsTemplateDialog::onAccepted()
{
	accept();
}

} // namespace fso::fred::dialogs
