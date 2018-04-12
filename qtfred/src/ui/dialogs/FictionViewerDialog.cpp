
#include <QCloseEvent>
#include "ui/dialogs/FictionViewerDialog.h"
#include "ui/util/SignalBlockers.h"
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

	connect(_model.get(), &AbstractDialogModel::modelChanged, this, &FictionViewerDialog::updateUI);

	connect(ui->storyFileEdit, &QLineEdit::textChanged, this, &FictionViewerDialog::storyFileTextChanged);
	connect(ui->fontFileEdit, &QLineEdit::textChanged, this, &FictionViewerDialog::fontFileTextChanged);
	connect(ui->voiceFileEdit, &QLineEdit::textChanged, this, &FictionViewerDialog::voiceFileTextChanged);
	connect(ui->musicComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &FictionViewerDialog::musicSelectionChanged);

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
	ui->musicComboBox->clear();

	for (const auto& el : _model->getMusicOptions()) {
		ui->musicComboBox->addItem(QString::fromStdString(el.name), QVariant(el.id));
	}

	if (ui->musicComboBox->count() > 0) {
		ui->musicComboBox->setEnabled(true);
		int selectedIndex = -1;
		for (int i = 0; i < ui->musicComboBox->count(); ++i) {
			const int itemId = ui->musicComboBox->itemData(i).value<int>();
			if (itemId == _model->getFictionMusic()) {
				selectedIndex = i;
				break;
			}
		}
		ui->musicComboBox->setCurrentIndex(selectedIndex);
	} else {
		ui->musicComboBox->setEnabled(false);
	}
}
void FictionViewerDialog::updateUI() {
	util::SignalBlockers blockers(this);

	updateMusicComboBox();

	ui->storyFileEdit->setText(QString::fromStdString(_model->getStoryFile()));
	ui->fontFileEdit->setText(QString::fromStdString(_model->getFontFile()));
	ui->voiceFileEdit->setText(QString::fromStdString(_model->getVoiceFile()));
}

void FictionViewerDialog::musicSelectionChanged(int index) {
	if (index >= 0) {
		int itemId = ui->musicComboBox->itemData(index).value<int>();
		_model->setFictionMusic(itemId);
	}
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

void FictionViewerDialog::keyPressEvent(QKeyEvent* event) {
	if (event->key() == Qt::Key_Escape) {
		// Instead of calling reject when we close a dialog it should try to close the window which will will allow the
		// user to save unsaved changes
		event->ignore();
		this->close();
		return;
	}
	QDialog::keyPressEvent(event);
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
