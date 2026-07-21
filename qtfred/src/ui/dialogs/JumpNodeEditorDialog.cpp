#include "ui/dialogs/JumpNodeEditorDialog.h"
#include <ui/util/DialogUndo.h>
#include "ui/util/SignalBlockers.h"
#include "ui_JumpNodeEditorDialog.h"

#include <globalincs/globals.h>
#include <jumpnode/jumpnode.h>
#include <mission/util.h>
#include <ui/util/menu.h>
#include <object/object.h>

namespace fso::fred::dialogs {

namespace {
// Merge-identity key for FieldEditCommand: signatures of the selected nodes.
// Edits of the same field merge only while the selection is unchanged.
SCP_string selectionKey(const SCP_vector<int>& objNums)
{
	SCP_string key;
	for (int objNum : objNums) {
		key += std::to_string(Objects[objNum].signature);
		key += ',';
	}
	return key;
}
} // namespace

JumpNodeEditorDialog::JumpNodeEditorDialog(FredView* parent, EditorViewport* viewport)
	: QDialog(parent), _fredView(parent), _viewport(viewport),
	  ui(new Ui::JumpNodeEditorDialog()),
	  _model(new JumpNodeEditorDialogModel(this, viewport))
{
	this->setFocus();

	ui->setupUi(this);
	util::installMainStackUndoShortcuts(this, _fredView->mainUndoStack());

	ui->nameLineEdit->setMaxLength(NAME_LENGTH - 1);
	ui->displayNameLineEdit->setMaxLength(NAME_LENGTH - 1);
	ui->modelFileLineEdit->setMaxLength(MAX_FILENAME_LEN - 1);

	// -1 is the "mixed selection" sentinel... shown as blank via specialValueText.
	for (auto* sb : {ui->redSpinBox, ui->greenSpinBox, ui->blueSpinBox, ui->alphaSpinBox}) {
		sb->setMinimum(-1);
		sb->setSpecialValueText(" ");
	}

	initializeUi();
	updateUi();

	connect(_model.get(), &JumpNodeEditorDialogModel::jumpNodeMarkingChanged, this, [this] {
		initializeUi();
		updateUi();
	});

	// "Select Jump Node" menu: jump the editor to any jump node in the mission.
	Editor* editor = viewport->editor;
	util::installSelectMenu(
		this,
		[]() {
			std::vector<util::SelectMenuEntry> entries;
			for (const auto& jn : Jump_nodes) {
				entries.push_back({QString::fromUtf8(jn.GetName()), jn.GetSCPObjectNumber()});
			}
			return entries;
		},
		[this, editor]() { return _model->hasMultipleSelection() ? -1 : editor->currentObject; },
		[editor](int objnum) {
			editor->unmark_all();
			editor->selectObject(objnum);
		},
		tr("&Select Jump Node"));

	// Resize the dialog to the minimum size
	resize(QDialog::sizeHint());
}

JumpNodeEditorDialog::~JumpNodeEditorDialog() = default;

void JumpNodeEditorDialog::changeEvent(QEvent* e)
{
	if (e->type() == QEvent::ActivationChange && isActiveWindow())
		_fredView->undoGroup()->setActiveStack(_fredView->mainUndoStack());
	QDialog::changeEvent(e);
}

void JumpNodeEditorDialog::initializeUi()
{
	util::SignalBlockers blockers(this);

	ui->layerCombo->clear();
	for (const auto& name : _viewport->getLayerNames()) {
		ui->layerCombo->addItem(QString::fromStdString(name), QString::fromStdString(name));
	}

	const bool enabled    = _model->hasValidSelection();
	const bool hasAny     = _model->hasAnyNodesInMission();
	const bool multiSelect = _model->hasMultipleSelection();

	ui->nameLineEdit->setEnabled(enabled && !multiSelect);
	ui->displayNameLineEdit->setEnabled(enabled);
	ui->modelFileLineEdit->setEnabled(enabled);
	ui->redSpinBox->setEnabled(enabled);
	ui->greenSpinBox->setEnabled(enabled);
	ui->blueSpinBox->setEnabled(enabled);
	ui->alphaSpinBox->setEnabled(enabled);
	ui->hiddenByDefaultCheckBox->setEnabled(enabled);
	ui->layerCombo->setEnabled(enabled);
	ui->prevNodeButton->setEnabled(hasAny);
	ui->nextNodeButton->setEnabled(hasAny);

	if (multiSelect)
		setWindowTitle(QString("Edit %1 Jump Nodes").arg(_model->getSelectionCount()));
	else
		setWindowTitle("Jump Node Editor");
}

void JumpNodeEditorDialog::updateUi()
{
	util::SignalBlockers blockers(this);

	ui->nameLineEdit->setText(QString::fromStdString(_model->getName()));
	ui->displayNameLineEdit->setText(QString::fromStdString(_model->getDisplayName()));
	ui->modelFileLineEdit->setText(QString::fromStdString(_model->getModelFilename()));

	ui->redSpinBox->setValue(_model->isColorRMixed() ? -1 : _model->getColorR());
	ui->greenSpinBox->setValue(_model->isColorGMixed() ? -1 : _model->getColorG());
	ui->blueSpinBox->setValue(_model->isColorBMixed() ? -1 : _model->getColorB());
	ui->alphaSpinBox->setValue(_model->isColorAMixed() ? -1 : _model->getColorA());

	const int hiddenState = _model->getHiddenState();
	ui->hiddenByDefaultCheckBox->setTristate(hiddenState == Qt::PartiallyChecked);
	ui->hiddenByDefaultCheckBox->setCheckState(static_cast<Qt::CheckState>(hiddenState));

	ui->layerCombo->setCurrentIndex(
	    ui->layerCombo->findData(QString::fromStdString(_model->getLayer())));

	updateColorSwatch();
}

void JumpNodeEditorDialog::updateColorSwatch()
{
	if (_model->hasAnyColorMixed()) {
		ui->colorSwatch->setText("?");
		ui->colorSwatch->setAlignment(Qt::AlignCenter);
		ui->colorSwatch->setStyleSheet("background: #888; color: white;"
		                               "border: 1px solid #444; border-radius: 3px;");
		return;
	}
	ui->colorSwatch->setText("");
	ui->colorSwatch->setStyleSheet(QString("background: rgba(%1,%2,%3,%4);"
	                                       "border: 1px solid #444; border-radius: 3px;")
	        .arg(_model->getColorR())
	        .arg(_model->getColorG())
	        .arg(_model->getColorB())
	        .arg(_model->getColorA()));
}

void JumpNodeEditorDialog::on_prevNodeButton_clicked() { _model->selectPreviousNode(); }
void JumpNodeEditorDialog::on_nextNodeButton_clicked() { _model->selectNextNode(); }

void JumpNodeEditorDialog::on_nameLineEdit_editingFinished()
{
	const SCP_string newName = ui->nameLineEdit->text().toUtf8().constData();
	const SCP_string oldName = _model->getName();
	if (newName == oldName) return;
	if (!_model->setName(newName)) {
		util::SignalBlockers b(this);
		ui->nameLineEdit->setText(QString::fromStdString(_model->getName()));
		return;
	}
	const auto& objNums = _model->getSelectedObjNums();
	if (objNums.empty()) return;
	_fredView->mainUndoStack()->push(
	    new RenameObjectCommand(objNums[0], oldName, newName, _viewport->editor, true));

	// Edit is committed to the mission undo stack; clear the field's own text-undo history
	// (re-setting the text clears it) so a following Ctrl+Z undoes the mission edit, not the typing.
	util::SignalBlockers b(this);
	ui->nameLineEdit->setText(QString::fromStdString(_model->getName()));
}

void JumpNodeEditorDialog::on_displayNameLineEdit_editingFinished()
{
	const SCP_string newVal = ui->displayNameLineEdit->text().toUtf8().constData();
	if (newVal == _model->getDisplayName()) return;

	// Capture before values from live nodes
	SCP_vector<std::pair<int, SCP_string>> nodes;
	for (int objNum : _model->getSelectedObjNums()) {
		CJumpNode* jnp = jumpnode_get_by_objnum(objNum);
		if (!jnp) continue;
		nodes.emplace_back(objNum, SCP_string(jnp->GetDisplayName()));
	}

	if (!_model->setDisplayName(newVal)) {
		util::SignalBlockers b(this);
		ui->displayNameLineEdit->setText(QString::fromStdString(_model->getDisplayName()));
		return;
	}

	auto* cmd = new FieldEditCommand<SCP_string>(
	    FieldId::JN_DisplayName, _viewport->editor,
	    tr("Change Jump Node Display Name"), true);
	cmd->setTargetKey(selectionKey(_model->getSelectedObjNums()));
	for (const auto& kv : nodes) {
		const int objNum = kv.first;
		CJumpNode* jnp = jumpnode_get_by_objnum(objNum);
		if (!jnp) continue;
		const int sig = Objects[objNum].signature;
		cmd->addEntry(kv.second, SCP_string(jnp->GetDisplayName()),
		    [sig](const SCP_string& v) {
			    const int cur = obj_get_by_signature(sig);
			    CJumpNode* j  = (cur >= 0) ? jumpnode_get_by_objnum(cur) : nullptr;
			    if (j) j->SetDisplayName(v.c_str());
		    });
	}
	if (cmd->isEmpty()) { delete cmd; return; }
	_fredView->mainUndoStack()->push(cmd);

	// Edit committed; clear the field's text-undo history so Ctrl+Z hits the mission stack.
	util::SignalBlockers b(this);
	ui->displayNameLineEdit->setText(QString::fromStdString(_model->getDisplayName()));
}

void JumpNodeEditorDialog::on_modelFileLineEdit_editingFinished()
{
	const SCP_string newVal = ui->modelFileLineEdit->text().toUtf8().constData();
	if (newVal == _model->getModelFilename()) return;

	SCP_vector<std::pair<int, SCP_string>> nodes;
	for (int objNum : _model->getSelectedObjNums()) {
		CJumpNode* jnp = jumpnode_get_by_objnum(objNum);
		if (!jnp) continue;
		nodes.emplace_back(objNum, SCP_string(jnp->GetModelFilename()));
	}

	if (!_model->setModelFilename(newVal)) {
		util::SignalBlockers b(this);
		ui->modelFileLineEdit->setText(QString::fromStdString(_model->getModelFilename()));
		return;
	}

	auto* cmd = new FieldEditCommand<SCP_string>(
	    FieldId::JN_ModelFile, _viewport->editor,
	    tr("Change Jump Node Model"), true);
	cmd->setTargetKey(selectionKey(_model->getSelectedObjNums()));
	for (const auto& kv : nodes) {
		const int objNum = kv.first;
		CJumpNode* jnp = jumpnode_get_by_objnum(objNum);
		if (!jnp) continue;
		const int sig = Objects[objNum].signature;
		cmd->addEntry(kv.second, SCP_string(jnp->GetModelFilename()),
		    [sig](const SCP_string& v) {
			    const int cur = obj_get_by_signature(sig);
			    CJumpNode* j  = (cur >= 0) ? jumpnode_get_by_objnum(cur) : nullptr;
			    if (!j) return;
			    if (v == JN_DEFAULT_MODEL) j->ResetToDefaultModel();
			    else j->SetModel(v.c_str());
		    });
	}
	if (cmd->isEmpty()) { delete cmd; return; }
	_fredView->mainUndoStack()->push(cmd);

	// Edit committed; clear the field's text-undo history so Ctrl+Z hits the mission stack.
	util::SignalBlockers b(this);
	ui->modelFileLineEdit->setText(QString::fromStdString(_model->getModelFilename()));
}

void JumpNodeEditorDialog::on_redSpinBox_valueChanged(int value)
{
	if (value == -1) return;

	SCP_vector<std::pair<int, int>> nodes;
	for (int objNum : _model->getSelectedObjNums()) {
		CJumpNode* jnp = jumpnode_get_by_objnum(objNum);
		if (!jnp) continue;
		nodes.emplace_back(objNum, static_cast<int>(jnp->GetColor().red));
	}

	_model->setColorR(value);
	updateColorSwatch();

	auto* cmd = new FieldEditCommand<int>(
	    FieldId::JN_ColorR, _viewport->editor, tr("Change Jump Node Color"), true);
	cmd->setTargetKey(selectionKey(_model->getSelectedObjNums()));
	for (const auto& kv : nodes) {
		const int objNum = kv.first;
		CJumpNode* jnp = jumpnode_get_by_objnum(objNum);
		if (!jnp) continue;
		const int sig = Objects[objNum].signature;
		cmd->addEntry(kv.second, static_cast<int>(jnp->GetColor().red),
		    [sig](const int& r) {
			    const int cur = obj_get_by_signature(sig);
			    CJumpNode* j  = (cur >= 0) ? jumpnode_get_by_objnum(cur) : nullptr;
			    if (!j) return;
			    const color& c = j->GetColor();
			    j->SetAlphaColor(r, c.green, c.blue, c.alpha);
		    });
	}
	if (cmd->isEmpty()) { delete cmd; return; }
	_fredView->mainUndoStack()->push(cmd);
}

void JumpNodeEditorDialog::on_greenSpinBox_valueChanged(int value)
{
	if (value == -1) return;

	SCP_vector<std::pair<int, int>> nodes;
	for (int objNum : _model->getSelectedObjNums()) {
		CJumpNode* jnp = jumpnode_get_by_objnum(objNum);
		if (!jnp) continue;
		nodes.emplace_back(objNum, static_cast<int>(jnp->GetColor().green));
	}

	_model->setColorG(value);
	updateColorSwatch();

	auto* cmd = new FieldEditCommand<int>(
	    FieldId::JN_ColorG, _viewport->editor, tr("Change Jump Node Color"), true);
	cmd->setTargetKey(selectionKey(_model->getSelectedObjNums()));
	for (const auto& kv : nodes) {
		const int objNum = kv.first;
		CJumpNode* jnp = jumpnode_get_by_objnum(objNum);
		if (!jnp) continue;
		const int sig = Objects[objNum].signature;
		cmd->addEntry(kv.second, static_cast<int>(jnp->GetColor().green),
		    [sig](const int& g) {
			    const int cur = obj_get_by_signature(sig);
			    CJumpNode* j  = (cur >= 0) ? jumpnode_get_by_objnum(cur) : nullptr;
			    if (!j) return;
			    const color& c = j->GetColor();
			    j->SetAlphaColor(c.red, g, c.blue, c.alpha);
		    });
	}
	if (cmd->isEmpty()) { delete cmd; return; }
	_fredView->mainUndoStack()->push(cmd);
}

void JumpNodeEditorDialog::on_blueSpinBox_valueChanged(int value)
{
	if (value == -1) return;

	SCP_vector<std::pair<int, int>> nodes;
	for (int objNum : _model->getSelectedObjNums()) {
		CJumpNode* jnp = jumpnode_get_by_objnum(objNum);
		if (!jnp) continue;
		nodes.emplace_back(objNum, static_cast<int>(jnp->GetColor().blue));
	}

	_model->setColorB(value);
	updateColorSwatch();

	auto* cmd = new FieldEditCommand<int>(
	    FieldId::JN_ColorB, _viewport->editor, tr("Change Jump Node Color"), true);
	cmd->setTargetKey(selectionKey(_model->getSelectedObjNums()));
	for (const auto& kv : nodes) {
		const int objNum = kv.first;
		CJumpNode* jnp = jumpnode_get_by_objnum(objNum);
		if (!jnp) continue;
		const int sig = Objects[objNum].signature;
		cmd->addEntry(kv.second, static_cast<int>(jnp->GetColor().blue),
		    [sig](const int& b) {
			    const int cur = obj_get_by_signature(sig);
			    CJumpNode* j  = (cur >= 0) ? jumpnode_get_by_objnum(cur) : nullptr;
			    if (!j) return;
			    const color& c = j->GetColor();
			    j->SetAlphaColor(c.red, c.green, b, c.alpha);
		    });
	}
	if (cmd->isEmpty()) { delete cmd; return; }
	_fredView->mainUndoStack()->push(cmd);
}

void JumpNodeEditorDialog::on_alphaSpinBox_valueChanged(int value)
{
	if (value == -1) return;

	SCP_vector<std::pair<int, int>> nodes;
	for (int objNum : _model->getSelectedObjNums()) {
		CJumpNode* jnp = jumpnode_get_by_objnum(objNum);
		if (!jnp) continue;
		nodes.emplace_back(objNum, static_cast<int>(jnp->GetColor().alpha));
	}

	_model->setColorA(value);
	updateColorSwatch();

	auto* cmd = new FieldEditCommand<int>(
	    FieldId::JN_ColorA, _viewport->editor, tr("Change Jump Node Color"), true);
	cmd->setTargetKey(selectionKey(_model->getSelectedObjNums()));
	for (const auto& kv : nodes) {
		const int objNum = kv.first;
		CJumpNode* jnp = jumpnode_get_by_objnum(objNum);
		if (!jnp) continue;
		const int sig = Objects[objNum].signature;
		cmd->addEntry(kv.second, static_cast<int>(jnp->GetColor().alpha),
		    [sig](const int& a) {
			    const int cur = obj_get_by_signature(sig);
			    CJumpNode* j  = (cur >= 0) ? jumpnode_get_by_objnum(cur) : nullptr;
			    if (!j) return;
			    const color& c = j->GetColor();
			    j->SetAlphaColor(c.red, c.green, c.blue, a);
		    });
	}
	if (cmd->isEmpty()) { delete cmd; return; }
	_fredView->mainUndoStack()->push(cmd);
}

void JumpNodeEditorDialog::on_hiddenByDefaultCheckBox_clicked()
{
	// clicked() (not toggled()) so a tri-state PartiallyChecked click still routes here.
	SCP_vector<std::pair<int, bool>> nodes;
	for (int objNum : _model->getSelectedObjNums()) {
		CJumpNode* jnp = jumpnode_get_by_objnum(objNum);
		if (!jnp) continue;
		nodes.emplace_back(objNum, jnp->IsHidden());
	}

	_model->setHidden(ui->hiddenByDefaultCheckBox->isChecked());

	auto* cmd = new FieldEditCommand<bool>(
	    FieldId::JN_Hidden, _viewport->editor, tr("Change Jump Node Visibility"), true);
	cmd->setTargetKey(selectionKey(_model->getSelectedObjNums()));
	for (const auto& kv : nodes) {
		const int objNum = kv.first;
		CJumpNode* jnp = jumpnode_get_by_objnum(objNum);
		if (!jnp) continue;
		const int sig = Objects[objNum].signature;
		cmd->addEntry(kv.second, jnp->IsHidden(),
		    [sig](const bool& hidden) {
			    const int cur = obj_get_by_signature(sig);
			    CJumpNode* j  = (cur >= 0) ? jumpnode_get_by_objnum(cur) : nullptr;
			    if (j) j->SetVisibility(!hidden);
		    });
	}
	if (cmd->isEmpty()) { delete cmd; return; }
	_fredView->mainUndoStack()->push(cmd);
}

void JumpNodeEditorDialog::on_layerCombo_currentIndexChanged(int index)
{
	if (index < 0) return;
	const SCP_string newLayer =
	    ui->layerCombo->itemData(index).toString().toStdString();

	SCP_vector<ObjectLayerChange> changes;
	for (int objNum : _model->getSelectedObjNums()) {
		CJumpNode* jnp = jumpnode_get_by_objnum(objNum);
		if (!jnp) continue;
		SCP_string oldLayer = jnp->GetFredLayer();
		if (oldLayer == newLayer) continue;
		changes.push_back({ Objects[objNum].signature, std::move(oldLayer), newLayer });
	}
	if (changes.empty()) return;
	// Push MoveLayerCommand directly; its redo() applies the change so we
	// do not also call _model->setLayer().
	_fredView->mainUndoStack()->push(
	    new MoveLayerCommand(std::move(changes), _viewport, _viewport->editor));
}

} // namespace fso::fred::dialogs
