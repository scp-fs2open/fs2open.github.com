//
//

#include "AboutDialog.h"

#include "ui_AboutDialog.h"

#include <QtGui/QDesktopServices>
#include <QtGui/QPixmap>
#include <QtCore/QUrl>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QVBoxLayout>

namespace fso::fred::dialogs {

AboutDialog::~AboutDialog() = default;

static QString joinLines(const SCP_vector<SCP_string>& lines)
{
	QStringList parts;
	for (const auto& line : lines)
		parts << QString::fromStdString(line);
	return parts.join('\n');
}

AboutDialog::AboutDialog(QWidget* parent, EditorViewport* viewport)
	: QDialog(parent),
	  ui(new Ui::AboutDialog()),
	  _model(new AboutDialogModel(this, viewport))
{
	ui->setupUi(this);

	connect(ui->okBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

	ui->logoLabel->setPixmap(
		QPixmap(":/images/fred_about.png").scaled(128, 128, Qt::KeepAspectRatio, Qt::SmoothTransformation));

	ui->labelVersion->setText(QString::fromStdString(_model->getVersionString()));
	ui->labelCopyright->setText(QString::fromStdString(_model->getCopyrightString()));
	ui->labelQtFREDCredits->setText(joinLines(_model->getQtFREDCredits()));
	ui->labelGraphicsCredits->setText(joinLines(_model->getGraphicsCredits()));
	ui->labelQuote->setText(QString::fromStdString(_model->getQuoteString()));
}

void AboutDialog::on_scpCreditsButton_clicked()
{
	auto* dialog = new QDialog(this);
	dialog->setWindowTitle(tr("SCP Team Credits"));
	dialog->setAttribute(Qt::WA_DeleteOnClose);
	dialog->resize(500, 600);

	auto* layout = new QVBoxLayout(dialog);

	auto* textEdit = new QPlainTextEdit(dialog);
	textEdit->setReadOnly(true);
	textEdit->setPlainText(QString::fromStdString(_model->getSCPCreditsText()));
	layout->addWidget(textEdit);

	auto* buttons = new QDialogButtonBox(QDialogButtonBox::Close, dialog);
	connect(buttons, &QDialogButtonBox::rejected, dialog, &QDialog::reject);
	layout->addWidget(buttons);

	dialog->show();
}

void AboutDialog::on_reportBugButton_clicked() // NOLINT(readability-convert-member-functions-to-static)
{
	QDesktopServices::openUrl(QUrl("https://github.com/scp-fs2open/fs2open.github.com/issues", QUrl::TolerantMode));
}

void AboutDialog::on_visitForumsButton_clicked() // NOLINT(readability-convert-member-functions-to-static)
{
	QDesktopServices::openUrl(QUrl("https://www.hard-light.net/forums/", QUrl::TolerantMode));
}

void AboutDialog::on_aboutQtButton_clicked() // NOLINT(readability-convert-member-functions-to-static)
{
	QApplication::aboutQt();
}

} // namespace fso::fred::dialogs
