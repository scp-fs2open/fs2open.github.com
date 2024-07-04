#include "ReinforcementsEditorDialog.h"
#include "ui_ReinforcementsDialog.h"

#include <globalincs/linklist.h>
#include <ui/util/SignalBlockers.h>
#include <QCloseEvent>
#include <QListWidget>
#include <qlineedit.h>

namespace fso {
namespace fred {
namespace dialogs {

	

	ReinforcementsDialog::ReinforcementsDialog(FredView* parent, EditorViewport* viewport)
		: QDialog(parent), ui(new Ui::ReinforcementsDialog()), _model(new ReinforcementsDialogModel(this, viewport)),
		_viewport(viewport)
	{
		this->setFocus();
		ui->setupUi(this);

		connect(_model.get(), &AbstractDialogModel::modelChanged, this, &ReinforcementsDialog::updateUI);
		connect(this, &QDialog::accepted, _model.get(), &ReinforcementsDialogModel::apply);
		connect(this, &QDialog::rejected, _model.get(), &ReinforcementsDialogModel::reject);			


		connect(ui->delayLineEdit,
			static_cast<void (QLineEdit::*)(const QString&)>(&QLineEdit::textChanged),
			this,
			&ReinforcementsDialog::onDelayChanged);

		connect(ui->useLineEdit,
			static_cast<void (QLineEdit::*)(const QString&)>(&QLineEdit::textChanged),
			this,
			&ReinforcementsDialog::onUseCountChanged);

		// force to a number value. Spinbox would be better here, except there are times where we'll want it to be empty, not zero or some other value.
		// cannot be done in the designer so do it while setting up other things involving the LineEdit.
		ui->delayLineEdit->setValidator(new QIntValidator(0, INT_MAX, this));
		ui->useLineEdit->setValidator(new QIntValidator(0, INT_MAX, this));

		connect(ui->chosenShipsList,
			static_cast<void (QListWidget::*)(QListWidgetItem*)>(&QListWidget::itemClicked),
			this,
			&ReinforcementsDialog::onReinforcementItemChanged);

		connect(ui->chosenShipsList,
			static_cast<void (QListWidget::*)(QDropEvent*)>(&QListWidget::dropEvent),
			this,
			&ReinforcementsDialog::onReinforcementItemChanged);

		connect(ui->actionAddShip,
			&QPushButton::clicked,
			this,
			&ReinforcementsDialog::on_actionAddShip_clicked);

		 connect(ui->actionRemoveShip,
			 &QPushButton::clicked,
			 this,
			 &ReinforcementsDialog::on_actionRemoveShip_clicked);

		updateUI();
	}

	void ReinforcementsDialog::updateUI(){
	
		if (_model->listUpdateRequired()) {
			ui->chosenShipsList->clear();
			ui->possibleShipsList->clear();
			
			auto newShipPoolList = _model->getShipPoolList();
			auto newReinforcementList = _model->getReinforcementList();

			for (auto& candidate : newShipPoolList) {
				ui->possibleShipsList->addItem(QString(candidate.c_str()));
			}

			for (auto& reinforcement : newReinforcementList) {
				ui->chosenShipsList->addItem(QString(reinforcement.c_str()));
			}
		}
	
		if (_model->numberLineEditUpdateRequired()) {
			int delay = _model->getBeforeArrivalDelay();
			
			if (delay == -1) {
				ui->delayLineEdit->clear();
				ui->delayLineEdit->setDisabled(false);
			}
			else if (delay == -2) {
				ui->delayLineEdit->clear();
				ui->delayLineEdit->setDisabled(true);
			} else {
				ui->delayLineEdit->setDisabled(false);
				ui->delayLineEdit->setText(QString::number(delay));
			}

			int use = _model->getUseCount();

			if (use == -1) {
				ui->useLineEdit->setDisabled(false);
				ui->useLineEdit->clear();
			}
			else if (use == -2) {
				ui->useLineEdit->clear();
				ui->useLineEdit->setDisabled(true);
			} else {
				ui->useLineEdit->setDisabled(false);
				ui->useLineEdit->setText(QString::number(use));
			}
		}
	}


	void ReinforcementsDialog::onReinforcementItemChanged()
	{
		SCP_vector<SCP_string> listOut;
		for (auto& currentItem : ui->chosenShipsList->selectedItems()){
			listOut.push_back(currentItem->text().toStdString());
		}
	
		if (ui->chosenShipsList->selectedItems().count() > 0) {
			ui->delayLineEdit->setDisabled(false);
			ui->useLineEdit->setDisabled(false);
		}

		const SCP_vector<SCP_string> listOutFinal = listOut;

		_model->selectReinforcement(listOutFinal);
	}

	void ReinforcementsDialog::onDelayChanged() 
	{
		// need to check that it's not empty so as not to pass nonsense values back to the model.
		if (ui->chosenShipsList->selectedItems().count() >= 0 && !ui->delayLineEdit->text().isEmpty()) {
			_model->setBeforeArrivalDelay(ui->delayLineEdit->text().toInt());
		}
	}

	void ReinforcementsDialog::onUseCountChanged()
	{
		// need to check that it's not empty so as not to pass nonsense values back to the model.
		if (ui->chosenShipsList->selectedItems().count() > 0 && !ui->useLineEdit->text().isEmpty()) {
			_model->setUseCount(ui->useLineEdit->text().toInt());
		}
	}


	void ReinforcementsDialog::on_actionRemoveShip_clicked()
	{
		SCP_vector<SCP_string> selectedItems;

		for (int i = 0; i < ui->chosenShipsList->count(); i++) {
			auto current = ui->chosenShipsList->item(i);
			if (current->isSelected()) {
				selectedItems.push_back(current->text().toStdString());
			}
		}

		const SCP_vector<SCP_string> selectedItemsOut = selectedItems;

		_model->removeFromReinforcements(selectedItemsOut);

	}

	void ReinforcementsDialog::on_actionAddShip_clicked()
	{
		SCP_vector<SCP_string> selectedItems;

		for (int i = 0; i < ui->possibleShipsList->count(); i++) {
			auto current = ui->possibleShipsList->item(i);
			if (current->isSelected()) {
				selectedItems.push_back(current->text().toStdString());
			}
		}

		const SCP_vector<SCP_string> selectedItemsOut = selectedItems;

		_model->addToReinforcements(selectedItemsOut);

	}

	void ReinforcementsDialog::on_moveSelectionUp_clicked() 
	{
		_model->moveReinforcementsUp();
	}

	void ReinforcementsDialog::on_moveSelectionDown_clicked() 
	{
		_model->moveReinforcementsDown();
	}

	void ReinforcementsDialog::enableDisableControls(){}

	void ReinforcementsDialog::on_chosenShipsList_clicked(){}

	ReinforcementsDialog::~ReinforcementsDialog() {} // NOLINT

	void ReinforcementsDialog::closeEvent(QCloseEvent* ){}

}
}
}