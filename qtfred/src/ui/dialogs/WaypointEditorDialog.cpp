#include <QtWidgets/QTextEdit>
#include <ui/util/DialogUndo.h>
#include "ui/dialogs/WaypointEditorDialog.h"
#include "ui/util/SignalBlockers.h"
#include "ui_WaypointEditorDialog.h"

#include <globalincs/globals.h>
#include <mission/commands/FredCommands.h>
#include <mission/util.h>
#include <object/waypoint.h>
#include <ui/util/menu.h>
#include <object/object.h>

namespace fso::fred::dialogs {

// Full color state for a single waypoint path — used by FieldEditCommand<WpColorState>
// so that toggling custom-color can be atomically undone (set_color / clear_color pair).
namespace {
struct WpColorState {
	bool hasCustomColor;
	int  r, g, b;
};

// Merge-identity key for FieldEditCommand: first-waypoint signatures of the
// selected paths. Edits merge only while the path selection is unchanged.
SCP_string selectionKey(const SCP_vector<int>& pathIndices)
{
	SCP_string key;
	for (int idx : pathIndices) {
		if (!SCP_vector_inbounds(Waypoint_lists, idx)) continue;
		const auto& wl = Waypoint_lists[idx];
		if (wl.get_waypoints().empty()) continue;
		key += std::to_string(Objects[wl.get_waypoints().front().get_objnum()].signature);
		key += ',';
	}
	return key;
}
} // namespace

WaypointEditorDialog::WaypointEditorDialog(FredView* parent, EditorViewport* viewport) :
	QDialog(parent),
	_fredView(parent),
	_viewport(viewport),
	ui(new Ui::WaypointEditorDialog()),
	_model(new WaypointEditorDialogModel(this, viewport))
{
	this->setFocus();

	ui->setupUi(this);
	util::installMainStackUndoShortcuts(this, _fredView->mainUndoStack());

	ui->nameEdit->setMaxLength(NAME_LENGTH - 1);

	// -1 is the "mixed selection" sentinel; shown as blank via specialValueText.
	for (auto* sb : {ui->colorRSpinBox, ui->colorGSpinBox, ui->colorBSpinBox}) {
		sb->setMinimum(-1);
		sb->setSpecialValueText(" ");
	}

	initializeUi();
	updateUi();

	connect(_model.get(), &WaypointEditorDialogModel::waypointPathMarkingChanged, this, [this] {
		initializeUi();
		updateUi();
	});

	// "Select Waypoint Path" menu: jump the editor to any path in the mission.
	auto* model = _model.get();
	util::installSelectMenu(
		this,
		[]() {
			std::vector<util::SelectMenuEntry> entries;
				entries.reserve(Waypoint_lists.size());
			for (int i = 0; i < static_cast<int>(Waypoint_lists.size()); i++) {
				entries.push_back({QString::fromUtf8(Waypoint_lists[i].get_name()), i});
			}
			return entries;
		},
		[model]() { return model->hasMultipleSelection() ? -1 : model->getSelectedPathIndex(); },
		[model](int idx) { model->selectWaypointPathByIndex(idx); },
		tr("&Select Waypoint Path"));

	// Resize the dialog to the minimum size
	resize(QDialog::sizeHint());
}

WaypointEditorDialog::~WaypointEditorDialog() = default;

void WaypointEditorDialog::changeEvent(QEvent* e)
{
	if (e->type() == QEvent::ActivationChange && isActiveWindow())
		_fredView->undoGroup()->setActiveStack(_fredView->mainUndoStack());
	QDialog::changeEvent(e);
}

void WaypointEditorDialog::initializeUi()
{
	util::SignalBlockers blockers(this);

	ui->layerCombo->clear();
	for (const auto& name : _viewport->getLayerNames()) {
		ui->layerCombo->addItem(QString::fromStdString(name), QString::fromStdString(name));
	}

	const bool enabled = _model->hasValidSelection();
	const bool hasAny = _model->hasAnyPathsInMission();
	const bool multiSelect = _model->hasMultipleSelection();

	ui->nameEdit->setEnabled(enabled && !multiSelect);
	ui->noDrawLinesCheck->setEnabled(enabled);
	ui->customColorCheck->setEnabled(enabled);
	ui->layerCombo->setEnabled(enabled);
	ui->prevPathButton->setEnabled(hasAny);
	ui->nextPathButton->setEnabled(hasAny);

	if (multiSelect) {
		setWindowTitle(QString("Edit %1 Waypoint Paths").arg(_model->getSelectionCount()));
	} else {
		setWindowTitle("Waypoint Path Editor");
	}
}

void WaypointEditorDialog::updateUi()
{
	util::SignalBlockers blockers(this);
	ui->nameEdit->setText(QString::fromStdString(_model->getCurrentName()));
	ui->layerCombo->setCurrentIndex(ui->layerCombo->findData(QString::fromStdString(_model->getLayer())));

	const int noDrawState = _model->getNoDrawLinesState();
	ui->noDrawLinesCheck->setTristate(noDrawState == Qt::PartiallyChecked);
	ui->noDrawLinesCheck->setCheckState(static_cast<Qt::CheckState>(noDrawState));

	const int customColorState = _model->getHasCustomColorState();
	ui->customColorCheck->setTristate(customColorState == Qt::PartiallyChecked);
	ui->customColorCheck->setCheckState(static_cast<Qt::CheckState>(customColorState));

	ui->colorRSpinBox->setValue(_model->isColorRMixed() ? -1 : _model->getColorR());
	ui->colorGSpinBox->setValue(_model->isColorGMixed() ? -1 : _model->getColorG());
	ui->colorBSpinBox->setValue(_model->isColorBMixed() ? -1 : _model->getColorB());

	const bool customResolved = customColorState == Qt::Checked;
	const bool colorEnabled = _model->hasValidSelection() && customResolved;
	ui->colorRSpinBox->setEnabled(colorEnabled);
	ui->colorGSpinBox->setEnabled(colorEnabled);
	ui->colorBSpinBox->setEnabled(colorEnabled);

	updateColorSwatch();
}

void WaypointEditorDialog::updateColorSwatch()
{
	if (_model->hasAnyColorMixed()) {
		ui->colorSwatch->setText("?");
		ui->colorSwatch->setAlignment(Qt::AlignCenter);
		ui->colorSwatch->setStyleSheet("background: #888; color: white;"
		                               "border: 1px solid #444; border-radius: 3px;");
		return;
	}
	ui->colorSwatch->setText("");
	ui->colorSwatch->setStyleSheet(QString("background: rgb(%1,%2,%3);"
	                                       "border: 1px solid #444; border-radius: 3px;")
	        .arg(_model->getColorR())
	        .arg(_model->getColorG())
	        .arg(_model->getColorB()));
}

void WaypointEditorDialog::on_prevPathButton_clicked()
{
	_model->selectPreviousPath();
}

void WaypointEditorDialog::on_nextPathButton_clicked()
{
	_model->selectNextPath();
}

void WaypointEditorDialog::on_nameEdit_editingFinished()
{
	const SCP_string newName = ui->nameEdit->text().toUtf8().constData();
	const SCP_string oldName = _model->getCurrentName();
	if (newName == oldName) return;

	if (!_model->setCurrentName(newName)) {
		util::SignalBlockers blockers(this);
		ui->nameEdit->setText(QString::fromStdString(_model->getCurrentName()));
		return;
	}

	// Waypoint paths have no single object signature; pass objNum=-1 as sentinel.
	// RenameObjectCommand::applyName() detects _signature==-1 and calls rename_waypoint_list().
	_fredView->mainUndoStack()->push(
	    new RenameObjectCommand(-1, oldName, newName, _viewport->editor, /*skipFirstRedo=*/true));

	// Clear the field's text-undo history so Ctrl+Z hits the mission stack, not Qt's text widget.
	util::SignalBlockers b(this);
	ui->nameEdit->setText(QString::fromStdString(_model->getCurrentName()));
}

void WaypointEditorDialog::on_noDrawLinesCheck_clicked()
{
	const bool newVal = ui->noDrawLinesCheck->isChecked();

	// Capture per-path before values by first-waypoint signature for stable identity.
	SCP_vector<std::pair<int, bool>> pathsBefore;
	for (int idx : _model->getSelectedPathIndices()) {
		if (!SCP_vector_inbounds(Waypoint_lists, idx)) continue;
		const auto& wl = Waypoint_lists[idx];
		if (wl.get_waypoints().empty()) continue;
		const int firstObjNum = wl.get_waypoints().front().get_objnum();
		pathsBefore.emplace_back(Objects[firstObjNum].signature, wl.get_no_draw_lines());
	}

	_model->setNoDrawLines(newVal);
	updateUi();

	auto* cmd = new FieldEditCommand<bool>(
	    FieldId::WP_NoDrawLines, _viewport->editor,
	    tr("Change Waypoint No-Draw Lines"), /*skipFirstRedo=*/true);
	cmd->setTargetKey(selectionKey(_model->getSelectedPathIndices()));
	for (auto& [firstWpSig, before] : pathsBefore) {
		cmd->addEntry(before, newVal,
		    [firstWpSig = firstWpSig](const bool& v) {
			    const int objNum = obj_get_by_signature(firstWpSig);
			    if (objNum < 0) return;
			    const int li = calc_waypoint_list_index(Objects[objNum].instance);
			    if (!SCP_vector_inbounds(Waypoint_lists, li)) return;
			    Waypoint_lists[li].set_no_draw_lines(v);
		    });
	}
	if (cmd->isEmpty()) { delete cmd; return; }
	_fredView->mainUndoStack()->push(cmd);
}

void WaypointEditorDialog::on_customColorCheck_clicked()
{
	const bool newVal = ui->customColorCheck->isChecked();

	// Capture full color state per path (needed to restore set_color/clear_color correctly on undo).
	SCP_vector<std::pair<int, WpColorState>> pathsBefore;
	for (int idx : _model->getSelectedPathIndices()) {
		if (!SCP_vector_inbounds(Waypoint_lists, idx)) continue;
		const auto& wl = Waypoint_lists[idx];
		if (wl.get_waypoints().empty()) continue;
		const int firstObjNum = wl.get_waypoints().front().get_objnum();
		pathsBefore.push_back({ Objects[firstObjNum].signature,
		    { wl.get_has_custom_color(), wl.get_color_r(), wl.get_color_g(), wl.get_color_b() } });
	}

	_model->setHasCustomColor(newVal);
	updateUi();

	auto* cmd = new FieldEditCommand<WpColorState>(
	    FieldId::WP_HasCustomColor, _viewport->editor,
	    tr("Change Waypoint Custom Color"), /*skipFirstRedo=*/true);
	cmd->setTargetKey(selectionKey(_model->getSelectedPathIndices()));
	for (auto& [firstWpSig, before] : pathsBefore) {
		const int objNum = obj_get_by_signature(firstWpSig);
		if (objNum < 0) continue;
		const int li = calc_waypoint_list_index(Objects[objNum].instance);
		if (!SCP_vector_inbounds(Waypoint_lists, li)) continue;
		const auto& wl = Waypoint_lists[li];
		WpColorState after = { wl.get_has_custom_color(), wl.get_color_r(), wl.get_color_g(), wl.get_color_b() };
		cmd->addEntry(before, after,
		    [firstWpSig = firstWpSig](const WpColorState& s) {
			    const int cur = obj_get_by_signature(firstWpSig);
			    if (cur < 0) return;
			    const int li2 = calc_waypoint_list_index(Objects[cur].instance);
			    if (!SCP_vector_inbounds(Waypoint_lists, li2)) return;
			    auto& path = Waypoint_lists[li2];
			    if (s.hasCustomColor)
				    path.set_color(s.r, s.g, s.b);
			    else
				    path.clear_color();
		    });
	}
	if (cmd->isEmpty()) { delete cmd; return; }
	_fredView->mainUndoStack()->push(cmd);
}

void WaypointEditorDialog::on_colorRSpinBox_valueChanged(int value)
{
	if (value < 0) return;

	SCP_vector<std::pair<int, int>> pathsBefore;
	for (int idx : _model->getSelectedPathIndices()) {
		if (!SCP_vector_inbounds(Waypoint_lists, idx)) continue;
		const auto& wl = Waypoint_lists[idx];
		if (!wl.get_has_custom_color()) continue;
		if (wl.get_waypoints().empty()) continue;
		const int firstObjNum = wl.get_waypoints().front().get_objnum();
		pathsBefore.emplace_back(Objects[firstObjNum].signature, wl.get_color_r());
	}

	_model->setColorR(value);
	updateColorSwatch();

	auto* cmd = new FieldEditCommand<int>(
	    FieldId::WP_ColorR, _viewport->editor, tr("Change Waypoint Color"), /*skipFirstRedo=*/true);
	cmd->setTargetKey(selectionKey(_model->getSelectedPathIndices()));
	for (auto& [firstWpSig, before] : pathsBefore) {
		const int objNum = obj_get_by_signature(firstWpSig);
		if (objNum < 0) continue;
		const int li = calc_waypoint_list_index(Objects[objNum].instance);
		if (!SCP_vector_inbounds(Waypoint_lists, li)) continue;
		const int after = Waypoint_lists[li].get_color_r();
		cmd->addEntry(before, after,
		    [firstWpSig = firstWpSig](const int& r) {
			    const int cur = obj_get_by_signature(firstWpSig);
			    if (cur < 0) return;
			    const int li2 = calc_waypoint_list_index(Objects[cur].instance);
			    if (!SCP_vector_inbounds(Waypoint_lists, li2)) return;
			    auto& path = Waypoint_lists[li2];
			    path.set_color(r, path.get_color_g(), path.get_color_b());
		    });
	}
	if (cmd->isEmpty()) { delete cmd; return; }
	_fredView->mainUndoStack()->push(cmd);
}

void WaypointEditorDialog::on_colorGSpinBox_valueChanged(int value)
{
	if (value < 0) return;

	SCP_vector<std::pair<int, int>> pathsBefore;
	for (int idx : _model->getSelectedPathIndices()) {
		if (!SCP_vector_inbounds(Waypoint_lists, idx)) continue;
		const auto& wl = Waypoint_lists[idx];
		if (!wl.get_has_custom_color()) continue;
		if (wl.get_waypoints().empty()) continue;
		const int firstObjNum = wl.get_waypoints().front().get_objnum();
		pathsBefore.emplace_back(Objects[firstObjNum].signature, wl.get_color_g());
	}

	_model->setColorG(value);
	updateColorSwatch();

	auto* cmd = new FieldEditCommand<int>(
	    FieldId::WP_ColorG, _viewport->editor, tr("Change Waypoint Color"), /*skipFirstRedo=*/true);
	cmd->setTargetKey(selectionKey(_model->getSelectedPathIndices()));
	for (auto& [firstWpSig, before] : pathsBefore) {
		const int objNum = obj_get_by_signature(firstWpSig);
		if (objNum < 0) continue;
		const int li = calc_waypoint_list_index(Objects[objNum].instance);
		if (!SCP_vector_inbounds(Waypoint_lists, li)) continue;
		const int after = Waypoint_lists[li].get_color_g();
		cmd->addEntry(before, after,
		    [firstWpSig = firstWpSig](const int& g) {
			    const int cur = obj_get_by_signature(firstWpSig);
			    if (cur < 0) return;
			    const int li2 = calc_waypoint_list_index(Objects[cur].instance);
			    if (!SCP_vector_inbounds(Waypoint_lists, li2)) return;
			    auto& path = Waypoint_lists[li2];
			    path.set_color(path.get_color_r(), g, path.get_color_b());
		    });
	}
	if (cmd->isEmpty()) { delete cmd; return; }
	_fredView->mainUndoStack()->push(cmd);
}

void WaypointEditorDialog::on_colorBSpinBox_valueChanged(int value)
{
	if (value < 0) return;

	SCP_vector<std::pair<int, int>> pathsBefore;
	for (int idx : _model->getSelectedPathIndices()) {
		if (!SCP_vector_inbounds(Waypoint_lists, idx)) continue;
		const auto& wl = Waypoint_lists[idx];
		if (!wl.get_has_custom_color()) continue;
		if (wl.get_waypoints().empty()) continue;
		const int firstObjNum = wl.get_waypoints().front().get_objnum();
		pathsBefore.emplace_back(Objects[firstObjNum].signature, wl.get_color_b());
	}

	_model->setColorB(value);
	updateColorSwatch();

	auto* cmd = new FieldEditCommand<int>(
	    FieldId::WP_ColorB, _viewport->editor, tr("Change Waypoint Color"), /*skipFirstRedo=*/true);
	cmd->setTargetKey(selectionKey(_model->getSelectedPathIndices()));
	for (auto& [firstWpSig, before] : pathsBefore) {
		const int objNum = obj_get_by_signature(firstWpSig);
		if (objNum < 0) continue;
		const int li = calc_waypoint_list_index(Objects[objNum].instance);
		if (!SCP_vector_inbounds(Waypoint_lists, li)) continue;
		const int after = Waypoint_lists[li].get_color_b();
		cmd->addEntry(before, after,
		    [firstWpSig = firstWpSig](const int& b) {
			    const int cur = obj_get_by_signature(firstWpSig);
			    if (cur < 0) return;
			    const int li2 = calc_waypoint_list_index(Objects[cur].instance);
			    if (!SCP_vector_inbounds(Waypoint_lists, li2)) return;
			    auto& path = Waypoint_lists[li2];
			    path.set_color(path.get_color_r(), path.get_color_g(), b);
		    });
	}
	if (cmd->isEmpty()) { delete cmd; return; }
	_fredView->mainUndoStack()->push(cmd);
}

void WaypointEditorDialog::on_layerCombo_currentIndexChanged(int index)
{
	if (index < 0) return;
	const SCP_string newLayer =
	    ui->layerCombo->itemData(index).toString().toStdString();

	// Build MoveLayerCommand with one entry per selected path (first-waypoint signature).
	// MoveLayerCommand::redo() calls moveObjectToLayer() on that waypoint, which propagates
	// to the entire path via setObjectLayerByIndex (OBJ_WAYPOINT syncs all waypoints in path).
	SCP_vector<ObjectLayerChange> changes;
	for (int idx : _model->getSelectedPathIndices()) {
		if (!SCP_vector_inbounds(Waypoint_lists, idx)) continue;
		const auto& wl = Waypoint_lists[idx];
		if (wl.get_waypoints().empty()) continue;
		const int firstObjNum = wl.get_waypoints().front().get_objnum();
		SCP_string oldLayer = _viewport->getObjectLayerName(firstObjNum);
		if (oldLayer == newLayer) continue;
		changes.push_back({ Objects[firstObjNum].signature, std::move(oldLayer), newLayer });
	}
	if (changes.empty()) return;
	_fredView->mainUndoStack()->push(
	    new MoveLayerCommand(std::move(changes), _viewport, _viewport->editor));
}

} // namespace fso::fred::dialogs
