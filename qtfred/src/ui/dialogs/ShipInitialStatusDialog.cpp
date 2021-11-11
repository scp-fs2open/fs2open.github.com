#include "ShipInitialStatusDialog.h"

#include "globalincs/alphacolors.h"

#include "ui_ShipInitialStatus.h"

#include "object/objectdock.h"

#include <globalincs/linklist.h>
#include <mission/object.h>
#include <ui/util/SignalBlockers.h>

#include <QCloseEvent>

namespace fso {
	namespace fred {
		namespace dialogs {

			ShipInitialStatusDialog::ShipInitialStatusDialog(QWidget* parent, EditorViewport* viewport, bool multi)
				: QDialog(parent), ui(new Ui::ShipInitialStatusDialog()),
				_model(new ShipInitialStatusDialogModel(this, viewport, multi)), _viewport(viewport)
			{
				ui->setupUi(this);

				connect(this, &QDialog::accepted, _model.get(), &ShipInitialStatusDialogModel::apply);
				connect(this, &QDialog::rejected, _model.get(), &ShipInitialStatusDialogModel::reject);
				connect(_model.get(), &AbstractDialogModel::modelChanged, this, &ShipInitialStatusDialog::updateUI);

				// Velocity
				connect(ui->velocitySpinBox,
					static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
					this,
					&ShipInitialStatusDialog::velocityChanged);
				// Dockpoint List
				connect(ui->dockpointList, &QListWidget::currentItemChanged, this, &ShipInitialStatusDialog::dockChanged);
				connect(ui->dockeeComboBox,
					static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
					this,
					&ShipInitialStatusDialog::dockeeComboChanged);
				connect(ui->dockeePointComboBox,
					static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
					this,
					&ShipInitialStatusDialog::dockeePointChanged);
				connect(ui->hullSpinBox,
					static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
					this,
					&ShipInitialStatusDialog::hullChanged);
				connect(ui->hasShieldCheckBox, &QCheckBox::stateChanged, this, &ShipInitialStatusDialog::hasShieldChanged);
				connect(ui->shieldHullSpinBox,
					static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
					this,
					&ShipInitialStatusDialog::shieldHullChanged);
				connect(ui->forceShieldsCheckBox, &QCheckBox::stateChanged, this, &ShipInitialStatusDialog::forceShieldChanged);
				connect(ui->shipLockCheckBox, &QCheckBox::stateChanged, this, &ShipInitialStatusDialog::shipLockChanged);
				connect(ui->weaponLockCheckBox, &QCheckBox::stateChanged, this, &ShipInitialStatusDialog::weaponLockChanged);
				connect(ui->primaryLockCheckBox, &QCheckBox::stateChanged, this, &ShipInitialStatusDialog::primaryLockChanged);
				connect(ui->secondaryLockCheckBox, &QCheckBox::stateChanged, this, &ShipInitialStatusDialog::secondaryLockChanged);
				connect(ui->turretLockCheckBox, &QCheckBox::stateChanged, this, &ShipInitialStatusDialog::turretLockChanged);
				connect(ui->afterburnerLockCheckBox,
					&QCheckBox::stateChanged,
					this,
					&ShipInitialStatusDialog::afterburnerLockChanged);

				connect(ui->subsystemList, &QListWidget::currentRowChanged, this, &ShipInitialStatusDialog::subsystemChanged);

				connect(ui->subIntegritySpinBox,
					static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
					this,
					&ShipInitialStatusDialog::subIntegrityChanged);
				connect(ui->cargoEdit, &QLineEdit::editingFinished, this, &ShipInitialStatusDialog::cargoChanged);
				connect(ui->colourComboBox,
					static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
					this,
					&ShipInitialStatusDialog::colourChanged);

				updateUI();

				// Resize the dialog to the minimum size
				resize(QDialog::sizeHint());
			}

			ShipInitialStatusDialog::~ShipInitialStatusDialog() = default;

			void ShipInitialStatusDialog::closeEvent(QCloseEvent* event)
			{
				if (_model->query_modified()) {
					auto button = _viewport->dialogProvider->showButtonDialog(DialogType::Question,
						"Changes detected",
						"Do you want to keep your changes?",
						{ DialogButton::Yes, DialogButton::No, DialogButton::Cancel });

					if (button == DialogButton::Cancel) {
						event->ignore();
						return;
					}

					if (button == DialogButton::Yes) {
						accept();
						return;
					}
				}

				QDialog::closeEvent(event);
			}
			void ShipInitialStatusDialog::updateUI()
			{
				util::SignalBlockers blockers(this);
				auto value = _model->getVelocity();
				if (value != BLANKFIELD) {
					ui->velocitySpinBox->setValue(value);
				}
				else {
					ui->velocitySpinBox->setSpecialValueText("-");
				}
				value = _model->getHull();
				if (value != BLANKFIELD) {
					ui->hullSpinBox->setValue(value);
				}
				else {
					ui->hullSpinBox->setSpecialValueText("-");
				}
				value = _model->getHasShield();
				ui->hasShieldCheckBox->setCheckState(Qt::CheckState(value));
				if (_model->getHasShield()) {
					ui->shieldHullSpinBox->setEnabled(true);
				}
				else {
					ui->shieldHullSpinBox->setEnabled(false);
				}
				value = _model->getShieldHull();
				if (value != BLANKFIELD) {
					ui->shieldHullSpinBox->setValue(value);
				}
				else {
					ui->shieldHullSpinBox->setSpecialValueText("-");
				}
				updateFlags();
				updateDocks();
				updateDockee();
				updateSubsystems();
				ui->colourComboBox->clear();
				if (_model->m_use_teams) {
					int i = 0;
					ui->colourComboBox->setEnabled(true);
					ui->colourComboBox->addItem("none", i);
					++i;
					for (auto& Team_Name : Team_Names) {
						ui->colourComboBox->addItem(Team_Name.c_str(), i);
						i++;
					}
					auto currentText = _model->getColour();
					ui->colourComboBox->setCurrentIndex(ui->colourComboBox->findText(currentText.c_str()));
				}
			}
			void ShipInitialStatusDialog::updateFlags()
			{
				auto value = _model->getForceShield();
				ui->forceShieldsCheckBox->setCheckState(Qt::CheckState(value));
				value = _model->getShipLocked();
				ui->shipLockCheckBox->setCheckState(Qt::CheckState(value));
				value = _model->getWeaponLocked();
				ui->weaponLockCheckBox->setCheckState(Qt::CheckState(value));
				value = _model->getPrimariesDisabled();
				ui->primaryLockCheckBox->setCheckState(Qt::CheckState(value));
				value = _model->getSecondariesDisabled();
				ui->secondaryLockCheckBox->setCheckState(Qt::CheckState(value));
				value = _model->getTurretsDisabled();
				ui->turretLockCheckBox->setCheckState(Qt::CheckState(value));
				value = _model->getAfterburnerDisabled();
				ui->afterburnerLockCheckBox->setCheckState(Qt::CheckState(value));
			}
			void ShipInitialStatusDialog::updateDocks()
			{
				auto index = ui->dockpointList->currentIndex();
				ui->dockpointList->clear();
				if (!_model->m_multi_edit) {
					for (int dockpoint = 0; dockpoint < _model->getnum_dock_points(); dockpoint++) {
						auto newItem = new QListWidgetItem;
						newItem->setText(
							model_get_dock_name(Ship_info[Ships[_model->getShip()].ship_info_index].model_num, dockpoint));
						newItem->setData(Qt::UserRole, dockpoint);
						ui->dockpointList->addItem(newItem);
					}
				}
				ui->dockpointList->setCurrentIndex(index);
				if (cur_docker_point < 0) {
					// clear the dropdowns
					list_dockees(-1);
					list_dockee_points(-1);
				}
				else {
					// populate with all possible dockees
					list_dockees(
						model_get_dock_index_type(Ship_info[Ships[_model->getShip()].ship_info_index].model_num, cur_docker_point));

					// see if there's a dockee here
					if (_model->getdockpoint_array()[cur_docker_point].dockee_shipnum >= 0) {
						// select the dockee
						ui->dockeeComboBox->setCurrentIndex(ui->dockeeComboBox->findText(
							Ships[_model->getdockpoint_array()[cur_docker_point].dockee_shipnum].ship_name));
					}
					else {
						ui->dockeeComboBox->setCurrentIndex(0);
					}
				}
			}
			void ShipInitialStatusDialog::updateDockee()
			{
				if (cur_dockee < 0) {
					// clear the dropdown
					list_dockee_points(-1);
				}
				else {
					// populate with dockee points
					list_dockee_points(cur_dockee);
					if (_model->getdockpoint_array()[cur_docker_point].dockee_point < 0) {
						// select the dockpoint
						dockeePointChanged(0);
					}

					// see if there's a dockpoint here
					if (_model->getdockpoint_array()[cur_docker_point].dockee_point >= 0) {
						// select the dockpoint
						ui->dockeePointComboBox->setCurrentIndex(ui->dockeePointComboBox->findText(
							model_get_dock_name(Ship_info[Ships[cur_dockee].ship_info_index].model_num,
								_model->getdockpoint_array()[cur_docker_point].dockee_point)));
					}
				}
			}
			void ShipInitialStatusDialog::list_dockees(int dock_types)
			{ // enable/disable dropdown
				ui->dockeeComboBox->setEnabled((dock_types >= 0));

				// clear the existing dockees
				ui->dockeeComboBox->clear();
				// that might be all we need to do
				if (dock_types < 0)
					return;
				// populate with potential dockees

				// add "nothing"
				ui->dockeeComboBox->addItem("Nothing", QVariant(int(-1)));

				// add ships
				for (object* objp = GET_FIRST(&obj_used_list); objp != END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp)) {
					if ((objp->type == OBJ_SHIP) || (objp->type == OBJ_START)) {
						int ship = get_ship_from_obj(objp);

						// mustn't be the same ship
						if (ship == _model->getShip()) {
							continue;
						}

						// mustn't also be docked elsewhere
						bool docked_elsewhere = false;
						for (int i = 0; i < _model->getnum_dock_points(); i++) {
							// don't erroneously check the same point
							if (i == cur_docker_point) {
								continue;
							}

							// see if this ship is also on a different dockpoint
							if (_model->getdockpoint_array()[i].dockee_shipnum == ship) {
								docked_elsewhere = true;
								break;
							}
						}

						if (docked_elsewhere) {
							continue;
						}

						// docking must be valid
						if (!ship_docking_valid(_model->getShip(), ship) && !ship_docking_valid(ship, _model->getShip())) {
							continue;
						}

						// dock types must match
						if (!(model_get_dock_types(Ship_info[Ships[ship].ship_info_index].model_num) & dock_types)) {
							continue;
						}

						// add to list
						ui->dockeeComboBox->addItem(Ships[ship].ship_name, QVariant(int(ship)));
					}
				}
			}
			void ShipInitialStatusDialog::list_dockee_points(int shipnum)
			{
				ship* shipp = &Ships[_model->getShip()];
				ship* other_shipp = &Ships[shipnum];

				// enable/disable dropdown
				ui->dockeePointComboBox->setEnabled((shipnum >= 0));

				// clear the existing dockee points
				ui->dockeePointComboBox->clear();

				// that might be all we need to do
				if (shipnum < 0) {
					return;
				}

				// get the required dock type(s)
				int dock_type = model_get_dock_index_type(Ship_info[shipp->ship_info_index].model_num, cur_docker_point);

				// populate with the right kind of dockee points
				for (int i = 0; i < model_get_num_dock_points(Ship_info[other_shipp->ship_info_index].model_num); i++) {
					// make sure this dockpoint is not occupied by someone else
					object* docked_objp = dock_find_object_at_dockpoint(&Objects[other_shipp->objnum], i);
					if ((docked_objp != nullptr) && (docked_objp != &Objects[shipp->objnum]))
						continue;

					// make sure its type matches
					if (!(model_get_dock_index_type(Ship_info[other_shipp->ship_info_index].model_num, i) & dock_type))
						continue;

					// add to list
					ui->dockeePointComboBox->addItem(model_get_dock_name(Ship_info[other_shipp->ship_info_index].model_num, i),
						QVariant(int(i)));
				}
			}
			void ShipInitialStatusDialog::updateSubsystems()
			{
				ship_subsys* ptr;
				auto index = ui->subsystemList->currentIndex();
				ui->subsystemList->clear();
				if (!_model->m_multi_edit) {
					ui->subsystemList->setEnabled(true);
					ui->subIntegritySpinBox->setEnabled(true);
					if (_model->getShip_has_scannable_subsystems()) {
						ui->cargoEdit->setEnabled(true);
					}
					for (ptr = GET_FIRST(&Ships[_model->getShip()].subsys_list);
						ptr != END_OF_LIST(&Ships[_model->getShip()].subsys_list);
						ptr = GET_NEXT(ptr)) {
						auto newItem = new QListWidgetItem;
						newItem->setText(ptr->system_info->subobj_name);
						ui->subsystemList->addItem(newItem);
					}
					ui->subsystemList->setCurrentIndex(index);
				}
				else {
					ui->subsystemList->setEnabled(false);
					ui->subIntegritySpinBox->setEnabled(false);
					ui->cargoEdit->setEnabled(false);
				}
				auto value = _model->getDamage();
				ui->subIntegritySpinBox->setValue(value);
				auto cargovalue = _model->getCargo();
				ui->cargoEdit->setText(cargovalue.c_str());
			}
			void ShipInitialStatusDialog::velocityChanged(int value)
			{
				_model->setVelocity(value);
			}

			void ShipInitialStatusDialog::hullChanged(int value)
			{
				_model->setHull(value);
			}

			void ShipInitialStatusDialog::hasShieldChanged(int state)
			{
				_model->setHasShield(state);
			}

			void ShipInitialStatusDialog::shieldHullChanged(int value)
			{
				_model->setShieldHull(value);
			}

			void ShipInitialStatusDialog::forceShieldChanged(int state)
			{
				_model->setForceShield(state);
			}

			void ShipInitialStatusDialog::shipLockChanged(int state)
			{
				_model->setShipLocked(state);
			}

			void ShipInitialStatusDialog::weaponLockChanged(int state)
			{
				_model->setWeaponLocked(state);
			}

			void ShipInitialStatusDialog::primaryLockChanged(int state)
			{
				_model->setPrimariesDisabled(state);
			}

			void ShipInitialStatusDialog::secondaryLockChanged(int state)
			{
				_model->setSecondariesDisabled(state);
			}

			void ShipInitialStatusDialog::turretLockChanged(int state)
			{
				_model->setTurretsDisabled(state);
			}

			void ShipInitialStatusDialog::afterburnerLockChanged(int state)
			{
				_model->setAfterburnerDisabled(state);
			}

			void ShipInitialStatusDialog::dockChanged(QListWidgetItem* current)
			{
				cur_docker_point = current->data(Qt::UserRole).value<int>();
				updateUI();
			}
			void ShipInitialStatusDialog::dockeeComboChanged(int index)
			{
				auto dockeeData = ui->dockeeComboBox->itemData(index).value<int>();
				cur_dockee = dockeeData;
				_model->setDockee(cur_docker_point, dockeeData);
			}
			void ShipInitialStatusDialog::dockeePointChanged(int index)
			{
				auto dockeeData = ui->dockeePointComboBox->itemData(index).value<int>();
				cur_dockee_point = dockeeData;
				_model->setDockeePoint(cur_docker_point, dockeeData);
			}
			void ShipInitialStatusDialog::subsystemChanged(int index)
			{
				_model->change_subsys(index);
			}
			void ShipInitialStatusDialog::subIntegrityChanged(int value)
			{
				_model->setDamage(value);
			}
			void ShipInitialStatusDialog::cargoChanged()
			{
				_model->setCargo(ui->cargoEdit->text().toStdString());
			}
			void ShipInitialStatusDialog::colourChanged(int index)
			{
				_model->setColour(ui->colourComboBox->itemText(index).toStdString());
			}
		} // namespace dialogs
	} // namespace fred
} // namespace fso