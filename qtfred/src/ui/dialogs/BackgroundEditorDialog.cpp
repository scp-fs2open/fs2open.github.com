#include "BackgroundEditorDialog.h"
#include "ui/util/SignalBlockers.h"
#include "ui/dialogs/General/ImagePickerDialog.h"
#include "ui_BackgroundEditor.h"

#include <QMessageBox>
#include <QSettings>
#include <QFileDialog>
#include <QFileInfo>
#include <QInputDialog>
#include <QStringList>

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
	updateBackgroundControls();

	// Bitmaps
	ui->bitmapPitchSpin->setRange(_model->getOrientLimit().first, _model->getOrientLimit().second);
	ui->bitmapBankSpin->setRange(_model->getOrientLimit().first, _model->getOrientLimit().second);
	ui->bitmapHeadingSpin->setRange(_model->getOrientLimit().first, _model->getOrientLimit().second);
	ui->bitmapScaleXDoubleSpinBox->setRange(_model->getBitmapScaleLimit().first, _model->getBitmapScaleLimit().second);
	ui->bitmapScaleYDoubleSpinBox->setRange(_model->getBitmapScaleLimit().first, _model->getBitmapScaleLimit().second);
	ui->bitmapDivXSpinBox->setRange(_model->getDivisionLimit().first, _model->getDivisionLimit().second);
	ui->bitmapDivYSpinBox->setRange(_model->getDivisionLimit().first, _model->getDivisionLimit().second);
	ui->skyboxPitchSpin->setRange(_model->getOrientLimit().first, _model->getOrientLimit().second);
	ui->skyboxBankSpin->setRange(_model->getOrientLimit().first, _model->getOrientLimit().second);
	ui->skyboxHeadingSpin->setRange(_model->getOrientLimit().first, _model->getOrientLimit().second);

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

	// Nebula
	const auto& nebula_names = _model->getNebulaPatternNames();
	for (const auto& s : nebula_names) {
		ui->nebulaPatternCombo->addItem(QString::fromStdString(s));
	}

	const auto& lightning_names = _model->getLightningNames();
	for (const auto& s : lightning_names) {
		ui->nebulaLightningCombo->addItem(QString::fromStdString(s));
	}

	const auto& poof_names = _model->getPoofNames();
	for (const auto& s : poof_names) {
		ui->poofsListWidget->addItem(QString::fromStdString(s));
	}

	ui->fogSwatch->setFrameShape(QFrame::Box);

	updateNebulaControls();

	// Old nebula
	const auto& old_nebula_names = _model->getOldNebulaPatternOptions();
	for (const auto& s : old_nebula_names) {
		ui->oldNebulaPatternCombo->addItem(QString::fromStdString(s));
	}

	const auto& old_nebula_colors = _model->getOldNebulaColorOptions();
	for (const auto& s : old_nebula_colors) {
		ui->oldNebulaColorCombo->addItem(QString::fromStdString(s));
	}

	updateOldNebulaControls();

	// Ambient light
	ui->ambientSwatch->setMinimumSize(28, 28);
	ui->ambientSwatch->setFrameShape(QFrame::Box);

	updateAmbientLightControls();

	// Skybox
	updateSkyboxControls();

	// Misc
	ui->numStarsSlider->setRange(_model->getStarsLimit().first, _model->getStarsLimit().second);
	const auto& profiles = _model->getLightingProfileOptions();

	for (const auto& s : profiles) {
		ui->lightingProfileCombo->addItem(QString::fromStdString(s));
	}

	updateMiscControls();

}

void BackgroundEditorDialog::updateUi()
{
	util::SignalBlockers blockers(this);

	updateBackgroundControls();
	refreshBitmapList();
	refreshSunList();
	updateNebulaControls();
	updateOldNebulaControls();
	updateAmbientLightControls();
	updateSkyboxControls();
	updateMiscControls();
}

void BackgroundEditorDialog::updateBackgroundControls()
{
	util::SignalBlockers blockers(this);

	ui->backgroundSelectionCombo->clear();
	const auto names = _model->getBackgroundNames();
	for (const auto& s : names){
		ui->backgroundSelectionCombo->addItem(QString::fromStdString(s));
		ui->swapWithCombo->addItem(QString::fromStdString(s));
	}

	ui->removeButton->setEnabled(_model->getBackgroundNames().size() > 1);
	ui->backgroundSelectionCombo->setCurrentIndex(_model->getActiveBackgroundIndex());
	ui->swapWithCombo->setCurrentIndex(_model->getSwapWithIndex());
	ui->useCorrectAngleFormatCheckBox->setChecked(_model->getSaveAnglesCorrectFlag());
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

void BackgroundEditorDialog::updateNebulaControls()
{
	util::SignalBlockers blockers(this);
	
	bool enabled = _model->getFullNebulaEnabled();
	ui->rangeSpinBox->setEnabled(enabled);
	ui->nebulaPatternCombo->setEnabled(enabled);
	ui->nebulaLightningCombo->setEnabled(enabled);
	ui->poofsListWidget->setEnabled(enabled);
	ui->shipTrailsCheckBox->setEnabled(enabled);
	ui->fogNearDoubleSpinBox->setEnabled(enabled);
	ui->fogFarDoubleSpinBox->setEnabled(enabled);
	ui->displayBgsInNebulaCheckbox->setEnabled(enabled);
	ui->overrideFogPaletteCheckBox->setEnabled(enabled);

	bool override = _model->getFogPaletteOverride();
	ui->fogOverrideRedSpinBox->setEnabled(enabled && override);
	ui->fogOverrideGreenSpinBox->setEnabled(enabled && override);
	ui->fogOverrideBlueSpinBox->setEnabled(enabled && override);

	ui->fullNebulaCheckBox->setChecked(enabled);
	ui->rangeSpinBox->setValue(_model->getFullNebulaRange());
	ui->nebulaPatternCombo->setCurrentIndex(ui->nebulaPatternCombo->findText(QString::fromStdString(_model->getNebulaFullPattern())));
	ui->nebulaLightningCombo->setCurrentIndex(ui->nebulaLightningCombo->findText(QString::fromStdString(_model->getLightning())));

	const auto& selected_poofs = _model->getSelectedPoofs();
	for (auto& poof : selected_poofs) {
		auto items = ui->poofsListWidget->findItems(QString::fromStdString(poof), Qt::MatchExactly);
		for (auto* item : items) {
			item->setSelected(true);
		}
	}

	ui->shipTrailsCheckBox->setChecked(_model->getShipTrailsToggled());
	ui->fogNearDoubleSpinBox->setValue(static_cast<double>(_model->getFogNearMultiplier()));
	ui->fogFarDoubleSpinBox->setValue(static_cast<double>(_model->getFogFarMultiplier()));
	ui->displayBgsInNebulaCheckbox->setChecked(_model->getDisplayBackgroundBitmaps());
	ui->overrideFogPaletteCheckBox->setChecked(override);
	ui->fogOverrideRedSpinBox->setValue(_model->getFogR());
	ui->fogOverrideGreenSpinBox->setValue(_model->getFogG());
	ui->fogOverrideBlueSpinBox->setValue(_model->getFogB());

	updateFogSwatch();

	updateOldNebulaControls();
}

void BackgroundEditorDialog::updateFogSwatch()
{
	const int r = _model->getFogR();
	const int g = _model->getFogG();
	const int b = _model->getFogB();
	ui->fogSwatch->setStyleSheet(QString("background: rgb(%1,%2,%3);"
											 "border: 1px solid #444; border-radius: 3px;")
			.arg(r)
			.arg(g)
			.arg(b));
}

void BackgroundEditorDialog::updateOldNebulaControls()
{
	util::SignalBlockers blockers(this);
	
	const bool enabled = !_model->getFullNebulaEnabled();
	const bool old_enabled = _model->getOldNebulaPattern() != "<None>";

	ui->oldNebulaPatternCombo->setEnabled(enabled);
	ui->oldNebulaColorCombo->setEnabled(enabled && old_enabled);
	ui->oldNebulaPitchSpinBox->setEnabled(enabled && old_enabled);
	ui->oldNebulaBankSpinBox->setEnabled(enabled && old_enabled);
	ui->oldNebulaHeadingSpinBox->setEnabled(enabled && old_enabled);

	ui->oldNebulaPatternCombo->setCurrentIndex(ui->oldNebulaPatternCombo->findText(QString::fromStdString(_model->getOldNebulaPattern())));
	ui->oldNebulaColorCombo->setCurrentIndex(ui->oldNebulaColorCombo->findText(QString::fromStdString(_model->getOldNebulaColorName())));
	ui->oldNebulaPitchSpinBox->setValue(_model->getOldNebulaPitch());
	ui->oldNebulaBankSpinBox->setValue(_model->getOldNebulaBank());
	ui->oldNebulaHeadingSpinBox->setValue(_model->getOldNebulaHeading());
}

void BackgroundEditorDialog::updateAmbientLightControls()
{
	util::SignalBlockers blockers(this);
	
	const int r = _model->getAmbientR();
	const int g = _model->getAmbientG();
	const int b = _model->getAmbientB();

	ui->ambientLightRedSlider->setValue(r);
	ui->ambientLightGreenSlider->setValue(g);
	ui->ambientLightBlueSlider->setValue(b);

	QString redText = "R: " + QString::number(r);
	QString greenText = "G: " + QString::number(g);
	QString blueText = "B: " + QString::number(b);

	ui->ambientLightRedLabel->setText(redText);
	ui->ambientLightGreenLabel->setText(greenText);
	ui->ambientLightBlueLabel->setText(blueText);

	updateAmbientSwatch();
}

void BackgroundEditorDialog::updateSkyboxControls()
{
	util::SignalBlockers blockers(this);
	
	bool enabled = !_model->getSkyboxModelName().empty();

	ui->skyboxPitchSpin->setEnabled(enabled);
	ui->skyboxBankSpin->setEnabled(enabled);
	ui->skyboxHeadingSpin->setEnabled(enabled);
	ui->noLightingCheckBox->setEnabled(enabled);
	ui->transparentCheckBox->setEnabled(enabled);
	ui->forceClampCheckBox->setEnabled(enabled);
	ui->noZBufferCheckBox->setEnabled(enabled);
	ui->noCullCheckBox->setEnabled(enabled);
	ui->noGlowMapsCheckBox->setEnabled(enabled);

	ui->skyboxEdit->setText(QString::fromStdString(_model->getSkyboxModelName()));
	ui->skyboxPitchSpin->setValue(_model->getSkyboxPitch());
	ui->skyboxBankSpin->setValue(_model->getSkyboxBank());
	ui->skyboxHeadingSpin->setValue(_model->getSkyboxHeading());
	ui->noLightingCheckBox->setChecked(_model->getSkyboxNoLighting());
	ui->transparentCheckBox->setChecked(_model->getSkyboxAllTransparent());
	ui->forceClampCheckBox->setChecked(_model->getSkyboxForceClamp());
	ui->noZBufferCheckBox->setChecked(_model->getSkyboxNoZbuffer());
	ui->noCullCheckBox->setChecked(_model->getSkyboxNoCull());
	ui->noGlowMapsCheckBox->setChecked(_model->getSkyboxNoGlowmaps());
}

void BackgroundEditorDialog::updateMiscControls()
{
	util::SignalBlockers blockers(this);
	
	QString text = "Number of stars: " + QString::number(_model->getNumStars());
	ui->numStarsLabel->setText(text);
	ui->numStarsSlider->setValue(_model->getNumStars());
	ui->subspaceCheckBox->setChecked(_model->getTakesPlaceInSubspace());
	ui->envMapEdit->setText(QString::fromStdString(_model->getEnvironmentMapName()));
	ui->lightingProfileCombo->setCurrentIndex(ui->lightingProfileCombo->findText(QString::fromStdString(_model->getLightingProfileName())));
}

int BackgroundEditorDialog::pickBackgroundIndexDialog(QWidget* parent, int count, int defaultIndex)
{
	if (count <= 0)
		return -1;

	QStringList items;
	items.reserve(count);
	for (int i = 0; i < count; ++i)
		items << QObject::tr("Background %1").arg(i + 1);

	bool ok = false;
	const int start = std::clamp(defaultIndex, 0, count - 1);
	const QString sel = QInputDialog::getItem(parent,
		QObject::tr("Choose Background to Import"),
		QObject::tr("Import which background?"),
		items,
		start,
		false,
		&ok);
	if (!ok)
		return -1;
	return items.indexOf(sel);
}

void BackgroundEditorDialog::on_backgroundSelectionCombo_currentIndexChanged(int index)
{
	if (index < 0)
		return;

	_model->setActiveBackgroundIndex(index);
	updateUi();
}

void BackgroundEditorDialog::on_addButton_clicked()
{
	_model->addBackground();
	updateUi();
}

void BackgroundEditorDialog::on_removeButton_clicked()
{
	_model->removeActiveBackground();
	updateUi();
}

void BackgroundEditorDialog::on_importButton_clicked()
{
	const QString file = QFileDialog::getOpenFileName(this, "Import Backgrounds from File", QString(), "Freespace 2 Mission Files (*.fs2);;All Files (*)");
	if (file.isEmpty())
		return;
	int count = _model->getImportableBackgroundCount(file.toUtf8().constData());

	if (count <= 0) {
		QMessageBox::information(this, "Import Background", "No backgrounds found in the specified file.");
		return;
	}

	int which = pickBackgroundIndexDialog(this, count);

	if (which < 0)
		return;

	_model->importBackgroundFromMission(file.toUtf8().constData(), which);

	updateUi();
}

void BackgroundEditorDialog::on_swapWithButton_clicked()
{
	_model->swapBackgrounds();

	updateUi();
}

void BackgroundEditorDialog::on_swapWithCombo_currentIndexChanged(int index)
{
	_model->setSwapWithIndex(index);
}

void BackgroundEditorDialog::on_useCorrectAngleFormatCheckBox_toggled(bool checked)
{
	_model->setSaveAnglesCorrectFlag(checked);
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

void BackgroundEditorDialog::on_fullNebulaCheckBox_toggled(bool checked)
{
	_model->setFullNebulaEnabled(checked);
	updateNebulaControls();
}

void BackgroundEditorDialog::on_rangeSpinBox_valueChanged(int arg1)
{
	_model->setFullNebulaRange(static_cast<float>(arg1));
}

void BackgroundEditorDialog::on_nebulaPatternCombo_currentIndexChanged(int index)
{
	if (index < 0)
		return;

	const QString text = ui->nebulaPatternCombo->itemText(index);
	_model->setNebulaFullPattern(text.toUtf8().constData());
}

void BackgroundEditorDialog::on_nebulaLightningCombo_currentIndexChanged(int index)
{
	if (index < 0)
		return;

	const QString text = ui->nebulaLightningCombo->itemText(index);
	_model->setLightning(text.toUtf8().constData());
}

void BackgroundEditorDialog::on_poofsListWidget_itemSelectionChanged()
{
	QStringList selected;
	for (auto* item : ui->poofsListWidget->selectedItems()) {
		selected << item->text();
	}
	SCP_vector<SCP_string> selected_std;
	selected_std.reserve(static_cast<size_t>(selected.size()));
	for (const auto& s : selected) {
		selected_std.emplace_back(s.toUtf8().constData());
	}
	_model->setSelectedPoofs(selected_std);
}

void BackgroundEditorDialog::on_shipTrailsCheckBox_toggled(bool checked)
{
	_model->setShipTrailsToggled(checked);
}

void BackgroundEditorDialog::on_fogNearDoubleSpinBox_valueChanged(double arg1)
{
	_model->setFogNearMultiplier(static_cast<float>(arg1));
}

void BackgroundEditorDialog::on_fogFarDoubleSpinBox_valueChanged(double arg1)
{
	_model->setFogFarMultiplier(static_cast<float>(arg1));
}

void BackgroundEditorDialog::on_displayBgsInNebulaCheckbox_toggled(bool checked)
{
	_model->setDisplayBackgroundBitmaps(checked);
}

void BackgroundEditorDialog::on_overrideFogPaletteCheckBox_toggled(bool checked)
{
	_model->setFogPaletteOverride(checked);
	updateNebulaControls();
}

void BackgroundEditorDialog::on_fogOverrideRedSpinBox_valueChanged(int arg1)
{
	_model->setFogR(arg1);
	updateFogSwatch();
}

void BackgroundEditorDialog::on_fogOverrideGreenSpinBox_valueChanged(int arg1)
{
	_model->setFogG(arg1);
	updateFogSwatch();
}

void BackgroundEditorDialog::on_fogOverrideBlueSpinBox_valueChanged(int arg1)
{
	_model->setFogB(arg1);
	updateFogSwatch();
}

void BackgroundEditorDialog::on_oldNebulaPatternCombo_currentIndexChanged(int index)
{
	if (index < 0)
		return;
	const QString text = ui->oldNebulaPatternCombo->itemText(index);
	_model->setOldNebulaPattern(text.toUtf8().constData());
	updateOldNebulaControls();
}

void BackgroundEditorDialog::on_oldNebulaColorCombo_currentIndexChanged(int index)
{
	if (index < 0)
		return;
	const QString text = ui->oldNebulaColorCombo->itemText(index);
	_model->setOldNebulaColorName(text.toUtf8().constData());
}

void BackgroundEditorDialog::on_oldNebulaPitchSpinBox_valueChanged(int arg1)
{
	_model->setOldNebulaPitch(arg1);
}

void BackgroundEditorDialog::on_oldNebulaBankSpinBox_valueChanged(int arg1)
{
	_model->setOldNebulaBank(arg1);
}

void BackgroundEditorDialog::on_oldNebulaHeadingSpinBox_valueChanged(int arg1)
{
	_model->setOldNebulaHeading(arg1);
}

void BackgroundEditorDialog::on_ambientLightRedSlider_valueChanged(int value)
{
	_model->setAmbientR(value);
	
	QString text = "R: " + QString::number(value);
	ui->ambientLightRedLabel->setText(text);
	updateAmbientSwatch();
}

void BackgroundEditorDialog::on_ambientLightGreenSlider_valueChanged(int value)
{
	_model->setAmbientG(value);
	
	QString text = "G: " + QString::number(value);
	ui->ambientLightGreenLabel->setText(text);
	updateAmbientSwatch();
}

void BackgroundEditorDialog::on_ambientLightBlueSlider_valueChanged(int value)
{
	_model->setAmbientB(value);

	QString text = "B: " + QString::number(value);
	ui->ambientLightBlueLabel->setText(text);
	updateAmbientSwatch();
}

void BackgroundEditorDialog::updateAmbientSwatch()
{
	const int r = _model->getAmbientR();
	const int g = _model->getAmbientG();
	const int b = _model->getAmbientB();
	ui->ambientSwatch->setStyleSheet(QString("background: rgb(%1,%2,%3);"
											 "border: 1px solid #444; border-radius: 3px;")
			.arg(r)
			.arg(g)
			.arg(b));
}

void BackgroundEditorDialog::on_skyboxModelButton_clicked()
{
	QSettings settings("QtFRED", "BackgroundEditor");
	const QString lastDir = settings.value("skybox/lastDir", QDir::homePath()).toString();

	const QString path =
		QFileDialog::getOpenFileName(this, tr("Select Skybox Model"), lastDir, tr("FS2 Models (*.pof);;All Files (*)"));
	if (path.isEmpty())
		return;

	const QFileInfo fi(path);
	settings.setValue("skybox/lastDir", fi.absolutePath());

	const QString baseName = fi.completeBaseName();
	_model->setSkyboxModelName(baseName.toUtf8().constData());

	updateSkyboxControls();
}

void BackgroundEditorDialog::on_skyboxEdit_textChanged(const QString& arg1)
{
	_model->setSkyboxModelName(arg1.toUtf8().constData());
	updateSkyboxControls();
}

void BackgroundEditorDialog::on_skyboxPitchSpin_valueChanged(int arg1)
{
	_model->setSkyboxPitch(arg1);
}

void BackgroundEditorDialog::on_skyboxBankSpin_valueChanged(int arg1)
{
	_model->setSkyboxBank(arg1);
}

void BackgroundEditorDialog::on_skyboxHeadingSpin_valueChanged(int arg1)
{
	_model->setSkyboxHeading(arg1);
}

void BackgroundEditorDialog::on_skyboxNoLightingCheckBox_toggled(bool checked)
{
	_model->setSkyboxNoLighting(checked);
}

void BackgroundEditorDialog::on_noLightingCheckBox_toggled(bool checked)
{
	_model->setSkyboxNoLighting(checked);
}

void BackgroundEditorDialog::on_transparentCheckBox_toggled(bool checked)
{
	_model->setSkyboxAllTransparent(checked);
}

void BackgroundEditorDialog::on_forceClampCheckBox_toggled(bool checked)
{
	_model->setSkyboxForceClamp(checked);
}

void BackgroundEditorDialog::on_noZBufferCheckBox_toggled(bool checked)
{
	_model->setSkyboxNoZbuffer(checked);
}

void BackgroundEditorDialog::on_noCullCheckBox_toggled(bool checked)
{
	_model->setSkyboxNoCull(checked);
}

void BackgroundEditorDialog::on_noGlowmapsCheckBox_toggled(bool checked)
{
	_model->setSkyboxNoGlowmaps(checked);
}

void BackgroundEditorDialog::on_numStarsSlider_valueChanged(int value)
{
	_model->setNumStars(value);

	QString text = "Number of stars: " + QString::number(value);
	ui->numStarsLabel->setText(text);
}

void BackgroundEditorDialog::on_subspaceCheckBox_toggled(bool checked)
{
	_model->setTakesPlaceInSubspace(checked);
}

void BackgroundEditorDialog::on_envMapButton_clicked()
{
	QSettings settings("QtFRED", "BackgroundEditor");
	const QString lastDir = settings.value("envmap/lastDir", QDir::homePath()).toString();
	const QString path = QFileDialog::getOpenFileName(this,
		tr("Select Environment Map"),
		lastDir,
		tr("Environment Maps (*.dds);;All Files (*)"));
	if (path.isEmpty())
		return;
	const QFileInfo fi(path);
	settings.setValue("envmap/lastDir", fi.absolutePath());
	const QString baseName = fi.completeBaseName();
	_model->setEnvironmentMapName(baseName.toUtf8().constData());
	updateMiscControls();
}

void BackgroundEditorDialog::on_envMapEdit_textChanged(const QString& arg1)
{
	_model->setEnvironmentMapName(arg1.toUtf8().constData());
}

void BackgroundEditorDialog::on_lightingProfileCombo_currentIndexChanged(int index)
{
	if (index < 0)
		return;

	const QString text = ui->lightingProfileCombo->itemText(index);
	_model->setLightingProfileName(text.toUtf8().constData());
}

} // namespace fso::fred::dialogs
