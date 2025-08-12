#include "WingEditorDialog.h"
#include "General/CheckBoxListDialog.h"
#include "General/ImagePickerDialog.h"
#include "ShipEditor/ShipGoalsDialog.h"
#include "ShipEditor/ShipCustomWarpDialog.h"

#include "ui_WingEditorDialog.h"

#include <ui/util/SignalBlockers.h>
#include <ui/util/ImageRenderer.h>
#include <QMessageBox>

namespace fso::fred::dialogs {

WingEditorDialog::WingEditorDialog(FredView* parent, EditorViewport* viewport)
	: QDialog(parent), ui(new Ui::WingEditorDialog()), _model(new WingEditorDialogModel(this, viewport)),
	  _viewport(viewport)
{
	ui->setupUi(this);

	setWindowTitle(tr("Wing Editor"));
	
	// Whenever the model reports changes, refresh the UI
	connect(_model.get(), &AbstractDialogModel::modelChanged, this, &WingEditorDialog::updateUi);
	connect(_model.get(), &WingEditorDialogModel::wingChanged, this, [this] {
		refreshAllDynamicCombos();
		updateUi();
	});

	refreshAllDynamicCombos();
	updateUi();

	// Resize the dialog to the minimum size
	resize(QDialog::sizeHint());
}

WingEditorDialog::~WingEditorDialog() = default;

void WingEditorDialog::updateUi()
{
	util::SignalBlockers blockers(this);
	
	ui->waveThresholdSpinBox->setMaximum(_model->getMaxWaveThreshold());
	ui->arrivalDistanceSpinBox->setMinimum(_model->getMinArrivalDistance());
	
	// Top section, first column
	ui->wingNameEdit->setText(_model->getWingName().c_str());
	ui->wingLeaderCombo->setCurrentIndex(_model->getWingLeaderIndex());
	ui->numWavesSpinBox->setValue(_model->getNumberOfWaves());
	ui->waveThresholdSpinBox->setValue(_model->getWaveThreshold());
	ui->hotkeyCombo->setCurrentIndex(ui->hotkeyCombo->findData(_model->getHotkey()));

	// Top section, second column
	ui->formationCombo->setCurrentIndex(ui->formationCombo->findData(_model->getFormationId()));
	ui->formationScaleSpinBox->setValue(_model->getFormationScale());
	updateLogoPreview();

	// Arrival controls
	ui->arrivalLocationCombo->setCurrentIndex(static_cast<int>(_model->getArrivalType()));
	ui->arrivalDelaySpinBox->setValue(_model->getArrivalDelay());
	ui->minDelaySpinBox->setValue(_model->getMinWaveDelay());
	ui->maxDelaySpinBox->setValue(_model->getMaxWaveDelay());
	ui->arrivalTargetCombo->setCurrentIndex(ui->arrivalTargetCombo->findData(_model->getArrivalTarget()));
	ui->arrivalDistanceSpinBox->setValue(_model->getArrivalDistance());

	ui->arrivalTree->initializeEditor(_viewport->editor, this);
	ui->arrivalTree->load_tree(_model->getArrivalTree());
	if (ui->arrivalTree->select_sexp_node != -1) {
		ui->arrivalTree->hilite_item(ui->arrivalTree->select_sexp_node);
	}
	ui->noArrivalWarpCheckBox->setChecked(_model->getNoArrivalWarpFlag());
	ui->noArrivalWarpAdjustCheckbox->setChecked(_model->getNoArrivalWarpAdjustFlag());

	// Departure controls
	ui->departureLocationCombo->setCurrentIndex(static_cast<int>(_model->getDepartureType()));
	ui->departureDelaySpinBox->setValue(_model->getDepartureDelay());
	ui->departureTargetCombo->setCurrentIndex(ui->departureTargetCombo->findData(_model->getDepartureTarget()));
	ui->departureTree->initializeEditor(_viewport->editor, this);
	ui->departureTree->load_tree(_model->getDepartureTree());
	if (ui->departureTree->select_sexp_node != -1) {
		ui->departureTree->hilite_item(ui->departureTree->select_sexp_node);
	}
	ui->noDepartureWarpCheckBox->setChecked(_model->getNoDepartureWarpFlag());
	ui->noDepartureWarpAdjustCheckbox->setChecked(_model->getNoDepartureWarpAdjustFlag());

	enableOrDisableControls();
}

void WingEditorDialog::updateLogoPreview()
{
	QImage img;
	QString err;
	const auto filename = _model->getSquadLogo();
	if (fso::fred::util::loadImageToQImage(filename, img, &err)) {
		// scale to the preview area
		const auto pix = QPixmap::fromImage(img).scaled(ui->squadLogoImage->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
		ui->squadLogoImage->setPixmap(pix);
		ui->squadLogoFile->setText(filename.c_str());
	} else {
		ui->squadLogoImage->setPixmap(QPixmap());
		ui->squadLogoFile->setText("(no logo)");
	}
}

void WingEditorDialog::enableOrDisableControls()
{
	util::SignalBlockers blockers(this);
	
	auto enableAll = [&](bool on) {
		// Top section, first column
		ui->wingNameEdit->setEnabled(on);
		ui->wingLeaderCombo->setEnabled(on);
		ui->numWavesSpinBox->setEnabled(on);
		ui->waveThresholdSpinBox->setEnabled(on);
		ui->hotkeyCombo->setEnabled(on);
		ui->wingFlagsButton->setEnabled(on);
		// Top section, second column
		ui->formationCombo->setEnabled(on);
		ui->formationScaleSpinBox->setEnabled(on);
		ui->alignFormationButton->setEnabled(on);
		// Top section, third column
		ui->deleteWingButton->setEnabled(on);
		ui->disbandWingButton->setEnabled(on);
		ui->initialOrdersButton->setEnabled(on);
		// Middle section
		ui->setSquadLogoButton->setEnabled(on);
		// Arrival controls
		ui->arrivalLocationCombo->setEnabled(on);
		ui->arrivalDelaySpinBox->setEnabled(on);
		ui->minDelaySpinBox->setEnabled(on);
		ui->maxDelaySpinBox->setEnabled(on);
		ui->arrivalTargetCombo->setEnabled(on);
		ui->arrivalDistanceSpinBox->setEnabled(on);
		ui->restrictArrivalPathsButton->setEnabled(on);
		ui->customWarpinButton->setEnabled(on);
		ui->arrivalTree->setEnabled(on);
		ui->noArrivalWarpCheckBox->setEnabled(on);
		ui->noArrivalWarpAdjustCheckbox->setEnabled(on);
		// Departure controls
		ui->departureLocationCombo->setEnabled(on);
		ui->departureDelaySpinBox->setEnabled(on);
		ui->departureTargetCombo->setEnabled(on);
		ui->restrictDeparturePathsButton->setEnabled(on);
		ui->customWarpoutButton->setEnabled(on);
		ui->departureTree->setEnabled(on);
		ui->noDepartureWarpCheckBox->setEnabled(on);
		ui->noDepartureWarpAdjustCheckbox->setEnabled(on);
	};

	if (!_model->wingIsValid()) {
		enableAll(false);
		clearGeneralFields();
		clearArrivalFields();
		clearDepartureFields();
		return;
	}

	enableAll(true);

	const bool isPlayerWing = _model->isPlayerWing();
	const bool containsPlayerStart = _model->containsPlayerStart();
	const bool allFighterBombers = _model->wingAllFighterBombers();

	// Waves / Threshold: enabled only if NOT a player wing and all members are fighter/bombers
	const bool wavesEnabled = (!isPlayerWing) && allFighterBombers;
	ui->numWavesSpinBox->setEnabled(wavesEnabled);
	ui->waveThresholdSpinBox->setEnabled(wavesEnabled);

	// Arrival section: disabled for starting wings (SP player wing or MP starting wing)
	const bool arrivalEditable = !isPlayerWing;
	ui->arrivalLocationCombo->setEnabled(arrivalEditable);
	ui->arrivalDelaySpinBox->setEnabled(arrivalEditable);
	ui->minDelaySpinBox->setEnabled(arrivalEditable);
	ui->maxDelaySpinBox->setEnabled(arrivalEditable);
	if (!arrivalEditable) {
		clearArrivalFields();
	}

	// Arrival target/distance and path/custom buttons
	const bool arrivalIsDockBay = _model->arrivalIsDockBay();
	const bool arrivalNeedsTarget = _model->arrivalNeedsTarget();

	ui->arrivalTargetCombo->setEnabled(arrivalEditable && arrivalNeedsTarget);
	ui->arrivalDistanceSpinBox->setEnabled(arrivalEditable && arrivalNeedsTarget);
	ui->restrictArrivalPathsButton->setEnabled(arrivalEditable && arrivalIsDockBay);
	ui->customWarpinButton->setEnabled(arrivalEditable && !arrivalIsDockBay);

	// Arrival cue tree: lock when the wing actually contains Player-1 start (retail behavior)
	ui->arrivalTree->setEnabled(!containsPlayerStart);

	// Also tie the "no arrival warp" checkboxes to whether arrival is editable
	ui->noArrivalWarpCheckBox->setEnabled(arrivalEditable);
	ui->noArrivalWarpAdjustCheckbox->setEnabled(arrivalEditable);

	// Departure side: never gated by starting-wing rule
	ui->departureLocationCombo->setEnabled(true);
	ui->departureDelaySpinBox->setEnabled(true);
	ui->departureTree->setEnabled(true);

	// Departure target and path/custom depends on location
	const bool departureIsDockBay = _model->departureIsDockBay();
	const bool departureNeedsTarget = _model->departureNeedsTarget();

	ui->departureTargetCombo->setEnabled(departureNeedsTarget);
	ui->restrictDeparturePathsButton->setEnabled(departureIsDockBay);
	ui->customWarpoutButton->setEnabled(!departureIsDockBay);

	// "No departure warp" checkboxes always enabled with a valid wing
	ui->noDepartureWarpCheckBox->setEnabled(true);
	ui->noDepartureWarpAdjustCheckbox->setEnabled(true);
}

void WingEditorDialog::clearGeneralFields()
{
	util::SignalBlockers blockers(this);

	ui->wingNameEdit->clear();
	ui->wingLeaderCombo->setCurrentIndex(-1);

	ui->hotkeyCombo->setCurrentIndex(-1);
	ui->formationCombo->setCurrentIndex(-1);

	ui->squadLogoFile->setText("");
}

void WingEditorDialog::clearArrivalFields()
{
	util::SignalBlockers blockers(this);
	
	ui->arrivalLocationCombo->setCurrentIndex(-1);
	ui->arrivalDelaySpinBox->setValue(ui->arrivalDelaySpinBox->minimum());
	ui->minDelaySpinBox->setValue(ui->minDelaySpinBox->minimum());
	ui->maxDelaySpinBox->setValue(ui->maxDelaySpinBox->minimum());

	ui->arrivalTargetCombo->setCurrentIndex(-1);
	ui->arrivalDistanceSpinBox->setValue(ui->arrivalDistanceSpinBox->minimum());

	ui->arrivalTree->clear();
}

void WingEditorDialog::clearDepartureFields()
{
	util::SignalBlockers blockers(this);
	
	ui->departureLocationCombo->setCurrentIndex(-1);
	ui->departureDelaySpinBox->setValue(ui->departureDelaySpinBox->minimum());

	ui->departureTargetCombo->setCurrentIndex(-1);

	ui->departureTree->clear();
}

void WingEditorDialog::refreshLeaderCombo()
{
	util::SignalBlockers blockers(this);
	ui->wingLeaderCombo->clear();
	auto [sel, names] = _model->getLeaderList();
	for (int i = 0; i < (int)names.size(); ++i) {
		ui->wingLeaderCombo->addItem(QString::fromUtf8(names[i].c_str()), i);
	}
	ui->wingLeaderCombo->setCurrentIndex((sel >= 0 && sel < (int)names.size()) ? sel : -1);
}

void WingEditorDialog::refreshHotkeyCombo()
{
	util::SignalBlockers blockers(this);
	ui->hotkeyCombo->clear();
	for (auto& [id, label] : _model->getHotkeyList())
		ui->hotkeyCombo->addItem(QString::fromUtf8(label.c_str()), id);
}

void WingEditorDialog::refreshFormationCombo()
{
	util::SignalBlockers blockers(this);
	ui->formationCombo->clear();
	for (auto& [id, label] : _model->getFormationList())
		ui->formationCombo->addItem(QString::fromUtf8(label.c_str()), id);
}

void WingEditorDialog::refreshArrivalLocationCombo()
{
	util::SignalBlockers blockers(this);
	ui->arrivalLocationCombo->clear();
	for (auto& [id, label] : _model->getArrivalLocationList())
		ui->arrivalLocationCombo->addItem(QString::fromUtf8(label.c_str()), id);
}

void WingEditorDialog::refreshDepartureLocationCombo()
{
	util::SignalBlockers blockers(this);
	ui->departureLocationCombo->clear();
	for (auto& [id, label] : _model->getDepartureLocationList())
		ui->departureLocationCombo->addItem(QString::fromUtf8(label.c_str()), id);
}

void WingEditorDialog::refreshArrivalTargetCombo()
{
	util::SignalBlockers blockers(this);
	ui->arrivalTargetCombo->clear();
	auto items = _model->getArrivalTargetList();
	for (auto& [id, label] : items) {
		ui->arrivalTargetCombo->addItem(QString::fromUtf8(label.c_str()), id);
	}
}

void WingEditorDialog::refreshDepartureTargetCombo()
{
	util::SignalBlockers blockers(this);
	ui->departureTargetCombo->clear();
	auto items = _model->getDepartureTargetList();
	for (auto& [id, label] : items) {
		ui->departureTargetCombo->addItem(QString::fromUtf8(label.c_str()), id);
	}
}

void WingEditorDialog::refreshAllDynamicCombos()
{
	refreshLeaderCombo();
	refreshHotkeyCombo();
	refreshFormationCombo();
	refreshArrivalLocationCombo();
	refreshDepartureLocationCombo();
	refreshArrivalTargetCombo();
	refreshDepartureTargetCombo();
}

void WingEditorDialog::on_hideCuesButton_clicked()
{
	_cues_hidden = !_cues_hidden;
	
	ui->arrivalGroupBox->setHidden(_cues_hidden);
	ui->departureGroupBox->setHidden(_cues_hidden);
	ui->helpText->setHidden(_cues_hidden);
	ui->HelpTitle->setHidden(_cues_hidden);
	ui->hideCuesButton->setText(_cues_hidden ? "Show Cues" : "Hide Cues");

	QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
	resize(sizeHint());
}

void WingEditorDialog::on_wingNameEdit_editingFinished()
{
	const auto newName = ui->wingNameEdit->text().toStdString();
	_model->setWingName(newName);
}

void WingEditorDialog::on_wingLeaderCombo_currentIndexChanged(int index)
{
	_model->setWingLeaderIndex(index);
}

void WingEditorDialog::on_numberOfWavesSpinBox_valueChanged(int value)
{
	_model->setNumberOfWaves(value);
	ui->waveThresholdSpinBox->setMaximum(_model->getMaxWaveThreshold());
}

void WingEditorDialog::on_waveThresholdSpinBox_valueChanged(int value)
{
	_model->setWaveThreshold(value);
}

void WingEditorDialog::on_hotkeyCombo_currentIndexChanged(int /*index*/)
{
	const int value = ui->hotkeyCombo->currentData().toInt(); // -1, 0..MAX_KEYED_TARGETS, or MAX_KEYED_TARGETS for Hidden
	_model->setHotkey(value);
}

void WingEditorDialog::on_formationCombo_currentIndexChanged(int /*index*/)
{
	_model->setFormationId(ui->formationCombo->currentData().toInt());
}

void WingEditorDialog::on_formationScaleSpinBox_valueChanged(double value)
{
	_model->setFormationScale(static_cast<float>(value));
}

void WingEditorDialog::on_alignFormationButton_clicked()
{
	_model->alignWingFormation();
}

void WingEditorDialog::on_setSquadLogoButton_clicked()
{
	const auto files = _model->getSquadLogoList();
	if (files.empty()) {
		QMessageBox::information(this, "Select Squad Image", "No images found.");
		return;
	}

	QStringList qnames;
	qnames.reserve(static_cast<int>(files.size()));
	for (const auto& s : files)
		qnames << QString::fromStdString(s);

	ImagePickerDialog dlg(this);
	dlg.setWindowTitle("Select Squad Image");
	dlg.allowUnset(true);
	dlg.setImageFilenames(qnames);

	// Optional: preselect current
	dlg.setInitialSelection(QString::fromStdString(_model->getSquadLogo()));

	if (dlg.exec() != QDialog::Accepted)
		return;

	const std::string chosen = dlg.selectedFile().toUtf8().constData();
	_model->setSquadLogo(chosen);
	updateLogoPreview();
}

void WingEditorDialog::on_prevWingButton_clicked()
{
	_model->selectPreviousWing();
}

void WingEditorDialog::on_nextWingButton_clicked()
{
	_model->selectNextWing();
}

void WingEditorDialog::on_deleteWingButton_clicked()
{
	if (QMessageBox::question(this, "Confirm", "Are you sure you want to delete this wing? This will remove the wing and delete its ships.") == QMessageBox::Yes) {
		_model->deleteCurrentWing();
	}
}

void WingEditorDialog::on_disbandWingButton_clicked()
{
	if (QMessageBox::question(this, "Confirm", "Are you sure you want to disband this wing? This will remove the wing but leave its ships intact.") == QMessageBox::Yes) {
		_model->disbandCurrentWing();
	}
}

void WingEditorDialog::on_initialOrdersButton_clicked()
{
	if (!_model->wingIsValid()) {
		QMessageBox::warning(this, "Initial Orders", "No valid wing selected.");
		return;
	}

	const int wingIndex = _model->getCurrentWingIndex(); // or your equivalent getter
	if (wingIndex < 0) {
		QMessageBox::warning(this, "Initial Orders", "No valid wing selected.");
		return;
	}

	// block for empty wings (matches old FRED behavior where goals apply to the wing’s ships)
	if (Wings[wingIndex].wave_count <= 0) {
		QMessageBox::information(this, "Initial Orders", "This wing has no ships (wave_count == 0).");
		return;
	}

	// Open the existing ShipGoals dialog in wing mode
	fso::fred::dialogs::ShipGoalsDialog dlg(this, _viewport, false, -1, wingIndex);

	dlg.exec();
}

void WingEditorDialog::on_wingFlagsButton_clicked()
{
	CheckBoxListDialog dlg(this);
	dlg.setCaption("Select Wing Flags");

	// Get our flag list and convert it to Qt's internal types
	auto wingFlags = _model->getWingFlags();

	QVector<std::pair<QString, bool>> checkbox_list;

	for (const auto& flag : wingFlags) {
		checkbox_list.append({flag.first.c_str(), flag.second});
	}

	dlg.setOptions(checkbox_list); // TODO upgrade checkbox to accept and display item descriptions

	if (dlg.exec() == QDialog::Accepted) {
		auto returned_values = dlg.getCheckedStates();

		std::vector<std::pair<SCP_string, bool>> updatedFlags;

		for (int i = 0; i < checkbox_list.size(); ++i) {
			// Convert back to std::string
			std::string name = checkbox_list[i].first.toUtf8().constData();
			updatedFlags.emplace_back(name, returned_values[i]);
		}

		_model->setWingFlags(updatedFlags);
	}
}

void WingEditorDialog::on_arrivalLocationCombo_currentIndexChanged(int /*index*/)
{
	const int value = ui->arrivalLocationCombo->currentData().toInt();
	_model->setArrivalType(static_cast<ArrivalLocation>(value));
	refreshArrivalTargetCombo();
	updateUi();
}

void WingEditorDialog::on_arrivalDelaySpinBox_valueChanged(int value)
{
	_model->setArrivalDelay(value);
}

void WingEditorDialog::on_minDelaySpinBox_valueChanged(int value)
{
	_model->setMinWaveDelay(value);

	util::SignalBlockers blockers(this);
	ui->maxDelaySpinBox->setMinimum(value);
}

void WingEditorDialog::on_maxDelaySpinBox_valueChanged(int value)
{
	_model->setMaxWaveDelay(value);
}

void WingEditorDialog::on_arrivalTargetCombo_currentIndexChanged(int /*index*/)
{
	const int value = ui->arrivalTargetCombo->currentData().toInt();
	_model->setArrivalTarget(value);
	updateUi();
}

void WingEditorDialog::on_arrivalDistanceSpinBox_valueChanged(int value)
{
	_model->setArrivalDistance(value);
}

void WingEditorDialog::on_restrictArrivalPathsButton_clicked()
{
	CheckBoxListDialog dlg(this);
	dlg.setCaption("Select Wing Flags");

	// Get our path list and convert it to Qt's internal types
	auto wingFlags = _model->getArrivalPaths();

	QVector<std::pair<QString, bool>> checkbox_list;

	for (const auto& flag : wingFlags) {
		checkbox_list.append({flag.first.c_str(), flag.second});
	}

	dlg.setOptions(checkbox_list);

	if (dlg.exec() == QDialog::Accepted) {
		auto returned_values = dlg.getCheckedStates();

		std::vector<std::pair<SCP_string, bool>> updatedFlags;

		for (int i = 0; i < checkbox_list.size(); ++i) {
			// Convert back to std::string
			std::string name = checkbox_list[i].first.toUtf8().constData();
			updatedFlags.emplace_back(name, returned_values[i]);
		}

		_model->setArrivalPaths(updatedFlags);
	}
}

void WingEditorDialog::on_customWarpinButton_clicked()
{
	if (!_model->wingIsValid())
		return;

	auto dlg = fso::fred::dialogs::ShipCustomWarpDialog(this,
		_viewport,
		false,
		_model->getCurrentWingIndex(),
		true);
	dlg.exec();
}

void WingEditorDialog::on_arrivalTree_nodeChanged(int newTree)
{
	_model->setArrivalTree(newTree); //TODO This seems broken in a wierd way. Will need followup
}

void WingEditorDialog::on_noArrivalWarpCheckBox_toggled(bool checked)
{
	_model->setNoArrivalWarpFlag(checked);
}

void WingEditorDialog::on_noArrivalWarpAdjustCheckbox_toggled(bool checked)
{
	_model->setNoArrivalWarpAdjustFlag(checked);
}

void WingEditorDialog::on_departureLocationCombo_currentIndexChanged(int /*index*/)
{
	const int value = ui->departureLocationCombo->currentData().toInt();
	_model->setDepartureType(static_cast<DepartureLocation>(value));
	refreshDepartureTargetCombo();
	updateUi();
}

void WingEditorDialog::on_departureDelaySpinBox_valueChanged(int value)
{
	_model->setDepartureDelay(value);
}

void WingEditorDialog::on_departureTargetCombo_currentIndexChanged(int /*index*/)
{
	const int value = ui->departureTargetCombo->currentData().toInt();
	_model->setDepartureTarget(value);
}

void WingEditorDialog::on_restrictDeparturePathsButton_clicked()
{
	CheckBoxListDialog dlg(this);
	dlg.setCaption("Select Wing Flags");

	// Get our path list and convert it to Qt's internal types
	auto wingFlags = _model->getDeparturePaths();

	QVector<std::pair<QString, bool>> checkbox_list;

	for (const auto& flag : wingFlags) {
		checkbox_list.append({flag.first.c_str(), flag.second});
	}

	dlg.setOptions(checkbox_list);

	if (dlg.exec() == QDialog::Accepted) {
		auto returned_values = dlg.getCheckedStates();

		std::vector<std::pair<SCP_string, bool>> updatedFlags;

		for (int i = 0; i < checkbox_list.size(); ++i) {
			// Convert back to std::string
			std::string name = checkbox_list[i].first.toUtf8().constData();
			updatedFlags.emplace_back(name, returned_values[i]);
		}

		_model->setDeparturePaths(updatedFlags);
	}
}

void WingEditorDialog::on_customWarpoutButton_clicked()
{
	if (!_model->wingIsValid())
		return;

	auto dlg = fso::fred::dialogs::ShipCustomWarpDialog(this,
		_viewport,
		true,
		_model->getCurrentWingIndex(),
		true);
	dlg.exec();
}

void WingEditorDialog::on_departureTree_nodeChanged(int newTree)
{
	_model->setDepartureTree(newTree); //TODO This seems broken in a wierd way. Will need followup
}

void WingEditorDialog::on_noDepartureWarpCheckBox_toggled(bool checked)
{
	_model->setNoDepartureWarpFlag(checked);
}

void WingEditorDialog::on_noDepartureWarpAdjustCheckbox_toggled(bool checked)
{
	_model->setNoDepartureWarpAdjustFlag(checked);
}

void WingEditorDialog::on_arrivalTree_helpChanged(const QString& help)
{
	ui->helpText->setPlainText(help);
}

void WingEditorDialog::on_arrivalTree_miniHelpChanged(const QString& help)
{
	ui->HelpTitle->setText(help);
}

void WingEditorDialog::on_departureTree_helpChanged(const QString& help)
{
	ui->helpText->setPlainText(help);
}

void WingEditorDialog::on_departureTree_miniHelpChanged(const QString& help)
{
	ui->HelpTitle->setText(help);
}

} // namespace fso::fred::dialogs
