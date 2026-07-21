#include "WingEditorDialog.h"
#include <ui/util/DialogUndo.h>
#include <QCloseEvent>
#include "General/CheckBoxListDialog.h"
#include "General/ImagePickerDialog.h"
#include "ShipEditor/ShipGoalsDialog.h"
#include "ShipEditor/ShipCustomWarpDialog.h"

#include "ui_WingEditorDialog.h"

#include <globalincs/globals.h>
#include <ship/ship.h>
#include <mission/commands/FredCommands.h>
#include <missioneditor/common.h>
#include <object/object.h>
#include <ui/util/SignalBlockers.h>
#include <ui/util/ImageRenderer.h>
#include <ui/util/menu.h>
#include <QMessageBox>

namespace fso::fred::dialogs {

// ---------------------------------------------------------------------------
// Anonymous-namespace helpers used by undo/redo lambdas in this file.
// ---------------------------------------------------------------------------
namespace {

// Struct types used as FieldEditCommand<T> template arguments.

struct WingDisplayNameState {
	SCP_string display_name;
	bool has_display_name_flag;
};

struct WingMemberWarp {
	int signature;
	int warpin_params_index;
	bool operator==(const WingMemberWarp& o) const {
		return signature == o.signature && warpin_params_index == o.warpin_params_index;
	}
};

struct WingMemberWarpOut {
	int signature;
	int warpout_params_index;
	bool operator==(const WingMemberWarpOut& o) const {
		return signature == o.signature && warpout_params_index == o.warpout_params_index;
	}
};

struct WingArrivalBlock {
	ArrivalLocation arrival_location;
	int arrival_anchor_val; // anchor_to_target() int
	int arrival_distance;
	int arrival_path_mask;
	SCP_vector<WingMemberWarp> memberWarpin;
	bool operator==(const WingArrivalBlock& o) const {
		return arrival_location == o.arrival_location && arrival_anchor_val == o.arrival_anchor_val &&
		       arrival_distance == o.arrival_distance && arrival_path_mask == o.arrival_path_mask &&
		       memberWarpin == o.memberWarpin;
	}
};

struct WingDepartureBlock {
	DepartureLocation departure_location;
	int departure_anchor_val;
	int departure_path_mask;
	SCP_vector<WingMemberWarpOut> memberWarpout;
	bool operator==(const WingDepartureBlock& o) const {
		return departure_location == o.departure_location && departure_anchor_val == o.departure_anchor_val &&
		       departure_path_mask == o.departure_path_mask && memberWarpout == o.memberWarpout;
	}
};

struct WingArrivalTargetState {
	int arrival_anchor_val;
	int arrival_path_mask;
	int arrival_distance;
};

struct WingDepartureTargetState {
	int departure_anchor_val;
	int departure_path_mask;
};


int findWingByName(const SCP_string& name)
{
	for (int i = 0; i < MAX_WINGS; ++i)
		if (Wings[i].wave_count > 0 && stricmp(Wings[i].name, name.c_str()) == 0)
			return i;
	return -1;
}

WingArrivalBlock captureArrivalBlock(int wingIdx)
{
	const auto& w = Wings[wingIdx];
	WingArrivalBlock b;
	b.arrival_location  = w.arrival_location;
	b.arrival_anchor_val = anchor_to_target(w.arrival_anchor);
	b.arrival_distance  = w.arrival_distance;
	b.arrival_path_mask = w.arrival_path_mask;
	for (int i = 0; i < w.wave_count; ++i) {
		const int si = w.ship_index[i];
		if (si < 0 || si >= MAX_SHIPS || Ships[si].objnum < 0) continue;
		b.memberWarpin.push_back({ Objects[Ships[si].objnum].signature, Ships[si].warpin_params_index });
	}
	return b;
}

void restoreArrivalBlock(int wingIdx, const WingArrivalBlock& b)
{
	auto& w = Wings[wingIdx];
	w.arrival_location  = b.arrival_location;
	w.arrival_anchor    = target_to_anchor(b.arrival_anchor_val);
	w.arrival_distance  = b.arrival_distance;
	w.arrival_path_mask = b.arrival_path_mask;
	for (const auto& mw : b.memberWarpin) {
		const int n = obj_get_by_signature(mw.signature);
		if (n < 0) continue;
		Ships[Objects[n].instance].warpin_params_index = mw.warpin_params_index;
	}
}

WingDepartureBlock captureDepartureBlock(int wingIdx)
{
	const auto& w = Wings[wingIdx];
	WingDepartureBlock b;
	b.departure_location  = w.departure_location;
	b.departure_anchor_val = anchor_to_target(w.departure_anchor);
	b.departure_path_mask = w.departure_path_mask;
	for (int i = 0; i < w.wave_count; ++i) {
		const int si = w.ship_index[i];
		if (si < 0 || si >= MAX_SHIPS || Ships[si].objnum < 0) continue;
		b.memberWarpout.push_back({ Objects[Ships[si].objnum].signature, Ships[si].warpout_params_index });
	}
	return b;
}

void restoreDepartureBlock(int wingIdx, const WingDepartureBlock& b)
{
	auto& w = Wings[wingIdx];
	w.departure_location  = b.departure_location;
	w.departure_anchor    = target_to_anchor(b.departure_anchor_val);
	w.departure_path_mask = b.departure_path_mask;
	for (const auto& mw : b.memberWarpout) {
		const int n = obj_get_by_signature(mw.signature);
		if (n < 0) continue;
		Ships[Objects[n].instance].warpout_params_index = mw.warpout_params_index;
	}
}

} // anonymous namespace


WingEditorDialog::WingEditorDialog(FredView* parent, EditorViewport* viewport)
	: QDialog(parent), SexpTreeEditorInterface(flagset<TreeFlags>()),
	  ui(new Ui::WingEditorDialog()), _model(new WingEditorDialogModel(this, viewport)),
	  _fredView(parent), _viewport(viewport)
{

	ui->setupUi(this);
	util::installMainStackUndoShortcuts(this, _fredView->mainUndoStack());

	ui->HelpTitle->setVisible(viewport->Show_sexp_help_wing_editor);
	ui->helpText->setVisible(viewport->Show_sexp_help_wing_editor);

	ui->wingNameEdit->setMaxLength(NAME_LENGTH - 1);
	ui->wingDisplayNameEdit->setMaxLength(NAME_LENGTH - 1);

	setWindowTitle(tr("Wing Editor"));

	// Whenever the model reports changes, refresh the UI
	connect(_model.get(), &AbstractDialogModel::modelChanged, this, &WingEditorDialog::updateUi);
	connect(_model.get(), &WingEditorDialogModel::wingChanged, this, [this] {
		refreshAllDynamicCombos();
		updateUi();
	});

	// The on_arrivalTree_*/on_departureTree_* slots (modified, helpChanged,
	// miniHelpChanged) are auto-connected by setupUi's connectSlotsByName;
	// connecting them here again would run each handler twice per signal (and
	// push duplicate undo commands).

	// "Select Wing" menu: jump the editor to any wing in the mission.
	Editor* editor = viewport->editor;
	util::installSelectMenu(
		this,
		[]() {
			std::vector<util::SelectMenuEntry> entries;
			for (int i = 0; i < MAX_WINGS; i++) {
				if (Wings[i].wave_count) {
					entries.push_back({QString::fromUtf8(Wings[i].name), i});
				}
			}
			return entries;
		},
		[editor]() { return editor->cur_wing; },
		[editor](int wing) { editor->mark_wing(wing); },
		tr("&Select Wing"));

	refreshAllDynamicCombos();
	updateUi();

	// Resize the dialog to the minimum size
	resize(QDialog::sizeHint());
}

WingEditorDialog::~WingEditorDialog() = default;

void WingEditorDialog::closeEvent(QCloseEvent* e)
{
	QDialog::closeEvent(e);
}

void WingEditorDialog::changeEvent(QEvent* e)
{
	if (e->type() == QEvent::ActivationChange && isActiveWindow())
		_fredView->undoGroup()->setActiveStack(_fredView->mainUndoStack());
	QDialog::changeEvent(e);
}

void WingEditorDialog::updateUi()
{
	util::SignalBlockers blockers(this);

	ui->waveThresholdSpinBox->setMaximum(_model->getMaxWaveThreshold());
	ui->arrivalDistanceSpinBox->setMinimum(_model->getMinArrivalDistance());

	// Top section, first column
	ui->wingNameEdit->setText(_model->getWingName().c_str());
	ui->wingDisplayNameEdit->setText(_model->getWingDisplayName().c_str());
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

	ui->arrivalTree->initializeEditor(_viewport->editor, this, _viewport, _fredView);
	ui->arrivalTree->load_tree(_model->getArrivalTree());
	ui->arrivalTree->expandAll();
	if (ui->arrivalTree->select_sexp_node != -1) {
		ui->arrivalTree->hilite_item(ui->arrivalTree->select_sexp_node);
	}
	ui->noArrivalWarpCheckBox->setChecked(_model->getNoArrivalWarpFlag());
	ui->noArrivalWarpAdjustCheckbox->setChecked(_model->getNoArrivalWarpAdjustFlag());

	// Departure controls
	ui->departureLocationCombo->setCurrentIndex(static_cast<int>(_model->getDepartureType()));
	ui->departureDelaySpinBox->setValue(_model->getDepartureDelay());
	ui->departureTargetCombo->setCurrentIndex(ui->departureTargetCombo->findData(_model->getDepartureTarget()));
	ui->departureTree->initializeEditor(_viewport->editor, this, _viewport, _fredView);
	ui->departureTree->load_tree(_model->getDepartureTree());
	ui->departureTree->expandAll();
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
		ui->wingDisplayNameEdit->setEnabled(on);
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
	ui->arrivalDistanceSpinBox->setEnabled(arrivalEditable && _model->arrivalNeedsDistance());
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
	ui->wingDisplayNameEdit->clear();
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
	const auto showHelp = _viewport->Show_sexp_help_wing_editor;

	_cues_hidden = !_cues_hidden;

	ui->arrivalGroupBox->setVisible(!_cues_hidden);
	ui->departureGroupBox->setVisible(!_cues_hidden);
	ui->helpText->setVisible(!_cues_hidden && showHelp);
	ui->HelpTitle->setVisible(!_cues_hidden && showHelp);
	ui->hideCuesButton->setText(_cues_hidden ? "Show Cues" : "Hide Cues");

	QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
	resize(sizeHint());
}

// ---------------------------------------------------------------------------
// Top section, first column
// ---------------------------------------------------------------------------

void WingEditorDialog::on_wingNameEdit_editingFinished()
{
	if (!_model->wingIsValid()) return;
	const SCP_string newName = ui->wingNameEdit->text().toStdString();
	const SCP_string oldName = _model->getWingName();
	if (newName == oldName) return;

	_model->setWingName(newName);
	const SCP_string actualNewName = _model->getWingName();

	if (actualNewName == oldName) {
		// rename failed or was a no-op — sync UI back
		util::SignalBlockers b(this);
		ui->wingNameEdit->setText(QString::fromStdString(oldName));
		return;
	}

	// Build undo command.  On undo find wing by actualNewName; on redo find by oldName.
	auto* cmd = new FieldEditCommand<SCP_string>(
	    FieldId::Wing_Name, _viewport->editor, tr("Rename Wing"), true);
	cmd->setTargetKey(std::to_string(_model->getCurrentWingIndex()));
	// Never merge renames: the setter locates the wing by the names captured at
	// push time, so a merged A->B->C command would look for "B" which no longer
	// exists after the second rename.
	cmd->setNoMerge();
	cmd->addEntry(oldName, actualNewName,
	    [editor = _viewport->editor, oldName, actualNewName](const SCP_string& v) {
		    const SCP_string& findBy = (v == oldName) ? actualNewName : oldName;
		    for (int i = 0; i < MAX_WINGS; ++i) {
			    if (Wings[i].wave_count > 0 && stricmp(Wings[i].name, findBy.c_str()) == 0) {
				    editor->rename_wing(i, v);
				    return;
			    }
		    }
	    });
	_fredView->mainUndoStack()->push(cmd);

	// Clear the text-undo history so Ctrl+Z hits the mission stack.
	util::SignalBlockers b(this);
	ui->wingNameEdit->setText(QString::fromStdString(_model->getWingName()));
	ui->wingDisplayNameEdit->setText(
	    Editor::get_display_name_for_text_box(_model->getWingName()).c_str());
}

void WingEditorDialog::on_wingDisplayNameEdit_editingFinished()
{
	if (!_model->wingIsValid()) return;
	const SCP_string newDisplayName = ui->wingDisplayNameEdit->text().toStdString();
	if (newDisplayName == _model->getWingDisplayName()) return;

	const SCP_string wingName = _model->getWingName();
	const int wingIdx = _model->getCurrentWingIndex();

	const WingDisplayNameState before {
	    Wings[wingIdx].display_name,
	    Wings[wingIdx].flags[Ship::Wing_Flags::Has_display_name]
	};

	_model->setWingDisplayName(newDisplayName);

	const WingDisplayNameState after {
	    Wings[wingIdx].display_name,
	    Wings[wingIdx].flags[Ship::Wing_Flags::Has_display_name]
	};

	if (before.display_name == after.display_name &&
	    before.has_display_name_flag == after.has_display_name_flag)
		return;

	auto* cmd = new FieldEditCommand<WingDisplayNameState>(
	    FieldId::Wing_DisplayName, _viewport->editor, tr("Edit Wing Display Name"), true);
	cmd->setTargetKey(std::to_string(_model->getCurrentWingIndex()));
	cmd->addEntry(before, after, [wingName](const WingDisplayNameState& v) {
		const int w = findWingByName(wingName);
		if (w < 0) return;
		Wings[w].display_name = v.display_name;
		if (v.has_display_name_flag)
			Wings[w].flags.set(Ship::Wing_Flags::Has_display_name);
		else
			Wings[w].flags.remove(Ship::Wing_Flags::Has_display_name);
	});
	_fredView->mainUndoStack()->push(cmd);

	util::SignalBlockers b(this);
	ui->wingDisplayNameEdit->setText(QString::fromStdString(_model->getWingDisplayName()));
}

void WingEditorDialog::on_wingLeaderCombo_currentIndexChanged(int index)
{
	if (!_model->wingIsValid()) return;
	const SCP_string wingName = _model->getWingName();
	const int before = Wings[_model->getCurrentWingIndex()].special_ship;
	_model->setWingLeaderIndex(index);
	const int after = Wings[_model->getCurrentWingIndex()].special_ship;
	if (before == after) return;

	auto* cmd = new FieldEditCommand<int>(
	    FieldId::Wing_Leader, _viewport->editor, tr("Change Wing Leader"), true);
	cmd->setTargetKey(std::to_string(_model->getCurrentWingIndex()));
	cmd->addEntry(before, after, [wingName](const int& v) {
		const int w = findWingByName(wingName);
		if (w >= 0) Wings[w].special_ship = v;
	});
	_fredView->mainUndoStack()->push(cmd);
}

void WingEditorDialog::on_numWavesSpinBox_valueChanged(int value)
{
	if (!_model->wingIsValid()) return;
	const SCP_string wingName = _model->getWingName();
	const int before = Wings[_model->getCurrentWingIndex()].num_waves;
	_model->setNumberOfWaves(value);
	const int after = Wings[_model->getCurrentWingIndex()].num_waves;
	if (before == after) return;

	auto* cmd = new FieldEditCommand<int>(
	    FieldId::Wing_NumWaves, _viewport->editor, tr("Edit Wing Waves"), true);
	cmd->setTargetKey(std::to_string(_model->getCurrentWingIndex()));
	cmd->addEntry(before, after, [wingName](const int& v) {
		const int w = findWingByName(wingName);
		if (w >= 0) Wings[w].num_waves = v;
	});
	_fredView->mainUndoStack()->push(cmd);
	{
		// If the new maximum clamps the current threshold, don't let the
		// resulting valueChanged split this gesture into a second undo step —
		// the model setter below already clamps.
		QSignalBlocker blocker(ui->waveThresholdSpinBox);
		ui->waveThresholdSpinBox->setMaximum(_model->getMaxWaveThreshold());
	}
}

void WingEditorDialog::on_waveThresholdSpinBox_valueChanged(int value)
{
	if (!_model->wingIsValid()) return;
	const SCP_string wingName = _model->getWingName();
	const int before = Wings[_model->getCurrentWingIndex()].threshold;
	_model->setWaveThreshold(value);
	const int after = Wings[_model->getCurrentWingIndex()].threshold;
	if (before == after) return;

	auto* cmd = new FieldEditCommand<int>(
	    FieldId::Wing_Threshold, _viewport->editor, tr("Edit Wing Wave Threshold"), true);
	cmd->setTargetKey(std::to_string(_model->getCurrentWingIndex()));
	cmd->addEntry(before, after, [wingName](const int& v) {
		const int w = findWingByName(wingName);
		if (w >= 0) Wings[w].threshold = v;
	});
	_fredView->mainUndoStack()->push(cmd);
}

void WingEditorDialog::on_hotkeyCombo_currentIndexChanged(int /*index*/)
{
	if (!_model->wingIsValid()) return;
	const int value = ui->hotkeyCombo->currentData().toInt();
	const SCP_string wingName = _model->getWingName();
	const int before = Wings[_model->getCurrentWingIndex()].hotkey;
	_model->setHotkey(value);
	const int after = Wings[_model->getCurrentWingIndex()].hotkey;
	if (before == after) return;

	auto* cmd = new FieldEditCommand<int>(
	    FieldId::Wing_Hotkey, _viewport->editor, tr("Change Wing Hotkey"), true);
	cmd->setTargetKey(std::to_string(_model->getCurrentWingIndex()));
	cmd->addEntry(before, after, [wingName](const int& v) {
		const int w = findWingByName(wingName);
		if (w >= 0) Wings[w].hotkey = v;
	});
	_fredView->mainUndoStack()->push(cmd);
}

// ---------------------------------------------------------------------------
// Top section, second column
// ---------------------------------------------------------------------------

void WingEditorDialog::on_formationCombo_currentIndexChanged(int /*index*/)
{
	if (!_model->wingIsValid()) return;
	const SCP_string wingName = _model->getWingName();
	const int before = Wings[_model->getCurrentWingIndex()].formation;
	_model->setFormationId(ui->formationCombo->currentData().toInt());
	const int after = Wings[_model->getCurrentWingIndex()].formation;
	if (before == after) return;

	auto* cmd = new FieldEditCommand<int>(
	    FieldId::Wing_Formation, _viewport->editor, tr("Change Wing Formation"), true);
	cmd->setTargetKey(std::to_string(_model->getCurrentWingIndex()));
	cmd->addEntry(before, after, [wingName](const int& v) {
		const int w = findWingByName(wingName);
		if (w >= 0) Wings[w].formation = v;
	});
	_fredView->mainUndoStack()->push(cmd);
}

void WingEditorDialog::on_formationScaleSpinBox_valueChanged(double value)
{
	if (!_model->wingIsValid()) return;
	const SCP_string wingName = _model->getWingName();
	const float before = Wings[_model->getCurrentWingIndex()].formation_scale;
	_model->setFormationScale(static_cast<float>(value));
	const float after = Wings[_model->getCurrentWingIndex()].formation_scale;
	if (before == after) return;

	auto* cmd = new FieldEditCommand<float>(
	    FieldId::Wing_FormationScale, _viewport->editor, tr("Edit Wing Formation Scale"), true);
	cmd->setTargetKey(std::to_string(_model->getCurrentWingIndex()));
	cmd->addEntry(before, after, [wingName](const float& v) {
		const int w = findWingByName(wingName);
		if (w >= 0) Wings[w].formation_scale = v;
	});
	_fredView->mainUndoStack()->push(cmd);
}

void WingEditorDialog::on_alignFormationButton_clicked()
{
	if (!_model->wingIsValid()) return;
	const int wingIdx = _model->getCurrentWingIndex();
	const auto& wingp = Wings[wingIdx];

	// Capture before positions for non-leader members (indices 1..wave_count-1)
	SCP_vector<ObjectTransform> transforms;
	for (int i = 1; i < wingp.wave_count; ++i) {
		const int si = wingp.ship_index[i];
		if (si < 0 || si >= MAX_SHIPS || Ships[si].objnum < 0) continue;
		const auto& obj = Objects[Ships[si].objnum];
		transforms.push_back({ obj.signature, obj.pos, obj.orient, obj.pos, obj.orient });
	}

	_model->alignWingFormation();

	// Fill in posAfter / orientAfter after the formation realignment
	for (auto& t : transforms) {
		const int objNum = obj_get_by_signature(t.signature);
		if (objNum < 0) continue;
		t.posAfter    = Objects[objNum].pos;
		t.orientAfter = Objects[objNum].orient;
	}

	if (!transforms.empty())
		_fredView->mainUndoStack()->push(
		    new MoveObjectsCommand(std::move(transforms), _viewport->editor, _viewport));
}

void WingEditorDialog::on_setSquadLogoButton_clicked()
{
	if (!_model->wingIsValid()) return;
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
	dlg.setInitialSelection(QString::fromStdString(_model->getSquadLogo()));

	if (dlg.exec() != QDialog::Accepted)
		return;

	const SCP_string chosen = dlg.selectedFile().toUtf8().constData();
	const SCP_string wingName = _model->getWingName();
	const int wingIdx = _model->getCurrentWingIndex();
	const SCP_string before = Wings[wingIdx].wing_squad_filename;

	_model->setSquadLogo(chosen);
	_viewport->editor->missionChanged();

	const SCP_string after = Wings[wingIdx].wing_squad_filename;
	if (before == after) return;

	auto* cmd = new FieldEditCommand<SCP_string>(
	    FieldId::Wing_SquadLogo, _viewport->editor, tr("Change Wing Squad Logo"), true);
	cmd->setTargetKey(std::to_string(_model->getCurrentWingIndex()));
	cmd->addEntry(before, after, [wingName](const SCP_string& v) {
		const int w = findWingByName(wingName);
		if (w < 0) return;
		strcpy_s(Wings[w].wing_squad_filename, v.c_str());
	});
	_fredView->mainUndoStack()->push(cmd);
	updateLogoPreview();
}

// ---------------------------------------------------------------------------
// Top section, third column
// ---------------------------------------------------------------------------

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
	if (!_model->wingIsValid()) return;
	if (QMessageBox::question(this, "Confirm", "Are you sure you want to delete this wing? This will remove the wing and delete its ships.") == QMessageBox::Yes) {
		const int wingIndex = _model->getCurrentWingIndex();
		// Capture before the delete, push only if it wasn't canceled at the
		// reference check (first redo() is a no-op either way).
		auto* cmd = new DeleteWingCommand(wingIndex, _viewport->editor, _viewport);
		if (_viewport->editor->delete_wing(wingIndex, 0) == 0) {
			_fredView->mainUndoStack()->push(cmd);
		} else {
			delete cmd;
		}
	}
}

void WingEditorDialog::on_disbandWingButton_clicked()
{
	if (!_model->wingIsValid()) return;
	if (QMessageBox::question(this, "Confirm", "Are you sure you want to disband this wing? This will remove the wing but leave its ships intact.") == QMessageBox::Yes) {
		const int wingIndex = _model->getCurrentWingIndex();
		auto* cmd = new DisbandWingCommand(wingIndex, _viewport->editor, _viewport);
		if (_viewport->editor->disband_wing(wingIndex) == 0) {
			_fredView->mainUndoStack()->push(cmd);
		} else {
			delete cmd;
		}
	}
}

void WingEditorDialog::on_initialOrdersButton_clicked()
{
	if (!_model->wingIsValid()) return;
	const int wingIndex = _model->getCurrentWingIndex();
	if (wingIndex < 0) return;
	const SCP_string wingName = _model->getWingName();

	// Wing initial orders live in Wings[].ai_goals, not per-ship Ai_info[].goals.
	// Use copy assignment (not memcpy): ai_goal has ai_lua_parameters with a
	// SCP_vector<LuaValue> that requires deep copy.
	std::array<ai_goal, MAX_AI_GOALS> before;
	for (int g = 0; g < MAX_AI_GOALS; ++g)
		before[g] = Wings[wingIndex].ai_goals[g];

	fso::fred::dialogs::ShipGoalsDialog dlg(this, _viewport, false, -1, wingIndex);
	if (dlg.exec() != QDialog::Accepted) return;

	const int wi = findWingByName(wingName);
	if (wi < 0) return;
	std::array<ai_goal, MAX_AI_GOALS> after;
	for (int g = 0; g < MAX_AI_GOALS; ++g)
		after[g] = Wings[wi].ai_goals[g];

	auto* cmd = new FieldEditCommand<std::array<ai_goal, MAX_AI_GOALS>>(
	    FieldId::Wing_InitialOrders, _viewport->editor, tr("Edit Wing Initial Orders"), true);
	cmd->setTargetKey(std::to_string(_model->getCurrentWingIndex()));
	cmd->setNoMerge();
	cmd->addEntry(std::move(before), std::move(after),
	    [wingName](const std::array<ai_goal, MAX_AI_GOALS>& v) {
		    const int w = findWingByName(wingName);
		    if (w < 0) return;
		    for (int g = 0; g < MAX_AI_GOALS; ++g)
			    Wings[w].ai_goals[g] = v[g];
	    });
	_fredView->mainUndoStack()->push(cmd);
}

void WingEditorDialog::on_wingFlagsButton_clicked()
{
	if (!_model->wingIsValid()) return;
	QVector<std::pair<QString, int>> qtFlags;
	for (const auto& f : _model->getWingFlags())
		qtFlags.append({QString::fromUtf8(f.first.c_str()), f.second ? Qt::Checked : Qt::Unchecked});

	QVector<std::pair<QString, QString>> qtDescs;
	for (const auto& d : _model->getWingFlagDescriptions())
		qtDescs.append({QString::fromUtf8(d.first.c_str()), QString::fromUtf8(d.second.c_str())});

	dialogs::CheckBoxListDialog dlg(this);
	dlg.setCaption(tr("Wing Flags"));
	dlg.setOptions(qtFlags);
	dlg.setOptionDescriptions(qtDescs);

	if (dlg.exec() != QDialog::Accepted) return;

	const SCP_string wingName = _model->getWingName();
	const int wingIdx = _model->getCurrentWingIndex();
	const flagset<Ship::Wing_Flags> before = Wings[wingIdx].flags;

	SCP_vector<std::pair<SCP_string, bool>> result;
	for (const auto& f : dlg.getFlags())
		result.emplace_back(f.first.toUtf8().constData(), f.second == Qt::Checked);
	_model->setWingFlags(result);
	_viewport->editor->missionChanged();

	const flagset<Ship::Wing_Flags> after = Wings[wingIdx].flags;
	if (before == after) return;

	// Use flagset direct assignment rather than to_ubyte_vector/set_from_vector:
	// set_from_vector only sets bits and never clears, so restoring an empty
	// "before" state would silently leave all flags set.
	auto* cmd = new FieldEditCommand<flagset<Ship::Wing_Flags>>(
	    FieldId::Wing_Flags, _viewport->editor, tr("Edit Wing Flags"), true);
	cmd->setTargetKey(std::to_string(_model->getCurrentWingIndex()));
	cmd->setNoMerge();
	cmd->addEntry(before, after, [wingName](const flagset<Ship::Wing_Flags>& v) {
		const int w = findWingByName(wingName);
		if (w >= 0) Wings[w].flags = v;
	});
	_fredView->mainUndoStack()->push(cmd);
}

// ---------------------------------------------------------------------------
// Arrival controls
// ---------------------------------------------------------------------------

void WingEditorDialog::on_arrivalLocationCombo_currentIndexChanged(int /*index*/)
{
	const int value = ui->arrivalLocationCombo->currentData().toInt();
	if (!_model->wingIsValid()) return;
	const SCP_string wingName = _model->getWingName();
	const int wingIdx = _model->getCurrentWingIndex();

	const WingArrivalBlock before = captureArrivalBlock(wingIdx);
	_model->setArrivalType(static_cast<ArrivalLocation>(value));
	const WingArrivalBlock after = captureArrivalBlock(wingIdx);
	if (before == after) {
		// No-op or refused change: resync the UI, but push nothing.
		refreshArrivalTargetCombo();
		updateUi();
		return;
	}

	auto* cmd = new FieldEditCommand<WingArrivalBlock>(
	    FieldId::Wing_ArrivalType, _viewport->editor, tr("Change Wing Arrival Type"), true);
	cmd->setTargetKey(std::to_string(_model->getCurrentWingIndex()));
	cmd->addEntry(before, after, [wingName](const WingArrivalBlock& v) {
		const int w = findWingByName(wingName);
		if (w >= 0) restoreArrivalBlock(w, v);
	});
	_fredView->mainUndoStack()->push(cmd);
	refreshArrivalTargetCombo();
	updateUi();
}

void WingEditorDialog::on_arrivalDelaySpinBox_valueChanged(int value)
{
	if (!_model->wingIsValid()) return;
	const SCP_string wingName = _model->getWingName();
	const int before = Wings[_model->getCurrentWingIndex()].arrival_delay;
	_model->setArrivalDelay(value);
	const int after = Wings[_model->getCurrentWingIndex()].arrival_delay;
	if (before == after) return;

	auto* cmd = new FieldEditCommand<int>(
	    FieldId::Wing_ArrivalDelay, _viewport->editor, tr("Edit Wing Arrival Delay"), true);
	cmd->setTargetKey(std::to_string(_model->getCurrentWingIndex()));
	cmd->addEntry(before, after, [wingName](const int& v) {
		const int w = findWingByName(wingName);
		if (w >= 0) Wings[w].arrival_delay = v;
	});
	_fredView->mainUndoStack()->push(cmd);
}

void WingEditorDialog::on_minDelaySpinBox_valueChanged(int value)
{
	if (!_model->wingIsValid()) return;
	const SCP_string wingName = _model->getWingName();
	const int wingIdx = _model->getCurrentWingIndex();
	const std::pair<int,int> before { Wings[wingIdx].wave_delay_min, Wings[wingIdx].wave_delay_max };

	_model->setMinWaveDelay(value);

	const std::pair<int,int> after { Wings[wingIdx].wave_delay_min, Wings[wingIdx].wave_delay_max };
	if (before == after) return;

	auto* cmd = new FieldEditCommand<std::pair<int,int>>(
	    FieldId::Wing_WaveDelays, _viewport->editor, tr("Edit Wing Wave Delay"), true);
	cmd->setTargetKey(std::to_string(_model->getCurrentWingIndex()));
	cmd->addEntry(before, after, [wingName](const std::pair<int,int>& v) {
		const int w = findWingByName(wingName);
		if (w < 0) return;
		Wings[w].wave_delay_min = v.first;
		Wings[w].wave_delay_max = v.second;
	});
	_fredView->mainUndoStack()->push(cmd);

	util::SignalBlockers blockers(this);
	ui->maxDelaySpinBox->setMinimum(value);
}

void WingEditorDialog::on_maxDelaySpinBox_valueChanged(int value)
{
	if (!_model->wingIsValid()) return;
	const SCP_string wingName = _model->getWingName();
	const int wingIdx = _model->getCurrentWingIndex();
	const std::pair<int,int> before { Wings[wingIdx].wave_delay_min, Wings[wingIdx].wave_delay_max };

	_model->setMaxWaveDelay(value);

	const std::pair<int,int> after { Wings[wingIdx].wave_delay_min, Wings[wingIdx].wave_delay_max };
	if (before == after) return;

	auto* cmd = new FieldEditCommand<std::pair<int,int>>(
	    FieldId::Wing_WaveDelays, _viewport->editor, tr("Edit Wing Wave Delay"), true);
	cmd->setTargetKey(std::to_string(_model->getCurrentWingIndex()));
	cmd->addEntry(before, after, [wingName](const std::pair<int,int>& v) {
		const int w = findWingByName(wingName);
		if (w < 0) return;
		Wings[w].wave_delay_min = v.first;
		Wings[w].wave_delay_max = v.second;
	});
	_fredView->mainUndoStack()->push(cmd);
}

void WingEditorDialog::on_arrivalTargetCombo_currentIndexChanged(int /*index*/)
{
	const int value = ui->arrivalTargetCombo->currentData().toInt();
	if (!_model->wingIsValid()) return;
	const SCP_string wingName = _model->getWingName();
	const int wingIdx = _model->getCurrentWingIndex();

	const WingArrivalTargetState before {
	    anchor_to_target(Wings[wingIdx].arrival_anchor),
	    Wings[wingIdx].arrival_path_mask,
	    Wings[wingIdx].arrival_distance
	};
	_model->setArrivalTarget(value);
	const WingArrivalTargetState after {
	    anchor_to_target(Wings[wingIdx].arrival_anchor),
	    Wings[wingIdx].arrival_path_mask,
	    Wings[wingIdx].arrival_distance
	};

	if (before.arrival_anchor_val == after.arrival_anchor_val &&
	    before.arrival_path_mask == after.arrival_path_mask &&
	    before.arrival_distance == after.arrival_distance)
		return;

	auto* cmd = new FieldEditCommand<WingArrivalTargetState>(
	    FieldId::Wing_ArrivalTarget, _viewport->editor, tr("Change Wing Arrival Target"), true);
	cmd->setTargetKey(std::to_string(_model->getCurrentWingIndex()));
	cmd->addEntry(before, after, [wingName](const WingArrivalTargetState& v) {
		const int w = findWingByName(wingName);
		if (w < 0) return;
		Wings[w].arrival_anchor   = target_to_anchor(v.arrival_anchor_val);
		Wings[w].arrival_path_mask = v.arrival_path_mask;
		Wings[w].arrival_distance = v.arrival_distance;
	});
	_fredView->mainUndoStack()->push(cmd);
	updateUi();
}

void WingEditorDialog::on_arrivalDistanceSpinBox_valueChanged(int value)
{
	if (!_model->wingIsValid()) return;
	const SCP_string wingName = _model->getWingName();
	const int before = Wings[_model->getCurrentWingIndex()].arrival_distance;
	_model->setArrivalDistance(value);
	const int after = Wings[_model->getCurrentWingIndex()].arrival_distance;
	if (before == after) return;

	auto* cmd = new FieldEditCommand<int>(
	    FieldId::Wing_ArrivalDist, _viewport->editor, tr("Edit Wing Arrival Distance"), true);
	cmd->setTargetKey(std::to_string(_model->getCurrentWingIndex()));
	cmd->addEntry(before, after, [wingName](const int& v) {
		const int w = findWingByName(wingName);
		if (w >= 0) Wings[w].arrival_distance = v;
	});
	_fredView->mainUndoStack()->push(cmd);
}

void WingEditorDialog::on_restrictArrivalPathsButton_clicked()
{
	if (!_model->wingIsValid()) return;
	CheckBoxListDialog dlg(this);
	dlg.setCaption("Select Wing Flags");

	auto wingFlags = _model->getArrivalPaths();
	QVector<std::pair<QString, bool>> checkbox_list;
	for (const auto& flag : wingFlags)
		checkbox_list.append({flag.first.c_str(), flag.second});
	dlg.setOptions(checkbox_list);

	if (dlg.exec() != QDialog::Accepted) return;

	const SCP_string wingName = _model->getWingName();
	const int wingIdx = _model->getCurrentWingIndex();
	const int before = Wings[wingIdx].arrival_path_mask;

	auto returned_values = dlg.getCheckedStates();
	SCP_vector<std::pair<SCP_string, bool>> updatedFlags;
	for (int i = 0; i < checkbox_list.size(); ++i)
		updatedFlags.emplace_back(checkbox_list[i].first.toUtf8().constData(), returned_values[i]);
	_model->setArrivalPaths(updatedFlags);
	_viewport->editor->missionChanged();

	const int after = Wings[wingIdx].arrival_path_mask;
	if (before == after) return;

	auto* cmd = new FieldEditCommand<int>(
	    FieldId::Wing_ArrivalPaths, _viewport->editor, tr("Edit Wing Arrival Paths"), true);
	cmd->setTargetKey(std::to_string(_model->getCurrentWingIndex()));
	cmd->addEntry(before, after, [wingName](const int& v) {
		const int w = findWingByName(wingName);
		if (w >= 0) Wings[w].arrival_path_mask = v;
	});
	_fredView->mainUndoStack()->push(cmd);
}

void WingEditorDialog::on_customWarpinButton_clicked()
{
	if (!_model->wingIsValid()) return;
	const int wingIdx   = _model->getCurrentWingIndex();
	const SCP_string wingName = _model->getWingName();

	// Capture warpin_params_index for each member before the dialog runs
	SCP_vector<WingMemberWarp> before;
	const auto& wp = Wings[wingIdx];
	for (int i = 0; i < wp.wave_count; ++i) {
		const int si = wp.ship_index[i];
		if (si < 0 || si >= MAX_SHIPS || Ships[si].objnum < 0) continue;
		before.push_back({ Objects[Ships[si].objnum].signature, Ships[si].warpin_params_index });
	}

	auto dlg = fso::fred::dialogs::ShipCustomWarpDialog(this, _viewport, false, wingIdx, true);
	if (dlg.exec() != QDialog::Accepted) return;

	// Capture after state (locate wing by name in case index shifted)
	const int wi = findWingByName(wingName);
	if (wi < 0) return;
	SCP_vector<WingMemberWarp> after;
	const auto& wp2 = Wings[wi];
	for (int i = 0; i < wp2.wave_count; ++i) {
		const int si = wp2.ship_index[i];
		if (si < 0 || si >= MAX_SHIPS || Ships[si].objnum < 0) continue;
		after.push_back({ Objects[Ships[si].objnum].signature, Ships[si].warpin_params_index });
	}

	if (before == after) return;

	auto* cmd = new FieldEditCommand<SCP_vector<WingMemberWarp>>(
	    FieldId::Wing_WarpinParams, _viewport->editor, tr("Change Wing Warp-In Params"), true);
	cmd->setTargetKey(std::to_string(_model->getCurrentWingIndex()));
	cmd->setNoMerge();
	cmd->addEntry(std::move(before), std::move(after), [](const SCP_vector<WingMemberWarp>& v) {
		for (const auto& m : v) {
			const int objNum = obj_get_by_signature(m.signature);
			if (objNum < 0) continue;
			const int si = Objects[objNum].instance;
			if (si < 0 || si >= MAX_SHIPS) continue;
			Ships[si].warpin_params_index = m.warpin_params_index;
		}
	});
	_fredView->mainUndoStack()->push(cmd);
}

void WingEditorDialog::on_arrivalTree_modified()
{
	if (!_model->wingIsValid()) {
		_model->setArrivalTree(ui->arrivalTree->_model.save_tree());
		return;
	}

	const SCP_string wingName = _model->getWingName();
	auto* cmd = new SexpCueEditCommand(_viewport->editor, tr("Edit Wing Arrival Cue"), true);
	cmd->addOwner(Wings[_model->getCurrentWingIndex()].arrival_cue,
		[wingName]() {
			const int w = findWingByName(wingName);
			return w < 0 ? -1 : Wings[w].arrival_cue;
		},
		[wingName](int formula) {
			const int w = findWingByName(wingName);
			if (w >= 0)
				Wings[w].arrival_cue = formula;
		});

	const int newFormula = ui->arrivalTree->_model.save_tree();
	_model->setArrivalTree(newFormula);
	cmd->captureAfter(newFormula);
	_fredView->mainUndoStack()->push(cmd);
}

void WingEditorDialog::on_noArrivalWarpCheckBox_toggled(bool checked)
{
	if (!_model->wingIsValid()) return;
	const SCP_string wingName = _model->getWingName();
	const bool before = Wings[_model->getCurrentWingIndex()].flags[Ship::Wing_Flags::No_arrival_warp];
	_model->setNoArrivalWarpFlag(checked);
	const bool after = Wings[_model->getCurrentWingIndex()].flags[Ship::Wing_Flags::No_arrival_warp];
	if (before == after) return;

	auto* cmd = new FieldEditCommand<bool>(
	    FieldId::Wing_NoArrivalWarp, _viewport->editor, tr("Toggle No Arrival Warp"), true);
	cmd->setTargetKey(std::to_string(_model->getCurrentWingIndex()));
	cmd->addEntry(before, after, [wingName](const bool& v) {
		const int w = findWingByName(wingName);
		if (w < 0) return;
		if (v) Wings[w].flags.set(Ship::Wing_Flags::No_arrival_warp);
		else Wings[w].flags.remove(Ship::Wing_Flags::No_arrival_warp);
	});
	_fredView->mainUndoStack()->push(cmd);
	_viewport->editor->missionChanged();
}

void WingEditorDialog::on_noArrivalWarpAdjustCheckbox_toggled(bool checked)
{
	if (!_model->wingIsValid()) return;
	const SCP_string wingName = _model->getWingName();
	const bool before = Wings[_model->getCurrentWingIndex()].flags[Ship::Wing_Flags::Same_arrival_warp_when_docked];
	_model->setNoArrivalWarpAdjustFlag(checked);
	const bool after = Wings[_model->getCurrentWingIndex()].flags[Ship::Wing_Flags::Same_arrival_warp_when_docked];
	if (before == after) return;

	auto* cmd = new FieldEditCommand<bool>(
	    FieldId::Wing_NoArrivalWarpAdj, _viewport->editor, tr("Toggle Same Arrival Warp When Docked"), true);
	cmd->setTargetKey(std::to_string(_model->getCurrentWingIndex()));
	cmd->addEntry(before, after, [wingName](const bool& v) {
		const int w = findWingByName(wingName);
		if (w < 0) return;
		if (v) Wings[w].flags.set(Ship::Wing_Flags::Same_arrival_warp_when_docked);
		else Wings[w].flags.remove(Ship::Wing_Flags::Same_arrival_warp_when_docked);
	});
	_fredView->mainUndoStack()->push(cmd);
	_viewport->editor->missionChanged();
}

// ---------------------------------------------------------------------------
// Departure controls
// ---------------------------------------------------------------------------

void WingEditorDialog::on_departureLocationCombo_currentIndexChanged(int /*index*/)
{
	const int value = ui->departureLocationCombo->currentData().toInt();
	if (!_model->wingIsValid()) return;
	const SCP_string wingName = _model->getWingName();
	const int wingIdx = _model->getCurrentWingIndex();

	const WingDepartureBlock before = captureDepartureBlock(wingIdx);
	_model->setDepartureType(static_cast<DepartureLocation>(value));
	const WingDepartureBlock after = captureDepartureBlock(wingIdx);
	if (before == after) {
		// No-op or refused change: resync the UI, but push nothing.
		refreshDepartureTargetCombo();
		updateUi();
		return;
	}

	auto* cmd = new FieldEditCommand<WingDepartureBlock>(
	    FieldId::Wing_DepartureType, _viewport->editor, tr("Change Wing Departure Type"), true);
	cmd->setTargetKey(std::to_string(_model->getCurrentWingIndex()));
	cmd->addEntry(before, after, [wingName](const WingDepartureBlock& v) {
		const int w = findWingByName(wingName);
		if (w >= 0) restoreDepartureBlock(w, v);
	});
	_fredView->mainUndoStack()->push(cmd);
	refreshDepartureTargetCombo();
	updateUi();
}

void WingEditorDialog::on_departureDelaySpinBox_valueChanged(int value)
{
	if (!_model->wingIsValid()) return;
	const SCP_string wingName = _model->getWingName();
	const int before = Wings[_model->getCurrentWingIndex()].departure_delay;
	_model->setDepartureDelay(value);
	const int after = Wings[_model->getCurrentWingIndex()].departure_delay;
	if (before == after) return;

	auto* cmd = new FieldEditCommand<int>(
	    FieldId::Wing_DepartureDelay, _viewport->editor, tr("Edit Wing Departure Delay"), true);
	cmd->setTargetKey(std::to_string(_model->getCurrentWingIndex()));
	cmd->addEntry(before, after, [wingName](const int& v) {
		const int w = findWingByName(wingName);
		if (w >= 0) Wings[w].departure_delay = v;
	});
	_fredView->mainUndoStack()->push(cmd);
}

void WingEditorDialog::on_departureTargetCombo_currentIndexChanged(int /*index*/)
{
	const int value = ui->departureTargetCombo->currentData().toInt();
	if (!_model->wingIsValid()) return;
	const SCP_string wingName = _model->getWingName();
	const int wingIdx = _model->getCurrentWingIndex();

	const WingDepartureTargetState before {
	    anchor_to_target(Wings[wingIdx].departure_anchor),
	    Wings[wingIdx].departure_path_mask
	};
	_model->setDepartureTarget(value);
	const WingDepartureTargetState after {
	    anchor_to_target(Wings[wingIdx].departure_anchor),
	    Wings[wingIdx].departure_path_mask
	};

	if (before.departure_anchor_val == after.departure_anchor_val &&
	    before.departure_path_mask == after.departure_path_mask)
		return;

	auto* cmd = new FieldEditCommand<WingDepartureTargetState>(
	    FieldId::Wing_DepartureTarget, _viewport->editor, tr("Change Wing Departure Target"), true);
	cmd->setTargetKey(std::to_string(_model->getCurrentWingIndex()));
	cmd->addEntry(before, after, [wingName](const WingDepartureTargetState& v) {
		const int w = findWingByName(wingName);
		if (w < 0) return;
		Wings[w].departure_anchor    = target_to_anchor(v.departure_anchor_val);
		Wings[w].departure_path_mask = v.departure_path_mask;
	});
	_fredView->mainUndoStack()->push(cmd);
}

void WingEditorDialog::on_restrictDeparturePathsButton_clicked()
{
	if (!_model->wingIsValid()) return;
	CheckBoxListDialog dlg(this);
	dlg.setCaption("Select Wing Flags");

	auto wingFlags = _model->getDeparturePaths();
	QVector<std::pair<QString, bool>> checkbox_list;
	for (const auto& flag : wingFlags)
		checkbox_list.append({flag.first.c_str(), flag.second});
	dlg.setOptions(checkbox_list);

	if (dlg.exec() != QDialog::Accepted) return;

	const SCP_string wingName = _model->getWingName();
	const int wingIdx = _model->getCurrentWingIndex();
	const int before = Wings[wingIdx].departure_path_mask;

	auto returned_values = dlg.getCheckedStates();
	SCP_vector<std::pair<SCP_string, bool>> updatedFlags;
	for (int i = 0; i < checkbox_list.size(); ++i)
		updatedFlags.emplace_back(checkbox_list[i].first.toUtf8().constData(), returned_values[i]);
	_model->setDeparturePaths(updatedFlags);
	_viewport->editor->missionChanged();

	const int after = Wings[wingIdx].departure_path_mask;
	if (before == after) return;

	auto* cmd = new FieldEditCommand<int>(
	    FieldId::Wing_DeparturePaths, _viewport->editor, tr("Edit Wing Departure Paths"), true);
	cmd->setTargetKey(std::to_string(_model->getCurrentWingIndex()));
	cmd->addEntry(before, after, [wingName](const int& v) {
		const int w = findWingByName(wingName);
		if (w >= 0) Wings[w].departure_path_mask = v;
	});
	_fredView->mainUndoStack()->push(cmd);
}

void WingEditorDialog::on_customWarpoutButton_clicked()
{
	if (!_model->wingIsValid()) return;
	const int wingIdx   = _model->getCurrentWingIndex();
	const SCP_string wingName = _model->getWingName();

	// Capture warpout_params_index for each member before the dialog runs
	SCP_vector<WingMemberWarpOut> before;
	const auto& wp = Wings[wingIdx];
	for (int i = 0; i < wp.wave_count; ++i) {
		const int si = wp.ship_index[i];
		if (si < 0 || si >= MAX_SHIPS || Ships[si].objnum < 0) continue;
		before.push_back({ Objects[Ships[si].objnum].signature, Ships[si].warpout_params_index });
	}

	auto dlg = fso::fred::dialogs::ShipCustomWarpDialog(this, _viewport, true, wingIdx, true);
	if (dlg.exec() != QDialog::Accepted) return;

	// Capture after state (locate wing by name in case index shifted)
	const int wi = findWingByName(wingName);
	if (wi < 0) return;
	SCP_vector<WingMemberWarpOut> after;
	const auto& wp2 = Wings[wi];
	for (int i = 0; i < wp2.wave_count; ++i) {
		const int si = wp2.ship_index[i];
		if (si < 0 || si >= MAX_SHIPS || Ships[si].objnum < 0) continue;
		after.push_back({ Objects[Ships[si].objnum].signature, Ships[si].warpout_params_index });
	}

	if (before == after) return;

	auto* cmd = new FieldEditCommand<SCP_vector<WingMemberWarpOut>>(
	    FieldId::Wing_WarpoutParams, _viewport->editor, tr("Change Wing Warp-Out Params"), true);
	cmd->setTargetKey(std::to_string(_model->getCurrentWingIndex()));
	cmd->setNoMerge();
	cmd->addEntry(std::move(before), std::move(after), [](const SCP_vector<WingMemberWarpOut>& v) {
		for (const auto& m : v) {
			const int objNum = obj_get_by_signature(m.signature);
			if (objNum < 0) continue;
			const int si = Objects[objNum].instance;
			if (si < 0 || si >= MAX_SHIPS) continue;
			Ships[si].warpout_params_index = m.warpout_params_index;
		}
	});
	_fredView->mainUndoStack()->push(cmd);
}

void WingEditorDialog::on_departureTree_modified()
{
	if (!_model->wingIsValid()) {
		_model->setDepartureTree(ui->departureTree->_model.save_tree());
		return;
	}

	const SCP_string wingName = _model->getWingName();
	auto* cmd = new SexpCueEditCommand(_viewport->editor, tr("Edit Wing Departure Cue"), true);
	cmd->addOwner(Wings[_model->getCurrentWingIndex()].departure_cue,
		[wingName]() {
			const int w = findWingByName(wingName);
			return w < 0 ? -1 : Wings[w].departure_cue;
		},
		[wingName](int formula) {
			const int w = findWingByName(wingName);
			if (w >= 0)
				Wings[w].departure_cue = formula;
		});

	const int newFormula = ui->departureTree->_model.save_tree();
	_model->setDepartureTree(newFormula);
	cmd->captureAfter(newFormula);
	_fredView->mainUndoStack()->push(cmd);
}

void WingEditorDialog::on_noDepartureWarpCheckBox_toggled(bool checked)
{
	if (!_model->wingIsValid()) return;
	const SCP_string wingName = _model->getWingName();
	const bool before = Wings[_model->getCurrentWingIndex()].flags[Ship::Wing_Flags::No_departure_warp];
	_model->setNoDepartureWarpFlag(checked);
	const bool after = Wings[_model->getCurrentWingIndex()].flags[Ship::Wing_Flags::No_departure_warp];
	if (before == after) return;

	auto* cmd = new FieldEditCommand<bool>(
	    FieldId::Wing_NoDepartureWarp, _viewport->editor, tr("Toggle No Departure Warp"), true);
	cmd->setTargetKey(std::to_string(_model->getCurrentWingIndex()));
	cmd->addEntry(before, after, [wingName](const bool& v) {
		const int w = findWingByName(wingName);
		if (w < 0) return;
		if (v) Wings[w].flags.set(Ship::Wing_Flags::No_departure_warp);
		else Wings[w].flags.remove(Ship::Wing_Flags::No_departure_warp);
	});
	_fredView->mainUndoStack()->push(cmd);
	_viewport->editor->missionChanged();
}

void WingEditorDialog::on_noDepartureWarpAdjustCheckbox_toggled(bool checked)
{
	if (!_model->wingIsValid()) return;
	const SCP_string wingName = _model->getWingName();
	const bool before = Wings[_model->getCurrentWingIndex()].flags[Ship::Wing_Flags::Same_departure_warp_when_docked];
	_model->setNoDepartureWarpAdjustFlag(checked);
	const bool after = Wings[_model->getCurrentWingIndex()].flags[Ship::Wing_Flags::Same_departure_warp_when_docked];
	if (before == after) return;

	auto* cmd = new FieldEditCommand<bool>(
	    FieldId::Wing_NoDepartureWarpAdj, _viewport->editor, tr("Toggle Same Departure Warp When Docked"), true);
	cmd->setTargetKey(std::to_string(_model->getCurrentWingIndex()));
	cmd->addEntry(before, after, [wingName](const bool& v) {
		const int w = findWingByName(wingName);
		if (w < 0) return;
		if (v) Wings[w].flags.set(Ship::Wing_Flags::Same_departure_warp_when_docked);
		else Wings[w].flags.remove(Ship::Wing_Flags::Same_departure_warp_when_docked);
	});
	_fredView->mainUndoStack()->push(cmd);
	_viewport->editor->missionChanged();
}

// ---------------------------------------------------------------------------
// Sexp help text
// ---------------------------------------------------------------------------

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
