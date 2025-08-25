#include "BackgroundEditorDialog.h"
#include "ui/util/SignalBlockers.h"
#include "ui/dialogs/General/ImagePickerDialog.h"
#include "ui_BackgroundEditor.h"

#include <QMessageBox>

namespace fso::fred::dialogs {

BackgroundEditorDialog::BackgroundEditorDialog(FredView* parent, EditorViewport* viewport) : QDialog(parent), 
	ui(new Ui::BackgroundEditor()), _model(new BackgroundEditorDialogModel(this, viewport)), _viewport(viewport) {
    
	ui->setupUi(this);

	initializeUi();
	
	// Resize the dialog to the minimum size
	resize(QDialog::sizeHint());
}

BackgroundEditorDialog::~BackgroundEditorDialog() = default;

void BackgroundEditorDialog::initializeUi()
{
	util::SignalBlockers blockers(this);

	// Backgrounds
	// TODO

	// Bitmaps
	ui->bitmapPitchSpin->setRange(_model->getOrientLimit().first, _model->getOrientLimit().second);
	ui->bitmapBankSpin->setRange(_model->getOrientLimit().first, _model->getOrientLimit().second);
	ui->bitmapHeadingSpin->setRange(_model->getOrientLimit().first, _model->getOrientLimit().second);
	ui->bitmapScaleXDoubleSpinBox->setRange(_model->getBitmapScaleLimit().first, _model->getBitmapScaleLimit().second);
	ui->bitmapScaleYDoubleSpinBox->setRange(_model->getBitmapScaleLimit().first, _model->getBitmapScaleLimit().second);
	ui->bitmapDivXSpinBox->setRange(_model->getDivisionLimit().first, _model->getDivisionLimit().second);
	ui->bitmapDivYSpinBox->setRange(_model->getDivisionLimit().first, _model->getDivisionLimit().second);

	const auto& names = _model->getAvailableBitmapNames();
	for (const auto& s : names){
		ui->bitmapTypeCombo->addItem(QString::fromStdString(s));
	}

	refreshBitmapList();

	// Suns
	ui->sunPitchSpin->setRange(_model->getOrientLimit().first, _model->getOrientLimit().second);
	ui->sunHeadingSpin->setRange(_model->getOrientLimit().first, _model->getOrientLimit().second);
	ui->sunScaleDoubleSpinBox->setRange(_model->getSunScaleLimit().first, _model->getSunScaleLimit().second);

	const auto& sun_names = _model->getAvailableSunNames();
	for (const auto& s : sun_names) {
		ui->sunSelectionCombo->addItem(QString::fromStdString(s));
	}

	refreshSunList();

}

void BackgroundEditorDialog::updateUi()
{
	util::SignalBlockers blockers(this);

	refreshBitmapList();
	refreshSunList();
}

void BackgroundEditorDialog::refreshBitmapList()
{
	util::SignalBlockers blockers(this);
	
	const auto names = _model->getMissionBitmapNames();

	const int oldRow = ui->bitmapListWidget->currentRow();
	ui->bitmapListWidget->setUpdatesEnabled(false);
	ui->bitmapListWidget->clear();

	QStringList items;
	items.reserve(static_cast<int>(names.size()));
	for (const auto& s : names)
		items << QString::fromStdString(s);
	ui->bitmapListWidget->addItems(items);

	if (!items.isEmpty()) {
		const int clamped = qBound(0, oldRow, ui->bitmapListWidget->count() - 1);
		ui->bitmapListWidget->setCurrentRow(clamped);
	}

	ui->bitmapListWidget->setUpdatesEnabled(true);

	updateBitmapControls();
}

void BackgroundEditorDialog::updateBitmapControls()
{
	util::SignalBlockers blockers(this);
	
	bool enabled = (_model->getSelectedBitmapIndex() >= 0);
	
	ui->changeBitmapButton->setEnabled(enabled);
	ui->deleteBitmapButton->setEnabled(enabled);
	ui->bitmapTypeCombo->setEnabled(enabled);
	ui->bitmapPitchSpin->setEnabled(enabled);
	ui->bitmapBankSpin->setEnabled(enabled);
	ui->bitmapHeadingSpin->setEnabled(enabled);
	ui->bitmapScaleXDoubleSpinBox->setEnabled(enabled);
	ui->bitmapScaleYDoubleSpinBox->setEnabled(enabled);
	ui->bitmapDivXSpinBox->setEnabled(enabled);
	ui->bitmapDivYSpinBox->setEnabled(enabled);

	const int index = ui->bitmapTypeCombo->findText(QString::fromStdString(_model->getBitmapName()));
	ui->bitmapTypeCombo->setCurrentIndex(index);

	ui->bitmapPitchSpin->setValue(_model->getBitmapPitch());
	ui->bitmapBankSpin->setValue(_model->getBitmapBank());
	ui->bitmapHeadingSpin->setValue(_model->getBitmapHeading());
	ui->bitmapScaleXDoubleSpinBox->setValue(_model->getBitmapScaleX());
	ui->bitmapScaleYDoubleSpinBox->setValue(_model->getBitmapScaleY());
	ui->bitmapDivXSpinBox->setValue(_model->getBitmapDivX());
	ui->bitmapDivYSpinBox->setValue(_model->getBitmapDivY());
}

void BackgroundEditorDialog::refreshSunList()
{
	util::SignalBlockers blockers(this);

	const auto names = _model->getMissionSunNames();

	const int oldRow = ui->sunsListWidget->currentRow();
	ui->sunsListWidget->setUpdatesEnabled(false);
	ui->sunsListWidget->clear();

	QStringList items;
	items.reserve(static_cast<int>(names.size()));
	for (const auto& s : names)
		items << QString::fromStdString(s);
	ui->sunsListWidget->addItems(items);

	if (!items.isEmpty()) {
		const int clamped = qBound(0, oldRow, ui->sunsListWidget->count() - 1);
		ui->sunsListWidget->setCurrentRow(clamped);
	}

	ui->sunsListWidget->setUpdatesEnabled(true);

	updateSunControls();
}

void BackgroundEditorDialog::updateSunControls()
{
	util::SignalBlockers blockers(this);

	bool enabled = (_model->getSelectedSunIndex() >= 0);

	ui->changeSunButton->setEnabled(enabled);
	ui->deleteSunButton->setEnabled(enabled);
	ui->sunSelectionCombo->setEnabled(enabled);
	ui->sunPitchSpin->setEnabled(enabled);
	ui->sunHeadingSpin->setEnabled(enabled);
	ui->sunScaleDoubleSpinBox->setEnabled(enabled);

	const int index = ui->sunSelectionCombo->findText(QString::fromStdString(_model->getSunName()));
	ui->sunSelectionCombo->setCurrentIndex(index);

	ui->sunPitchSpin->setValue(_model->getSunPitch());
	ui->sunHeadingSpin->setValue(_model->getSunHeading());
	ui->sunScaleDoubleSpinBox->setValue(_model->getSunScale());
}

void BackgroundEditorDialog::on_bitmapListWidget_currentRowChanged(int row)
{
	_model->setSelectedBitmapIndex(row);
	updateBitmapControls();
}

void BackgroundEditorDialog::on_bitmapTypeCombo_currentIndexChanged(int index)
{
	if (index < 0)
		return;

	const QString text = ui->bitmapTypeCombo->itemText(index);
	_model->setBitmapName(text.toUtf8().constData());
	refreshBitmapList();
}

void BackgroundEditorDialog::on_bitmapPitchSpin_valueChanged(int arg1)
{
	_model->setBitmapPitch(arg1);
}

void BackgroundEditorDialog::on_bitmapBankSpin_valueChanged(int arg1)
{
	_model->setBitmapBank(arg1);
}

void BackgroundEditorDialog::on_bitmapHeadingSpin_valueChanged(int arg1)
{
	_model->setBitmapHeading(arg1);
}

void BackgroundEditorDialog::on_bitmapScaleXDoubleSpinBox_valueChanged(double arg1)
{
	_model->setBitmapScaleX(static_cast<float>(arg1));
}

void BackgroundEditorDialog::on_bitmapScaleYDoubleSpinBox_valueChanged(double arg1)
{
	_model->setBitmapScaleY(static_cast<float>(arg1));
}

void BackgroundEditorDialog::on_bitmapDivXSpinBox_valueChanged(int arg1)
{
	_model->setBitmapDivX(arg1);
}

void BackgroundEditorDialog::on_bitmapDivYSpinBox_valueChanged(int arg1)
{
	_model->setBitmapDivY(arg1);
}

void BackgroundEditorDialog::on_addBitmapButton_clicked()
{
	const auto files = _model->getAvailableBitmapNames();
	if (files.empty()) {
		QMessageBox::information(this, "Select Background Bitmap", "No bitmaps found.");
		return;
	}

	QStringList qnames;
	qnames.reserve(static_cast<int>(files.size()));
	for (const auto& s : files)
		qnames << QString::fromStdString(s);

	ImagePickerDialog dlg(this);
	dlg.setWindowTitle("Select Background Bitmap");
	dlg.setImageFilenames(qnames);

	if (dlg.exec() != QDialog::Accepted)
		return;

	const SCP_string chosen = dlg.selectedFile().toUtf8().constData();
	_model->addMissionBitmapByName(chosen);

	refreshBitmapList();
}

void BackgroundEditorDialog::on_changeBitmapButton_clicked()
{
	const auto files = _model->getAvailableBitmapNames();
	if (files.empty()) {
		QMessageBox::information(this, "Select Background Bitmap", "No bitmaps found.");
		return;
	}

	QStringList qnames;
	qnames.reserve(static_cast<int>(files.size()));
	for (const auto& s : files)
		qnames << QString::fromStdString(s);

	ImagePickerDialog dlg(this);
	dlg.setWindowTitle("Select Background Bitmap");
	dlg.setImageFilenames(qnames);

	// preselect current
	dlg.setInitialSelection(QString::fromStdString(_model->getBitmapName()));

	if (dlg.exec() != QDialog::Accepted)
		return;

	const SCP_string chosen = dlg.selectedFile().toUtf8().constData();
	_model->setBitmapName(chosen);

	refreshBitmapList();
}

void BackgroundEditorDialog::on_deleteBitmapButton_clicked()
{
	_model->removeMissionBitmap();
	refreshBitmapList();
}

void BackgroundEditorDialog::on_sunListWidget_currentRowChanged(int row)
{
	_model->setSelectedSunIndex(row);
	updateSunControls();
}

void BackgroundEditorDialog::on_sunSelectionCombo_currentIndexChanged(int index)
{
	if (index < 0)
		return;

	const QString text = ui->sunSelectionCombo->itemText(index);
	_model->setSunName(text.toUtf8().constData());
	refreshSunList();
}

void BackgroundEditorDialog::on_sunPitchSpin_valueChanged(int arg1)
{
	_model->setSunPitch(arg1);
}

void BackgroundEditorDialog::on_sunHeadingSpin_valueChanged(int arg1)
{
	_model->setSunHeading(arg1);
}

void BackgroundEditorDialog::on_sunScaleDoubleSpinBox_valueChanged(double arg1)
{
	_model->setSunScale(static_cast<float>(arg1));
}

void BackgroundEditorDialog::on_addSunButton_clicked()
{
	const auto files = _model->getAvailableSunNames();
	if (files.empty()) {
		QMessageBox::information(this, "Select Background Sun", "No suns found.");
		return;
	}

	QStringList qnames;
	qnames.reserve(static_cast<int>(files.size()));
	for (const auto& s : files)
		qnames << QString::fromStdString(s);

	ImagePickerDialog dlg(this);
	dlg.setWindowTitle("Select Background Sun");
	dlg.setImageFilenames(qnames);

	if (dlg.exec() != QDialog::Accepted)
		return;

	const SCP_string chosen = dlg.selectedFile().toUtf8().constData();
	_model->addMissionSunByName(chosen);

	refreshSunList();
}

void BackgroundEditorDialog::on_changeSunButton_clicked()
{
	const auto files = _model->getAvailableSunNames();
	if (files.empty()) {
		QMessageBox::information(this, "Select Background Sun", "No suns found.");
		return;
	}

	QStringList qnames;
	qnames.reserve(static_cast<int>(files.size()));
	for (const auto& s : files)
		qnames << QString::fromStdString(s);

	ImagePickerDialog dlg(this);
	dlg.setWindowTitle("Select Background Sun");
	dlg.setImageFilenames(qnames);

	// preselect current
	dlg.setInitialSelection(QString::fromStdString(_model->getSunName()));

	if (dlg.exec() != QDialog::Accepted)
		return;

	const SCP_string chosen = dlg.selectedFile().toUtf8().constData();
	_model->setSunName(chosen);

	refreshSunList();
}

void BackgroundEditorDialog::on_deleteSunButton_clicked()
{
	_model->removeMissionSun();
	refreshSunList();
}

} // namespace fso::fred::dialogs
