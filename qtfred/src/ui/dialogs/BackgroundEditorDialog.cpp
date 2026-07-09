#include "BackgroundEditorDialog.h"
#include <ui/util/DialogUndo.h>
#include <QCloseEvent>
#include "ui/util/default_dir.h"
#include "ui/util/SignalBlockers.h"
#include "ui/dialogs/General/ImagePickerDialog.h"
#include "ui_BackgroundEditor.h"

#include <globalincs/globals.h>
#include <mission/commands/FredCommands.h>
#include <QMessageBox>
#include <QFileDialog>
#include <QPointer>
#include <QToolButton>
#include <QFileInfo>
#include <QInputDialog>
#include <QStringList>

namespace fso::fred::dialogs {

namespace {

class BackgroundEditCommand : public QUndoCommand {
	// The command lives on the main stack and outlives the dialog, so it
	// restores through the model's static path and only uses the (guarded)
	// model pointer to resync the dialog while it is still open.
	QPointer<BackgroundEditorDialogModel> _model;
	Editor* _editor;
	QByteArray _before, _after;
	int _fieldId;
	bool _skipFirstRedo;

	void apply(const QByteArray& data) {
		const auto selection = BackgroundEditorDialogModel::restoreGlobalState(data, _editor);
		if (_model) {
			_model->applyRestoredSelection(selection.first, selection.second);
		}
	}

public:
	BackgroundEditCommand(BackgroundEditorDialogModel* model, Editor* editor,
	                      QByteArray before, QByteArray after,
	                      int fieldId, const QString& text,
	                      bool skipFirstRedo = true)
		: QUndoCommand(text), _model(model), _editor(editor), _before(std::move(before)),
		  _after(std::move(after)), _fieldId(fieldId), _skipFirstRedo(skipFirstRedo) {}

	void undo() override { apply(_before); }
	void redo() override {
		if (_skipFirstRedo) { _skipFirstRedo = false; return; }
		apply(_after);
	}

	bool mergeWith(const QUndoCommand* other) override {
		if (_fieldId < 0) return false;
		if (typeid(*other) != typeid(*this)) return false;
		const auto* o = static_cast<const BackgroundEditCommand*>(other);
		if (o->_fieldId != _fieldId) return false;
		_after = o->_after;
		return true;
	}

	int id() const override { return _fieldId; }
};

} // anonymous namespace

// Helper macro to reduce boilerplate for simple setter slots
#define BG_PUSH(fieldId, setter, text) \
	do { \
		const QByteArray before = _model->captureState(); \
		setter; \
		const QByteArray after = _model->captureState(); \
		if (before != after) \
			_fredView->mainUndoStack()->push( \
				new BackgroundEditCommand(_model.get(), _viewport->editor, before, after, FieldId::fieldId, tr(text))); \
	} while (false)

BackgroundEditorDialog::BackgroundEditorDialog(FredView* parent, EditorViewport* viewport) : QDialog(parent),
	ui(new Ui::BackgroundEditor()), _model(new BackgroundEditorDialogModel(this, viewport)),
	_viewport(viewport), _fredView(parent) {


	ui->setupUi(this);
	util::installMainStackUndoShortcuts(this, _fredView->mainUndoStack());

	ui->skyboxEdit->setMaxLength(MAX_FILENAME_LEN - 1);
	ui->envMapEdit->setMaxLength(MAX_FILENAME_LEN - 1);

	connect(_model.get(), &BackgroundEditorDialogModel::modelDataChanged, this, [this]() {
		updateUi();
	});

	initializeUi();

	// Resize the dialog to the minimum size
	resize(QDialog::sizeHint());
}

BackgroundEditorDialog::~BackgroundEditorDialog() = default;

void BackgroundEditorDialog::changeEvent(QEvent* e)
{
	if (e->type() == QEvent::ActivationChange && isActiveWindow())
		_fredView->undoGroup()->setActiveStack(_fredView->mainUndoStack());
	QDialog::changeEvent(e);
}

void BackgroundEditorDialog::closeEvent(QCloseEvent* e)
{
	_model->finalizeFogChanges();
	QDialog::closeEvent(e);
}

void BackgroundEditorDialog::initializeUi()
{
	util::SignalBlockers blockers(this);

	// Backgrounds
	updateBackgroundControls();

	// Bitmaps
	ui->bitmapPitchSpin->setRange(_model->getFloatOrientLimit().first, _model->getFloatOrientLimit().second);
	ui->bitmapBankSpin->setRange(_model->getFloatOrientLimit().first, _model->getFloatOrientLimit().second);
	ui->bitmapHeadingSpin->setRange(_model->getFloatOrientLimit().first, _model->getFloatOrientLimit().second);
	ui->bitmapScaleXDoubleSpinBox->setRange(_model->getBitmapScaleLimit().first, _model->getBitmapScaleLimit().second);
	ui->bitmapScaleYDoubleSpinBox->setRange(_model->getBitmapScaleLimit().first, _model->getBitmapScaleLimit().second);
	ui->bitmapDivXSpinBox->setRange(_model->getDivisionLimit().first, _model->getDivisionLimit().second);
	ui->bitmapDivYSpinBox->setRange(_model->getDivisionLimit().first, _model->getDivisionLimit().second);
	ui->skyboxPitchSpin->setRange(_model->getFloatOrientLimit().first, _model->getFloatOrientLimit().second);
	ui->skyboxBankSpin->setRange(_model->getFloatOrientLimit().first, _model->getFloatOrientLimit().second);
	ui->skyboxHeadingSpin->setRange(_model->getFloatOrientLimit().first, _model->getFloatOrientLimit().second);

	const auto& names = _model->getAvailableBitmapNames();

	for (const auto& s : names){
		ui->bitmapTypeCombo->addItem(QString::fromStdString(s));
	}

	refreshBitmapList();

	// Suns
	ui->sunPitchSpin->setRange(_model->getFloatOrientLimit().first, _model->getFloatOrientLimit().second);
	ui->sunHeadingSpin->setRange(_model->getFloatOrientLimit().first, _model->getFloatOrientLimit().second);
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

	// Old nebula — legacy FS1 system, collapsed by default
	connect(ui->legacyNebulaToggle, &QToolButton::toggled, this, [this](bool expanded) {
		ui->legacyNebulaToggle->setArrowType(expanded ? Qt::DownArrow : Qt::RightArrow);
		ui->oldNebulaGroupBox->setVisible(expanded);
		adjustSize();
	});

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
	ui->swapWithCombo->clear();
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

	const int targetRow = _model->getSelectedBitmapIndex();
	ui->bitmapListWidget->setUpdatesEnabled(false);
	ui->bitmapListWidget->clear();

	QStringList items;
	items.reserve(static_cast<int>(names.size()));
	for (const auto& s : names)
		items << QString::fromStdString(s);
	ui->bitmapListWidget->addItems(items);

	if (!items.isEmpty()) {
		const int clamped = qBound(0, targetRow, ui->bitmapListWidget->count() - 1);
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

	const int targetRow = _model->getSelectedSunIndex();
	ui->sunListWidget->setUpdatesEnabled(false);
	ui->sunListWidget->clear();

	QStringList items;
	items.reserve(static_cast<int>(names.size()));
	for (const auto& s : names)
		items << QString::fromStdString(s);
	ui->sunListWidget->addItems(items);

	if (!items.isEmpty()) {
		const int clamped = qBound(0, targetRow, ui->sunListWidget->count() - 1);
		ui->sunListWidget->setCurrentRow(clamped);
	}

	ui->sunListWidget->setUpdatesEnabled(true);

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
	ui->fog1000mVisDoubleSpinBox->setEnabled(enabled);
	ui->fogNearDistanceDoubleSpinBox->setEnabled(enabled);
	ui->fogSkyboxClipDoubleSpinBox->setEnabled(enabled);
	ui->fogClipDoubleSpinBox->setEnabled(enabled);
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

	ui->poofsListWidget->clearSelection();
	const auto& selected_poofs = _model->getSelectedPoofs();
	for (auto& poof : selected_poofs) {
		auto items = ui->poofsListWidget->findItems(QString::fromStdString(poof), Qt::MatchExactly);
		for (auto* item : items) {
			item->setSelected(true);
		}
	}

	ui->shipTrailsCheckBox->setChecked(_model->getShipTrailsToggled());
	ui->fog1000mVisDoubleSpinBox->setValue(static_cast<double>(_model->getFog1000mVisibility()));
	ui->fogNearDistanceDoubleSpinBox->setValue(static_cast<double>(_model->getFogNearDistance()));
	ui->fogSkyboxClipDoubleSpinBox->setValue(static_cast<double>(_model->getFogSkyboxClipDistance()));
	ui->fogClipDoubleSpinBox->setValue(static_cast<double>(_model->getFogClipDistance()));
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

	const bool fullNeb = _model->getFullNebulaEnabled();
	const bool hasLegacy = !fullNeb && _model->getOldNebulaPattern() != "<None>";
	const bool patternSet = _model->getOldNebulaPattern() != "<None>";

	// Drive toggle button and group box visibility from the model.
	// SignalBlockers prevents toggled from re-entering; sync arrow type manually.
	ui->legacyNebulaToggle->setChecked(hasLegacy);
	ui->legacyNebulaToggle->setArrowType(hasLegacy ? Qt::DownArrow : Qt::RightArrow);
	ui->oldNebulaGroupBox->setVisible(hasLegacy);

	// Always apply enabled states — controls must be correct if the user manually
	// expanded the section while pattern is still <None>.
	ui->oldNebulaPatternCombo->setEnabled(!fullNeb);
	ui->oldNebulaColorCombo->setEnabled(!fullNeb && patternSet);
	ui->oldNebulaPitchSpinBox->setEnabled(!fullNeb && patternSet);
	ui->oldNebulaBankSpinBox->setEnabled(!fullNeb && patternSet);
	ui->oldNebulaHeadingSpinBox->setEnabled(!fullNeb && patternSet);

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

// ---- Backgrounds (structural — fieldId=-1, no merging) ----

void BackgroundEditorDialog::on_backgroundSelectionCombo_currentIndexChanged(int index)
{
	if (index < 0)
		return;

	_model->setActiveBackgroundIndex(index);
	updateUi();
}

void BackgroundEditorDialog::on_addButton_clicked()
{
	const QByteArray before = _model->captureState();
	_model->addBackground();
	const QByteArray after = _model->captureState();
	if (before != after)
		_fredView->mainUndoStack()->push(
			new BackgroundEditCommand(_model.get(), _viewport->editor, before, after, -1, tr("Add Background")));
	updateUi();
}

void BackgroundEditorDialog::on_removeButton_clicked()
{
	const QByteArray before = _model->captureState();
	_model->removeActiveBackground();
	const QByteArray after = _model->captureState();
	if (before != after)
		_fredView->mainUndoStack()->push(
			new BackgroundEditCommand(_model.get(), _viewport->editor, before, after, -1, tr("Remove Background")));
	updateUi();
}

void BackgroundEditorDialog::on_importButton_clicked()
{
	const QString importLastDir = util::getLastDir("background/importBackgrounds", CF_TYPE_MISSIONS);

	const QString file = QFileDialog::getOpenFileName(this, "Import Backgrounds from File", importLastDir, "Freespace 2 Mission Files (*.fs2);;All Files (*)");
	if (file.isEmpty())
		return;
	util::saveLastDir("background/importBackgrounds", file);
	int count = _model->getImportableBackgroundCount(file.toUtf8().constData());

	if (count <= 0) {
		QMessageBox::information(this, "Import Background", "No backgrounds found in the specified file.");
		return;
	}

	int which = pickBackgroundIndexDialog(this, count);

	if (which < 0)
		return;

	const QByteArray before = _model->captureState();
	_model->importBackgroundFromMission(file.toUtf8().constData(), which);
	const QByteArray after = _model->captureState();
	if (before != after)
		_fredView->mainUndoStack()->push(
			new BackgroundEditCommand(_model.get(), _viewport->editor, before, after, -1, tr("Import Background")));

	updateUi();
}

void BackgroundEditorDialog::on_swapWithButton_clicked()
{
	const QByteArray before = _model->captureState();
	_model->swapBackgrounds();
	const QByteArray after = _model->captureState();
	if (before != after)
		_fredView->mainUndoStack()->push(
			new BackgroundEditCommand(_model.get(), _viewport->editor, before, after, -1, tr("Swap Backgrounds")));

	updateUi();
}

void BackgroundEditorDialog::on_swapWithCombo_currentIndexChanged(int index)
{
	// Navigation only — no undo entry
	_model->setSwapWithIndex(index);
}

void BackgroundEditorDialog::on_useCorrectAngleFormatCheckBox_toggled(bool checked)
{
	BG_PUSH(BG_AngleFormat, _model->setSaveAnglesCorrectFlag(checked), "Change Angle Format");
}

// ---- Bitmaps ----

void BackgroundEditorDialog::on_bitmapListWidget_currentRowChanged(int row)
{
	// Navigation only — no undo entry
	_model->setSelectedBitmapIndex(row);
	updateBitmapControls();
}

void BackgroundEditorDialog::on_bitmapTypeCombo_currentIndexChanged(int index)
{
	if (index < 0)
		return;

	const QString text = ui->bitmapTypeCombo->itemText(index);
	BG_PUSH(BG_BitmapName, _model->setBitmapName(text.toUtf8().constData()), "Change Bitmap");
	refreshBitmapList();
}

void BackgroundEditorDialog::on_bitmapPitchSpin_valueChanged(double arg1)
{
	BG_PUSH(BG_BitmapPitch, _model->setBitmapPitch(static_cast<float>(arg1)), "Change Bitmap Pitch");
}

void BackgroundEditorDialog::on_bitmapBankSpin_valueChanged(double arg1)
{
	BG_PUSH(BG_BitmapBank, _model->setBitmapBank(static_cast<float>(arg1)), "Change Bitmap Bank");
}

void BackgroundEditorDialog::on_bitmapHeadingSpin_valueChanged(double arg1)
{
	BG_PUSH(BG_BitmapHeading, _model->setBitmapHeading(static_cast<float>(arg1)), "Change Bitmap Heading");
}

void BackgroundEditorDialog::on_bitmapScaleXDoubleSpinBox_valueChanged(double arg1)
{
	BG_PUSH(BG_BitmapScaleX, _model->setBitmapScaleX(static_cast<float>(arg1)), "Change Bitmap Scale X");
}

void BackgroundEditorDialog::on_bitmapScaleYDoubleSpinBox_valueChanged(double arg1)
{
	BG_PUSH(BG_BitmapScaleY, _model->setBitmapScaleY(static_cast<float>(arg1)), "Change Bitmap Scale Y");
}

void BackgroundEditorDialog::on_bitmapDivXSpinBox_valueChanged(int arg1)
{
	BG_PUSH(BG_BitmapDivX, _model->setBitmapDivX(arg1), "Change Bitmap Divisions X");
}

void BackgroundEditorDialog::on_bitmapDivYSpinBox_valueChanged(int arg1)
{
	BG_PUSH(BG_BitmapDivY, _model->setBitmapDivY(arg1), "Change Bitmap Divisions Y");
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
	const QByteArray before = _model->captureState();
	_model->addMissionBitmapByName(chosen);
	const QByteArray after = _model->captureState();
	if (before != after)
		_fredView->mainUndoStack()->push(
			new BackgroundEditCommand(_model.get(), _viewport->editor, before, after, -1, tr("Add Bitmap")));

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
	BG_PUSH(BG_BitmapName, _model->setBitmapName(chosen), "Change Bitmap");

	refreshBitmapList();
}

void BackgroundEditorDialog::on_deleteBitmapButton_clicked()
{
	const QByteArray before = _model->captureState();
	_model->removeMissionBitmap();
	const QByteArray after = _model->captureState();
	if (before != after)
		_fredView->mainUndoStack()->push(
			new BackgroundEditCommand(_model.get(), _viewport->editor, before, after, -1, tr("Delete Bitmap")));
	refreshBitmapList();
}

// ---- Suns ----

void BackgroundEditorDialog::on_sunListWidget_currentRowChanged(int row)
{
	// Navigation only — no undo entry
	_model->setSelectedSunIndex(row);
	updateSunControls();
}

void BackgroundEditorDialog::on_sunSelectionCombo_currentIndexChanged(int index)
{
	if (index < 0)
		return;

	const QString text = ui->sunSelectionCombo->itemText(index);
	BG_PUSH(BG_SunName, _model->setSunName(text.toUtf8().constData()), "Change Sun");
	refreshSunList();
}

void BackgroundEditorDialog::on_sunPitchSpin_valueChanged(double arg1)
{
	BG_PUSH(BG_SunPitch, _model->setSunPitch(static_cast<float>(arg1)), "Change Sun Pitch");
}

void BackgroundEditorDialog::on_sunHeadingSpin_valueChanged(double arg1)
{
	BG_PUSH(BG_SunHeading, _model->setSunHeading(static_cast<float>(arg1)), "Change Sun Heading");
}

void BackgroundEditorDialog::on_sunScaleDoubleSpinBox_valueChanged(double arg1)
{
	BG_PUSH(BG_SunScale, _model->setSunScale(static_cast<float>(arg1)), "Change Sun Scale");
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
	const QByteArray before = _model->captureState();
	_model->addMissionSunByName(chosen);
	const QByteArray after = _model->captureState();
	if (before != after)
		_fredView->mainUndoStack()->push(
			new BackgroundEditCommand(_model.get(), _viewport->editor, before, after, -1, tr("Add Sun")));

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
	BG_PUSH(BG_SunName, _model->setSunName(chosen), "Change Sun");

	refreshSunList();
}

void BackgroundEditorDialog::on_deleteSunButton_clicked()
{
	const QByteArray before = _model->captureState();
	_model->removeMissionSun();
	const QByteArray after = _model->captureState();
	if (before != after)
		_fredView->mainUndoStack()->push(
			new BackgroundEditCommand(_model.get(), _viewport->editor, before, after, -1, tr("Delete Sun")));
	refreshSunList();
}

// ---- Nebula ----

void BackgroundEditorDialog::on_fullNebulaCheckBox_toggled(bool checked)
{
	BG_PUSH(BG_FullNebula, _model->setFullNebulaEnabled(checked), "Toggle Full Nebula");
	updateNebulaControls();
}

void BackgroundEditorDialog::on_rangeSpinBox_valueChanged(int arg1)
{
	BG_PUSH(BG_NebulaRange, _model->setFullNebulaRange(static_cast<float>(arg1)), "Change Nebula Range");
}

void BackgroundEditorDialog::on_nebulaPatternCombo_currentIndexChanged(int index)
{
	if (index < 0)
		return;

	const QString text = ui->nebulaPatternCombo->itemText(index);
	BG_PUSH(BG_NebulaPattern, _model->setNebulaFullPattern(text.toUtf8().constData()), "Change Nebula Pattern");
}

void BackgroundEditorDialog::on_nebulaLightningCombo_currentIndexChanged(int index)
{
	if (index < 0)
		return;

	const QString text = ui->nebulaLightningCombo->itemText(index);
	BG_PUSH(BG_Lightning, _model->setLightning(text.toUtf8().constData()), "Change Lightning");
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
	BG_PUSH(BG_Poofs, _model->setSelectedPoofs(selected_std), "Change Nebula Poofs");
}

void BackgroundEditorDialog::on_shipTrailsCheckBox_toggled(bool checked)
{
	BG_PUSH(BG_ShipTrails, _model->setShipTrailsToggled(checked), "Toggle Ship Trails");
}

void BackgroundEditorDialog::on_fog1000mVisDoubleSpinBox_valueChanged(double arg1)
{
	BG_PUSH(BG_Fog1000m, _model->setFog1000mVisibility(static_cast<float>(arg1)), "Change Fog 1000m Visibility");
}

void BackgroundEditorDialog::on_fogNearDistanceDoubleSpinBox_valueChanged(double arg1)
{
	BG_PUSH(BG_FogNear, _model->setFogNearDistance(static_cast<float>(arg1)), "Change Fog Near Distance");
}

void BackgroundEditorDialog::on_fogSkyboxClipDoubleSpinBox_valueChanged(double arg1)
{
	BG_PUSH(BG_FogSkybox, _model->setFogSkyboxClipDistance(static_cast<float>(arg1)), "Change Fog Skybox Clip");
}

void BackgroundEditorDialog::on_fogClipDoubleSpinBox_valueChanged(double arg1)
{
	BG_PUSH(BG_FogClip, _model->setFogClipDistance(static_cast<float>(arg1)), "Change Fog Clip Distance");
}

void BackgroundEditorDialog::on_displayBgsInNebulaCheckbox_toggled(bool checked)
{
	BG_PUSH(BG_DisplayBgInNeb, _model->setDisplayBackgroundBitmaps(checked), "Toggle Display Backgrounds in Nebula");
}

void BackgroundEditorDialog::on_overrideFogPaletteCheckBox_toggled(bool checked)
{
	BG_PUSH(BG_FogOverride, _model->setFogPaletteOverride(checked), "Toggle Fog Palette Override");
	updateNebulaControls();
}

void BackgroundEditorDialog::on_fogOverrideRedSpinBox_valueChanged(int arg1)
{
	BG_PUSH(BG_FogR, _model->setFogR(arg1), "Change Fog Red");
	updateFogSwatch();
}

void BackgroundEditorDialog::on_fogOverrideGreenSpinBox_valueChanged(int arg1)
{
	BG_PUSH(BG_FogG, _model->setFogG(arg1), "Change Fog Green");
	updateFogSwatch();
}

void BackgroundEditorDialog::on_fogOverrideBlueSpinBox_valueChanged(int arg1)
{
	BG_PUSH(BG_FogB, _model->setFogB(arg1), "Change Fog Blue");
	updateFogSwatch();
}

// ---- Old Nebula ----

void BackgroundEditorDialog::on_oldNebulaPatternCombo_currentIndexChanged(int index)
{
	if (index < 0)
		return;
	const QString text = ui->oldNebulaPatternCombo->itemText(index);
	BG_PUSH(BG_OldNebPattern, _model->setOldNebulaPattern(text.toUtf8().constData()), "Change Old Nebula Pattern");
	updateOldNebulaControls();
}

void BackgroundEditorDialog::on_oldNebulaColorCombo_currentIndexChanged(int index)
{
	if (index < 0)
		return;
	const QString text = ui->oldNebulaColorCombo->itemText(index);
	BG_PUSH(BG_OldNebColor, _model->setOldNebulaColorName(text.toUtf8().constData()), "Change Old Nebula Color");
}

void BackgroundEditorDialog::on_oldNebulaPitchSpinBox_valueChanged(int arg1)
{
	BG_PUSH(BG_OldNebPitch, _model->setOldNebulaPitch(arg1), "Change Old Nebula Pitch");
}

void BackgroundEditorDialog::on_oldNebulaBankSpinBox_valueChanged(int arg1)
{
	BG_PUSH(BG_OldNebBank, _model->setOldNebulaBank(arg1), "Change Old Nebula Bank");
}

void BackgroundEditorDialog::on_oldNebulaHeadingSpinBox_valueChanged(int arg1)
{
	BG_PUSH(BG_OldNebHeading, _model->setOldNebulaHeading(arg1), "Change Old Nebula Heading");
}

// ---- Ambient Light ----

void BackgroundEditorDialog::on_ambientLightRedSlider_valueChanged(int value)
{
	BG_PUSH(BG_AmbientR, _model->setAmbientR(value), "Change Ambient Red");

	QString text = "R: " + QString::number(value);
	ui->ambientLightRedLabel->setText(text);
	updateAmbientSwatch();
}

void BackgroundEditorDialog::on_ambientLightGreenSlider_valueChanged(int value)
{
	BG_PUSH(BG_AmbientG, _model->setAmbientG(value), "Change Ambient Green");

	QString text = "G: " + QString::number(value);
	ui->ambientLightGreenLabel->setText(text);
	updateAmbientSwatch();
}

void BackgroundEditorDialog::on_ambientLightBlueSlider_valueChanged(int value)
{
	BG_PUSH(BG_AmbientB, _model->setAmbientB(value), "Change Ambient Blue");

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

// ---- Skybox ----

void BackgroundEditorDialog::on_skyboxModelButton_clicked()
{
	const QString lastDir = util::getLastDir("background/skyboxModel", CF_TYPE_MODELS);

	const QString path =
		QFileDialog::getOpenFileName(this, tr("Select Skybox Model"), lastDir, tr("FS2 Models (*.pof);;All Files (*)"));
	if (path.isEmpty())
		return;

	util::saveLastDir("background/skyboxModel", path);
	const SCP_string chosen = QFileInfo(path).completeBaseName().toUtf8().constData();
	BG_PUSH(BG_SkyboxModel, _model->setSkyboxModelName(chosen), "Change Skybox Model");

	updateSkyboxControls();
}

void BackgroundEditorDialog::on_skyboxEdit_textChanged(const QString& arg1)
{
	BG_PUSH(BG_SkyboxModel, _model->setSkyboxModelName(arg1.toUtf8().constData()), "Change Skybox Model");
	updateSkyboxControls();
}

void BackgroundEditorDialog::on_skyboxPitchSpin_valueChanged(double arg1)
{
	BG_PUSH(BG_SkyboxPitch, _model->setSkyboxPitch(static_cast<float>(arg1)), "Change Skybox Pitch");
}

void BackgroundEditorDialog::on_skyboxBankSpin_valueChanged(double arg1)
{
	BG_PUSH(BG_SkyboxBank, _model->setSkyboxBank(static_cast<float>(arg1)), "Change Skybox Bank");
}

void BackgroundEditorDialog::on_skyboxHeadingSpin_valueChanged(double arg1)
{
	BG_PUSH(BG_SkyboxHeading, _model->setSkyboxHeading(static_cast<float>(arg1)), "Change Skybox Heading");
}

void BackgroundEditorDialog::on_noLightingCheckBox_toggled(bool checked)
{
	BG_PUSH(BG_SkyboxFlags, _model->setSkyboxNoLighting(checked), "Change Skybox Flags");
}

void BackgroundEditorDialog::on_transparentCheckBox_toggled(bool checked)
{
	BG_PUSH(BG_SkyboxFlags, _model->setSkyboxAllTransparent(checked), "Change Skybox Flags");
}

void BackgroundEditorDialog::on_forceClampCheckBox_toggled(bool checked)
{
	BG_PUSH(BG_SkyboxFlags, _model->setSkyboxForceClamp(checked), "Change Skybox Flags");
}

void BackgroundEditorDialog::on_noZBufferCheckBox_toggled(bool checked)
{
	BG_PUSH(BG_SkyboxFlags, _model->setSkyboxNoZbuffer(checked), "Change Skybox Flags");
}

void BackgroundEditorDialog::on_noCullCheckBox_toggled(bool checked)
{
	BG_PUSH(BG_SkyboxFlags, _model->setSkyboxNoCull(checked), "Change Skybox Flags");
}

void BackgroundEditorDialog::on_noGlowMapsCheckBox_toggled(bool checked)
{
	BG_PUSH(BG_SkyboxFlags, _model->setSkyboxNoGlowmaps(checked), "Change Skybox Flags");
}

// ---- Misc ----

void BackgroundEditorDialog::on_numStarsSlider_valueChanged(int value)
{
	BG_PUSH(BG_NumStars, _model->setNumStars(value), "Change Number of Stars");

	QString text = "Number of stars: " + QString::number(value);
	ui->numStarsLabel->setText(text);
}

void BackgroundEditorDialog::on_subspaceCheckBox_toggled(bool checked)
{
	BG_PUSH(BG_Subspace, _model->setTakesPlaceInSubspace(checked), "Toggle Subspace");
}

void BackgroundEditorDialog::on_envMapButton_clicked()
{
	const QString lastDir = util::getLastDir("background/envMap", CF_TYPE_MAPS);
	const QString path = QFileDialog::getOpenFileName(this,
		tr("Select Environment Map"),
		lastDir,
		tr("Environment Maps (*.dds);;All Files (*)"));
	if (path.isEmpty())
		return;
	util::saveLastDir("background/envMap", path);
	const SCP_string chosen = QFileInfo(path).completeBaseName().toUtf8().constData();
	BG_PUSH(BG_EnvMap, _model->setEnvironmentMapName(chosen), "Change Environment Map");
	updateMiscControls();
}

void BackgroundEditorDialog::on_envMapEdit_textChanged(const QString& arg1)
{
	BG_PUSH(BG_EnvMap, _model->setEnvironmentMapName(arg1.toUtf8().constData()), "Change Environment Map");
}

void BackgroundEditorDialog::on_lightingProfileCombo_currentIndexChanged(int index)
{
	if (index < 0)
		return;

	const QString text = ui->lightingProfileCombo->itemText(index);
	BG_PUSH(BG_LightProfile, _model->setLightingProfileName(text.toUtf8().constData()), "Change Lighting Profile");
}

} // namespace fso::fred::dialogs
