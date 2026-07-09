#include "ReinforcementsEditorDialog.h"
#include "ui_ReinforcementsDialog.h"
#include "mission/commands/FredCommands.h"
#include "mission/util.h"
#include "ui/Theme.h"
#include <globalincs/linklist.h>
#include <ui/util/DialogUndo.h>
#include <ui/util/SignalBlockers.h>
#include <QCloseEvent>
#include <QItemSelectionModel>
#include <qlineedit.h>

namespace fso::fred::dialogs {
	

ReinforcementsDialog::ReinforcementsDialog(FredView* parent, EditorViewport* viewport)
	: QDialog(parent), ui(new Ui::ReinforcementsDialog()), _model(new ReinforcementsDialogModel(this, viewport)),
	  _viewport(viewport), _fredView(parent)
{
	this->setFocus();
	ui->setupUi(this);

	_dialogStack = new QUndoStack(this);
	_fredView->undoGroup()->addStack(_dialogStack);
	util::setupDialogUndo(this, _fredView->undoGroup(), _dialogStack, tr("Reinforcements"));

	// Any chosen-list selection change starts a new merge epoch for the
	// spinbox snapshots.
	connect(ui->chosenShipsList->selectionModel(), &QItemSelectionModel::selectionChanged, this,
		[this]() { ++_selectionGeneration; });

	fso::fred::bindStandardIcon(ui->moveSelectionUp, QStyle::SP_ArrowUp);
	ui->moveSelectionUp->setText(QString());
	ui->moveSelectionUp->setToolTip(tr("Move selected reinforcement up"));

	fso::fred::bindStandardIcon(ui->moveSelectionDown, QStyle::SP_ArrowDown);
	ui->moveSelectionDown->setText(QString());
	ui->moveSelectionDown->setToolTip(tr("Move selected reinforcement down"));

	updateUi();
}

ReinforcementsDialog::~ReinforcementsDialog() = default;

void ReinforcementsDialog::accept()
{
	QByteArray stateBefore = _model->captureState();
	if (_model->apply()) {
		QByteArray stateAfter = _model->captureState();
		_model->setParent(nullptr);
		_fredView->mainUndoStack()->push(
			new ApplyDialogCommand(std::move(_model), stateBefore, stateAfter,
			                       _viewport->editor, tr("Edit Reinforcements")));
		_dialogStack->clear();
		_fredView->undoGroup()->setActiveStack(_fredView->mainUndoStack());
		QDialog::accept();
	}
}

void ReinforcementsDialog::reject()
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

void ReinforcementsDialog::closeEvent(QCloseEvent* e)
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

void ReinforcementsDialog::pushWorkingStateSnapshot(const QByteArray& before, const QString& label, int mergeId)
{
	const QByteArray after = _model->captureWorkingState();
	if (after == before)
		return;

	_dialogStack->push(new DialogSnapshotCommand(before, after,
		[this](const QByteArray& blob) {
			_model->restoreWorkingState(blob);
			updateUi();
		},
		label, true, mergeId));
}


static inline void setSpinMixed(QSpinBox* sb)
{
	sb->setSpecialValueText(QStringLiteral("-"));    // special text for mixed values
	sb->setMinimum(std::numeric_limits<int>::min()); // sentinel below real min
	sb->setValue(sb->minimum());                     // triggers special text display
}

static inline void setSpinNormal(QSpinBox* sb, int min, int max, int value)
{
	sb->setSpecialValueText(QString()); // disable special text
	sb->setRange(min, max);
	sb->setValue(value);
}

void ReinforcementsDialog::updateUi()
{
	util::SignalBlockers blockers(this);
	
	enableDisableControls();

	// Save current selections
	QSet<QString> chosen;
	for (auto* it : ui->chosenShipsList->selectedItems()) {
		chosen.insert(it->text());
	}

	QSet<QString> possible;
	for (auto* it : ui->possibleShipsList->selectedItems()) {
		possible.insert(it->text());
	}

	ui->chosenShipsList->clear();
	ui->possibleShipsList->clear();

	auto newShipPoolList = _model->getShipPoolList();
	auto newReinforcementList = _model->getReinforcementList();

	for (auto& candidate : newShipPoolList) {
		ui->possibleShipsList->addItem(QString(candidate.c_str()));
	}

	// Restore previous selections
	for (const auto& name : possible) {
		const auto matches = ui->possibleShipsList->findItems(name, Qt::MatchExactly);
		for (auto* it : matches) {
			it->setSelected(true);
		}
	}

	for (auto& reinforcement : newReinforcementList) {
		ui->chosenShipsList->addItem(QString(reinforcement.c_str()));
	}

	// Restore previous selections
	for (const auto& name : chosen) {
		const auto matches = ui->chosenShipsList->findItems(name, Qt::MatchExactly);
		for (auto* it : matches) {
			it->setSelected(true);
		}
	}

	int use = _model->getUseCount();

	if (use < 0) {
		setSpinMixed(ui->useSpinBox); 
	} else {
		setSpinNormal(ui->useSpinBox, 0, 16777215, use);
	}
	
	int delay = _model->getBeforeArrivalDelay();

	if (delay < 0) {
		setSpinMixed(ui->delaySpinBox); 
	} else {
		setSpinNormal(ui->delaySpinBox, 0, 16777215, delay);
	}
}

void ReinforcementsDialog::enableDisableControls()
{
	int count = ui->chosenShipsList->selectedItems().count();

	const auto selected = ui->chosenShipsList->selectedItems();

	const bool anySupportsUse = std::any_of(selected.cbegin(), selected.cend(), [&](const QListWidgetItem* it) {
		return _model->getUseCountEnabled(it->text().toUtf8().constData());
	});
	
	ui->useSpinBox->setEnabled(anySupportsUse && count > 0 && _model->getUseCount() != -2);
	ui->delaySpinBox->setEnabled(count > 0 && _model->getUseCount() != -2);
}

void ReinforcementsDialog::on_actionRemoveShip_clicked()
{
	SCP_vector<SCP_string> selectedItems;

	for (int i = 0; i < ui->chosenShipsList->count(); i++) {
		auto current = ui->chosenShipsList->item(i);
		if (current->isSelected()) {
			selectedItems.emplace_back(current->text().toUtf8().constData());
		}
	}

	const SCP_vector<SCP_string> selectedItemsOut = selectedItems;

	const QByteArray before = _model->captureWorkingState();
	_model->removeFromReinforcements(selectedItemsOut);
	pushWorkingStateSnapshot(before, tr("Remove Reinforcement"));

	updateUi();
}

void ReinforcementsDialog::on_okAndCancelButtons_accepted()
{
	accept();
}

void ReinforcementsDialog::on_okAndCancelButtons_rejected()
{
	reject();
}

void ReinforcementsDialog::on_actionAddShip_clicked()
{
	SCP_vector<SCP_string> selectedItems;

	for (int i = 0; i < ui->possibleShipsList->count(); i++) {
		auto current = ui->possibleShipsList->item(i);
		if (current->isSelected()) {
			selectedItems.emplace_back(current->text().toUtf8().constData());
		}
	}

	const SCP_vector<SCP_string> selectedItemsOut = selectedItems;

	const QByteArray before = _model->captureWorkingState();
	_model->addToReinforcements(selectedItemsOut);
	pushWorkingStateSnapshot(before, tr("Add Reinforcement"));

	updateUi();
}

void ReinforcementsDialog::on_moveSelectionUp_clicked()
{
	const QByteArray before = _model->captureWorkingState();
	_model->moveReinforcementsUp();
	pushWorkingStateSnapshot(before, tr("Move Reinforcement"));
	updateUi();
}

void ReinforcementsDialog::on_moveSelectionDown_clicked()
{
	const QByteArray before = _model->captureWorkingState();
	_model->moveReinforcementsDown();
	pushWorkingStateSnapshot(before, tr("Move Reinforcement"));
	updateUi();
}

void ReinforcementsDialog::on_useSpinBox_valueChanged(int val)
{
	// need to check that it's not empty so as not to pass nonsense values back to the model.
	if (ui->chosenShipsList->selectedItems().count() > 0) {
		if (val < 0) {
			val = 0;

			util::SignalBlockers blockers(this);
			ui->useSpinBox->setValue(val);
		}
		// The setter applies to the whole model-tracked selection, so this is
		// a snapshot; scrubs within one selection merge into one entry.
		const QByteArray before = _model->captureWorkingState();
		_model->setUseCount(val);
		pushWorkingStateSnapshot(before, tr("Change Use Count"),
			FieldId::Reinf_SnapSpinBase + 0 * 1000000 + (_selectionGeneration % 1000000));
	}
}

void ReinforcementsDialog::on_delaySpinBox_valueChanged(int val)
{
	// need to check that it's not empty so as not to pass nonsense values back to the model.
	if (ui->chosenShipsList->selectedItems().count() > 0) {
		if (val < 0) {
			val = 0;

			util::SignalBlockers blockers(this);
			ui->delaySpinBox->setValue(val);
		}
		const QByteArray before = _model->captureWorkingState();
		_model->setBeforeArrivalDelay(val);
		pushWorkingStateSnapshot(before, tr("Change Arrival Delay"),
			FieldId::Reinf_SnapSpinBase + 1 * 1000000 + (_selectionGeneration % 1000000));
	}
}

void ReinforcementsDialog::on_chosenShipsList_itemClicked(QListWidgetItem* /*item*/)
{
	SCP_vector<SCP_string> listOut;
	for (auto& currentItem : ui->chosenShipsList->selectedItems()) {
		listOut.emplace_back(currentItem->text().toUtf8().constData());
	}

	const SCP_vector<SCP_string> listOutFinal = listOut;

	_model->selectReinforcement(listOutFinal);

	updateUi();
}

void ReinforcementsDialog::on_chosenMultiselectCheckbox_toggled(bool checked)
{
	if (checked) {
		ui->chosenShipsList->setSelectionMode(QAbstractItemView::MultiSelection);
	} else {
		ui->chosenShipsList->setSelectionMode(QAbstractItemView::SingleSelection);
		ui->chosenShipsList->clearSelection();
		updateUi();
	}
}

void ReinforcementsDialog::on_poolMultiselectCheckbox_toggled(bool checked)
{
	if (checked) {
		ui->possibleShipsList->setSelectionMode(QAbstractItemView::MultiSelection);
	} else {
		ui->possibleShipsList->setSelectionMode(QAbstractItemView::SingleSelection);
		ui->possibleShipsList->clearSelection();
		updateUi();
	}
}

} // namespace fso::fred::dialogs
