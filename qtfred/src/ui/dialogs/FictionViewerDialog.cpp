
// TODO sort out includes
#include <QtWidgets/QTextEdit>
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
}
FictionViewerDialog::~FictionViewerDialog() {
}

void FictionViewerDialog::reject() {
	// TODO
	// reset model to match contents of Fiction_viewer data structure?
	_model->reject();
}

void FictionViewerDialog::updateMusicComboBox() {
	QSignalBlocker blocker(ui->musicComboBox);

	// Remove all previous entries
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

	updateMusicComboBox();

	// TODO you need something like this for the text controls
	//ui->nameEdit->setText(QString::fromStdString(_model->getCurrentName()));
	// ui->nameEdit->setEnabled(_model->isEnabled());
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

// TODO figure out what to do here
bool FictionViewerDialog::event(QEvent* event) {
	switch(event->type()) {
	case QEvent::WindowDeactivate:
		// _model->apply();
		// event->accept();
		return true;
	default:
		return QDialog::event(event);
	}
}

}
}
}
