#include "ReinforcementsEditorDialog.h"
#include "ui_ReinforcementsDialog.h"
#include "mission/util.h"
#include <globalincs/linklist.h>
#include <ui/util/SignalBlockers.h>
#include <QCloseEvent>
#include <qlineedit.h>

namespace fso::fred::dialogs {
	

ReinforcementsDialog::ReinforcementsDialog(FredView* parent, EditorViewport* viewport)
	: QDialog(parent), ui(new Ui::ReinforcementsDialog()), _model(new ReinforcementsDialogModel(this, viewport)),
	_viewport(viewport)
{
	this->setFocus();
	ui->setupUi(this);		

	updateUi();
}

ReinforcementsDialog::~ReinforcementsDialog() = default;

void ReinforcementsDialog::accept()
{
	// If apply() returns true, close the dialog
	if (_model->apply()) {
		QDialog::accept();
	}
	// else: validation failed, don’t close
}

void ReinforcementsDialog::reject()
{
	// Asks the user if they want to save changes, if any
	// If they do, it runs _model->apply() and returns the success value
	// If they don't, it runs _model->reject() and returns true
	if (rejectOrCloseHandler(this, _model.get(), _viewport)) {
		QDialog::reject(); // actually close
	}
	// else: do nothing, don't close
}

void ReinforcementsDialog::closeEvent(QCloseEvent* e)
{
	reject();
	e->ignore(); // Don't let the base class close the window
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
	for (auto* it : ui->chosenShipsList->selectedItems()) {
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

	_model->removeFromReinforcements(selectedItemsOut);

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

	_model->addToReinforcements(selectedItemsOut);

	updateUi();
}

void ReinforcementsDialog::on_moveSelectionUp_clicked() 
{
	_model->moveReinforcementsUp();
	updateUi();
}

void ReinforcementsDialog::on_moveSelectionDown_clicked() 
{
	_model->moveReinforcementsDown();
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
		_model->setUseCount(val);
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
		_model->setBeforeArrivalDelay(val);
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