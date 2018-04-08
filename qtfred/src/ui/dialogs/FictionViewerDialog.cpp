
#include <QCloseEvent>
#include "ui/dialogs/FictionViewerDialog.h"

#include "ui_FictionViewerDialog.h"

namespace fso {
namespace fred {
namespace dialogs {


FictionViewerDialog::FictionViewerDialog(FredView* parent, EditorViewport* viewport) :
	QDialog(parent),
	_viewport(viewport),
	_editor(viewport->editor),
	ui(new Ui::FictionViewerDialog()),
	_model(new FictionViewerDialogModel(this, viewport)) {

	ui->setupUi(this);

	ui->storyFileEdit->setMaxLength(_model->getMaxStoryFileLength());
	ui->fontFileEdit->setMaxLength(_model->getMaxFontFileLength());
	ui->voiceFileEdit->setMaxLength(_model->getMaxVoiceFileLength());

	connect(this, &QDialog::accepted, _model.get(), &FictionViewerDialogModel::apply);
	connect(this, &QDialog::rejected, _model.get(), &FictionViewerDialogModel::reject);

	//connect(parent, &FredView::viewWindowActivated, _model.get(), &FictionViewerDialogModel::apply);
	connect(_model.get(), &AbstractDialogModel::modelChanged, this, &FictionViewerDialog::updateUI);


	connect(ui->storyFileEdit, &QLineEdit::textChanged, this, &FictionViewerDialog::storyFileTextChanged);
	connect(ui->fontFileEdit, &QLineEdit::textChanged, this, &FictionViewerDialog::fontFileTextChanged);
	connect(ui->voiceFileEdit, &QLineEdit::textChanged, this, &FictionViewerDialog::voiceFileTextChanged);

	// Initial set up of the UI
	updateUI();

	// Resize the dialog to the minimum size
	resize(QDialog::sizeHint());

	if (_model->hasMultipleStages()) {
		viewport->dialogProvider->showButtonDialog(DialogType::Information, "Multiple stages detected",
			"This mission has multiple fiction viewer stages defined.  Currently, qtFRED will only allow you to edit the first stage.",
			{ DialogButton::Ok});
	}
}
FictionViewerDialog::~FictionViewerDialog() {
}

void FictionViewerDialog::updateMusicComboBox() {
	QSignalBlocker blocker(ui->musicComboBox);

	ui->musicComboBox->clear();

	for (const auto& el : _model->getMusicOptions()) {
		ui->musicComboBox->addItem(QString::fromStdString(el.name), QVariant(el.id));
	}

	ui->musicComboBox->setEnabled(ui->musicComboBox->count() > 0);
}
void FictionViewerDialog::updateUI() {
	// We need to block signals here or else updating the combobox would lead to and update of the model
	// which would call this function again
	QSignalBlocker blocker(this);
	QSignalBlocker storyBlocker(ui->storyFileEdit);
	QSignalBlocker fontBlocker(ui->fontFileEdit);
	QSignalBlocker voiceBlocker(ui->voiceFileEdit);

	updateMusicComboBox();

	ui->storyFileEdit->setText(QString::fromStdString(_model->getStoryFile()));
	ui->fontFileEdit->setText(QString::fromStdString(_model->getFontFile()));
	ui->voiceFileEdit->setText(QString::fromStdString(_model->getVoiceFile()));
}

void FictionViewerDialog::musicSelectionChanged(int index) {
	auto itemId = ui->musicComboBox->itemData(index).value<int>();
	_model->setFictionMusic(itemId);
}

void FictionViewerDialog::storyFileTextChanged() {
	_model->setStoryFile(ui->storyFileEdit->text().toStdString());
}

void FictionViewerDialog::fontFileTextChanged() {
	_model->setFontFile(ui->fontFileEdit->text().toStdString());
}

void FictionViewerDialog::voiceFileTextChanged() {
	_model->setVoiceFile(ui->voiceFileEdit->text().toStdString());
}

void FictionViewerDialog::closeEvent(QCloseEvent* event) {
	if (_model->query_modified()) {
		auto button = _viewport->dialogProvider->showButtonDialog(DialogType::Question, "Changes detected", "Do you want to keep your changes?",
			{ DialogButton::Yes, DialogButton::No, DialogButton::Cancel });

		if (button == DialogButton::Cancel) {
			event->ignore();
			return;
		}

		if (button == DialogButton::Yes) {
			accept();
			return;
		}
	}

	QDialog::closeEvent(event);
}

}
}
}
