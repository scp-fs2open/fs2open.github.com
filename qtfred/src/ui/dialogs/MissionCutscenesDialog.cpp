#include <QtWidgets/QMessageBox>
#include "MissionCutscenesDialog.h"

#include "mission/commands/FredCommands.h"
#include "mission/util.h"
#include "ui/util/SignalBlockers.h"
#include "ui_MissionCutscenesDialog.h"
#include <ui/util/DialogUndo.h>
#include <QSignalBlocker>

namespace fso::fred::dialogs {

MissionCutscenesDialog::MissionCutscenesDialog(FredView* parent, EditorViewport* viewport)
	: QDialog(parent), SexpTreeEditorInterface({TreeFlags::LabeledRoot, TreeFlags::RootDeletable}),
	  ui(new Ui::MissionCutscenesDialog()), _model(new MissionCutscenesDialogModel(this, viewport)),
	  _viewport(viewport), _fredView(parent)
{
	ui->setupUi(this);

	_dialogStack = new QUndoStack(this);
	_fredView->undoGroup()->addStack(_dialogStack);
	util::setupDialogUndo(this, _fredView->undoGroup(), _dialogStack, tr("Mission Cutscenes"));

	populateCutsceneCombos();

	ui->cutsceneEventTree->initializeEditor(viewport->editor, this, viewport, parent);
	_model->setTreeControl(ui->cutsceneEventTree);

	ui->cutsceneFilename->setMaxLength(NAME_LENGTH - 1);

	ui->helpTextBox->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
	ui->helpTextBox->setVisible(viewport->Show_sexp_help_mission_cutscenes);

	connect(_model.get(), &MissionCutscenesDialogModel::modelChanged, this, &MissionCutscenesDialog::updateUi);
	connect(ui->cutsceneEventTree, &sexp_tree_view::modified, this, &MissionCutscenesDialog::onCutsceneTreeModified);

	_model->initializeData();

	load_tree();

	recreate_tree();

	// The before-state for the next tree edit: the tree mutates before
	// modified() fires, so a fresh capture at handler time would already
	// contain the edit. indexChanged fires on every push/undo/redo, keeping
	// the cache aligned with the current working state.
	_workingStateCache = _model->captureWorkingState();
	connect(_dialogStack, &QUndoStack::indexChanged, this, [this](int) {
		// accept() moves _model into the ApplyDialogCommand and then clears
		// the stack, which emits indexChanged with no model left.
		if (_model)
			_workingStateCache = _model->captureWorkingState();
	});
}

MissionCutscenesDialog::~MissionCutscenesDialog() = default;

void MissionCutscenesDialog::accept()
{
	QByteArray stateBefore = _model->captureState();
	if (_model->apply()) {
		QByteArray stateAfter = _model->captureState();
		_model->setParent(nullptr);
		_fredView->mainUndoStack()->push(
			new ApplyDialogCommand(std::move(_model), stateBefore, stateAfter,
			                       _viewport->editor, tr("Edit Mission Cutscenes")));
		_dialogStack->clear();
		_fredView->undoGroup()->setActiveStack(_fredView->mainUndoStack());
		QDialog::accept();
	}
}

void MissionCutscenesDialog::reject()
{
	if (!_model) {
		_dialogStack->clear();
		_fredView->undoGroup()->setActiveStack(_fredView->mainUndoStack());
		QDialog::reject();
		return;
	}
	if (rejectOrCloseHandler(this, _model.get(), _viewport)) {
		_dialogStack->clear();
		_fredView->undoGroup()->setActiveStack(_fredView->mainUndoStack());
		QDialog::reject();
	}
}

void MissionCutscenesDialog::closeEvent(QCloseEvent* e)
{
	reject();
	// reject() hides the dialog when it actually closes. Let that close
	// proceed (so a dialog created with WA_DeleteOnClose is destroyed),
	// and only veto it when reject() decided to keep the dialog open (e.g.
	// the user cancelled the unsaved-changes prompt).
	if (isVisible()) {
		e->ignore();
	} else {
		e->accept();
	}
}

void MissionCutscenesDialog::onCutsceneTreeModified()
{
	_model->setModified();
	if (_suppressTreeUndo)
		return;

	pushWorkingStateSnapshot(_workingStateCache, tr("Edit Cutscene Formula"));
}

void MissionCutscenesDialog::pushWorkingStateSnapshot(const QByteArray& before, const QString& label)
{
	const QByteArray after = _model->captureWorkingState();
	if (after == before)
		return;

	_dialogStack->push(new DialogSnapshotCommand(before, after,
		[this](const QByteArray& blob) {
			_suppressTreeUndo = true;
			_model->restoreWorkingState(blob);
			recreate_tree();
			updateUi();
			_suppressTreeUndo = false;
		},
		label));
}

void MissionCutscenesDialog::syncCutsceneRootLabel(int cutsceneIndex)
{
	auto& cutscenes = _model->getCutscenes();
	if (!SCP_vector_inbounds(cutscenes, cutsceneIndex))
		return;

	const int formula = cutscenes[cutsceneIndex].formula;
	for (int i = 0; i < ui->cutsceneEventTree->topLevelItemCount(); ++i) {
		auto* item = ui->cutsceneEventTree->topLevelItem(i);
		if (item && item->data(0, sexp_tree_view::FormulaDataRole).toInt() == formula) {
			item->setText(0, QString::fromUtf8(cutscenes[cutsceneIndex].filename));
			break;
		}
	}
}

void MissionCutscenesDialog::updateUi()
{
	// Avoid infinite recursion by blocking signal calls caused by our changes here
	util::SignalBlockers blocker(this);

	if (!_model->isCurrentCutsceneValid()) {
		ui->cutsceneFilename->setText(QString());
		ui->cutsceneTypeCombo->setCurrentIndex(-1);

		ui->cutsceneTypeCombo->setEnabled(false);
		ui->cutsceneFilename->setEnabled(false);

		return;
	}

	auto& cutscene = _model->getCurrentCutscene();

	ui->cutsceneFilename->setText(QString::fromUtf8(cutscene.filename));
	ui->cutsceneTypeCombo->setCurrentIndex(cutscene.type);

	ui->cutsceneTypeCombo->setEnabled(true);
	ui->cutsceneFilename->setEnabled(true);

	setCutsceneTypeDescription();
}
void MissionCutscenesDialog::load_tree()
{
	ui->cutsceneEventTree->clear_tree();
	auto& cutscenes = _model->getCutscenes();
	for (auto& scene : cutscenes) {
		scene.formula = ui->cutsceneEventTree->_model.load_sub_tree(scene.formula, true, "true");
	}
	ui->cutsceneEventTree->_model.post_load();
}
void MissionCutscenesDialog::recreate_tree()
{
	ui->cutsceneEventTree->clear();
	const auto& cutscenes = _model->getCutscenes();
	for (const auto& scene : cutscenes) {
		if (!_model->isCutsceneVisible(scene)) {
			continue;
		}

		auto h = ui->cutsceneEventTree->insert(scene.filename);
		h->setData(0, sexp_tree_view::FormulaDataRole, scene.formula);
		ui->cutsceneEventTree->add_sub_tree(scene.formula, h);
	}

	_model->setCurrentCutscene(-1);
}
void MissionCutscenesDialog::createNewCutscene()
{
	auto& scene = _model->createNewCutscene();

	auto h = ui->cutsceneEventTree->insert(scene.filename);

	ui->cutsceneEventTree->setCurrentItemIndex(-1);
	ui->cutsceneEventTree->add_operator("true", h);
	auto index = scene.formula = ui->cutsceneEventTree->getCurrentItemIndex();
	h->setData(0, sexp_tree_view::FormulaDataRole, index);

	ui->cutsceneEventTree->setCurrentItem(h);
}
void MissionCutscenesDialog::changeCutsceneCategory(int type)
{
	if (_model->isCurrentCutsceneValid()) {
		const QByteArray before = _model->captureWorkingState();
		_model->setCurrentCutsceneType(type);
		recreate_tree();
		pushWorkingStateSnapshot(before, tr("Change Cutscene Type"));
	}
}

void MissionCutscenesDialog::populateCutsceneCombos()
{
	ui->displayTypeCombo->clear();
	ui->cutsceneTypeCombo->clear();

	for (auto& item : CutsceneMenuData) {
		ui->displayTypeCombo->addItem(QString::fromStdString(item.label), item.value);
		ui->cutsceneTypeCombo->addItem(QString::fromStdString(item.label), item.value);
	}

	ui->displayTypeCombo->setCurrentIndex(_model->getSelectedCutsceneType());
	setCutsceneTypeDescription();
}

void MissionCutscenesDialog::setCutsceneTypeDescription()
{
	auto index = _model->getCutsceneType();
	if (index < 0 || index >= Num_movie_types) {
		ui->cutsceneTypeDescription->setText(QString());
		return;
	}

	ui->cutsceneTypeDescription->setText(QString::fromStdString(CutsceneMenuData[index].desc));
}

void MissionCutscenesDialog::on_okAndCancelButtons_accepted()
{
	accept();
}

void MissionCutscenesDialog::on_okAndCancelButtons_rejected()
{
	reject();
}

void MissionCutscenesDialog::on_displayTypeCombo_currentIndexChanged(int index)
{
	const int before = _model->getSelectedCutsceneType();
	if (before == index)
		return;

	_model->setCutsceneType(index);
	setCutsceneTypeDescription();
	recreate_tree();

	auto* cmd = new FieldEditCommand<int>(FieldId::Cutscene_DisplayFilter, nullptr, tr("Change Cutscene Display Type"), true);
	cmd->addEntry(before, index, [this](const int& v) {
		_model->setCutsceneType(v);
		{
			QSignalBlocker blocker(ui->displayTypeCombo);
			ui->displayTypeCombo->setCurrentIndex(v);
		}
		setCutsceneTypeDescription();
		recreate_tree();
	});
	_dialogStack->push(cmd);
}

void MissionCutscenesDialog::on_cutsceneTypeCombo_currentIndexChanged(int index)
{
	changeCutsceneCategory(index);
}

void MissionCutscenesDialog::on_cutsceneFilename_textChanged(const QString& text)
{
	if (!_model->isCurrentCutsceneValid())
		return;

	const int index = _model->getCurrentCutsceneIndex();
	const SCP_string before = _model->getCurrentCutscene().filename;
	const SCP_string after  = text.toUtf8().constData();
	if (before == after)
		return;

	_model->setCurrentCutsceneFilename(after.c_str());
	syncCutsceneRootLabel(index);

	auto* cmd = new FieldEditCommand<SCP_string>(
	    FieldId::Cutscene_Filename + index * FieldId::Cutscene_FieldStride,
	    nullptr, tr("Change Cutscene Filename"), true);
	cmd->addEntry(before, after, [this, index](const SCP_string& v) {
		_model->setCutsceneFilenameAt(index, v);
		syncCutsceneRootLabel(index);
	});
	_dialogStack->push(cmd);
}

void MissionCutscenesDialog::on_newCutsceneBtn_clicked()
{
	const QByteArray before = _model->captureWorkingState();
	_suppressTreeUndo = true;
	createNewCutscene();
	_suppressTreeUndo = false;
	pushWorkingStateSnapshot(before, tr("Add Cutscene"));
}

void MissionCutscenesDialog::on_cutsceneEventTree_selectedRootChanged(int formula)
{
	auto& cutscenes = _model->getCutscenes();
	for (size_t i = 0; i < cutscenes.size(); ++i) {
		if (cutscenes[i].formula == formula) {
			_model->setCurrentCutscene(static_cast<int>(i));
			break;
		}
	}
}

void MissionCutscenesDialog::on_cutsceneEventTree_rootNodeDeleted(int node)
{
	// The widget has not freed the branch yet (it emits before free_node2),
	// so a fresh capture still serializes the deleted cutscene's formula. The
	// modified() the widget emits afterwards is absorbed by the snapshot
	// equality check.
	const QByteArray before = _model->captureWorkingState();
	_model->deleteCutscene(node);
	pushWorkingStateSnapshot(before, tr("Delete Cutscene"));
}

void MissionCutscenesDialog::on_cutsceneEventTree_rootNodeFormulaChanged(int old, int node)
{
	_model->changeFormula(old, node);
}

void MissionCutscenesDialog::on_cutsceneEventTree_helpChanged(const QString& help)
{
	ui->helpTextBox->setPlainText(help);
}

} // namespace fso::fred::dialogs
