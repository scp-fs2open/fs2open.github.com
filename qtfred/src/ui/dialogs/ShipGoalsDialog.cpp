#include "ShipGoalsDialog.h"

#include "ui_ShipGoalsDialog.h"

#include <globalincs/linklist.h>
#include <mission/object.h>
#include <ui/util/SignalBlockers.h>

#include <QCloseEvent>

namespace fso {
namespace fred {
namespace dialogs {
ShipGoalsDialog::ShipGoalsDialog(QWidget* parent, EditorViewport* viewport)
	: QDialog(parent), ui(new Ui::ShipGoalsDialog()),
	  _viewport(viewport)
{
	ui->setupUi(this);
	behaviors[0] = ui->comBehavior1;
	behaviors[1] = ui->comBehavior2;
	behaviors[2] = ui->comBehavior3;
	behaviors[3] = ui->comBehavior4;
	behaviors[4] = ui->comBehavior5;
	behaviors[5] = ui->comBehavior6;
	behaviors[6] = ui->comBehavior7;
	behaviors[7] = ui->comBehavior8;
	behaviors[8] = ui->comBehavior9;
	behaviors[9] = ui->comBehavior10;

	objects[0] = ui->comObject1;
	objects[1] = ui->comObject2;
	objects[2] = ui->comObject3;
	objects[3] = ui->comObject4;
	objects[4] = ui->comObject5;
	objects[5] = ui->comObject6;
	objects[6] = ui->comObject7;
	objects[7] = ui->comObject8;
	objects[8] = ui->comObject9;
	objects[9] = ui->comObject10;

	subsys[0] = ui->comSub1;
	subsys[1] = ui->comSub2;
	subsys[2] = ui->comSub3;
	subsys[3] = ui->comSub4;
	subsys[4] = ui->comSub5;
	subsys[5] = ui->comSub6;
	subsys[6] = ui->comSub7;
	subsys[7] = ui->comSub8;
	subsys[8] = ui->comSub9;
	subsys[9] = ui->comSub10;

	docks[0] = ui->comDock1;
	docks[1] = ui->comDock2;
	docks[2] = ui->comDock3;
	docks[3] = ui->comDock4;
	docks[4] = ui->comDock5;
	docks[5] = ui->comDock6;
	docks[6] = ui->comDock7;
	docks[7] = ui->comDock8;
	docks[8] = ui->comDock9;
	docks[9] = ui->comDock10;

	priority[0] = ui->prioritySpinBox1;
	priority[1] = ui->prioritySpinBox2;
	priority[2] = ui->prioritySpinBox3;
	priority[3] = ui->prioritySpinBox4;
	priority[4] = ui->prioritySpinBox5;
	priority[5] = ui->prioritySpinBox6;
	priority[6] = ui->prioritySpinBox7;
	priority[7] = ui->prioritySpinBox8;
	priority[8] = ui->prioritySpinBox9;
	priority[9] = ui->prioritySpinBox10;
	connect(this, &QDialog::accepted, _model.get(), &ShipGoalsDialogModel::apply);
	connect(this, &QDialog::rejected, _model.get(), &ShipGoalsDialogModel::reject);
	parentDialog = dynamic_cast<ShipEditorDialog*>(parent);
	if (parentDialog == nullptr) {
		//TODO: Add wing editor Here
		WingMode = true;
	} else {
		_model = std::unique_ptr<ShipGoalsDialogModel>(
			new ShipGoalsDialogModel(this, viewport, parentDialog->getMulti(), Ships[parentDialog->getSingleShip()].objnum, -1));
	}

	connect(_model.get(), &AbstractDialogModel::modelChanged, this, &ShipGoalsDialog::updateUI);
	for (int i = 0; i < ED_MAX_GOALS; i++) {
		connect(behaviors[i], QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index) {
			int datap = behaviors[i]->itemData(index).value<int>();
			_model->setBehavior(i, datap);
		});
		connect(objects[i], QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index) {
			int datap = objects[i]->itemData(index).value<int>();
			_model->setObject(i, datap);
		});
		connect(subsys[i], QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index) {
			auto datap = subsys[i]->itemData(index).value<QString>().toStdString();
			_model->setSubsys(i, datap);
		});
		connect(docks[i], QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index) {
			int datap = docks[i]->itemData(index).value<int>();
			_model->setDock(i, datap);
		});
		connect(priority[i], QOverload<int>::of(&QSpinBox::valueChanged), [=]() {
			int datap = priority[i]->value();
			_model->setPriority(i, datap);
		});
	}

	updateUI();

	// Resize the dialog to the minimum size
	resize(QDialog::sizeHint());
}

ShipGoalsDialog::~ShipGoalsDialog() = default;

void ShipGoalsDialog::showEvent(QShowEvent* e)
{
	if (!WingMode) {
		_model->initializeData(parentDialog->getMulti(), Ships[parentDialog->getSingleShip()].objnum, -1);
	}

	QDialog::showEvent(e);
}

void ShipGoalsDialog::closeEvent(QCloseEvent* event)
{
	if (_model->query_modified()) {
		auto button = _viewport->dialogProvider->showButtonDialog(DialogType::Question,
			"Changes detected",
			"Do you want to keep your changes?",
			{DialogButton::Yes, DialogButton::No, DialogButton::Cancel});

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

void ShipGoalsDialog::updateUI()
{

	util::SignalBlockers blockers(this);
	for (int i = 0; i < ED_MAX_GOALS; i++) {
		behaviors[i]->clear();
		objects[i]->clear();
		subsys[i]->clear();
		docks[i]->clear();
		auto value = _model->getBehavior(i);
		behaviors[i]->addItem("None", QVariant(int(AI_GOAL_NONE)));
		for (int j = 0; j < _model->getGoalsSize(); j++) {
			if (_model->getValid(j)) {
				behaviors[i]->addItem(_model->getGoalTypes()[j].name, QVariant(int(_model->getGoalTypes()[j].def)));
			}
		}
		behaviors[i]->setCurrentIndex(behaviors[i]->findData(value));
		auto mode = value;
		SCP_list<waypoint_list>::iterator ii;
		if (i >= MAX_AI_GOALS)
			behaviors[i]->setEnabled(false);
		if (value < 1) {
			objects[i]->setEnabled(false);
			subsys[i]->setEnabled(false);
			docks[i]->setEnabled(false);
			priority[i]->setEnabled(false);
			SCP_string blank;
			if (!_model->getSubsys(i).empty()) {
				_model->setSubsys(i, blank);
			}
			_model->setDock(i, -1);
		} else if ((mode == AI_GOAL_CHASE_ANY) || (mode == AI_GOAL_UNDOCK) || (mode == AI_GOAL_KEEP_SAFE_DISTANCE) ||
				   (mode == AI_GOAL_PLAY_DEAD) || (mode == AI_GOAL_PLAY_DEAD_PERSISTENT) || (mode == AI_GOAL_WARP)) {
			priority[i]->setEnabled(true);
			objects[i]->setEnabled(false);
			subsys[i]->setEnabled(false);
			docks[i]->setEnabled(false);
			SCP_string blank;
			_model->setSubsys(i, blank);
			_model->setDock(i, -1);
			priority[i]->setValue(_model->getPriority(i));
		} else {
			objects[i]->setEnabled(true);
			// for goals that deal with waypoint paths or individual waypoints
			switch (mode) {
			case AI_GOAL_WAYPOINTS:
			case AI_GOAL_WAYPOINTS_ONCE:
				// case AI_GOAL_WARP:
				int j;
				for (j = 0, ii = Waypoint_lists.begin(); ii != Waypoint_lists.end(); ++j, ++ii) {
					objects[i]->addItem(ii->get_name(), QVariant(int(j | TYPE_PATH)));
					if (_model->getObject(i) == (j | TYPE_PATH)) {
						objects[i]->setCurrentIndex(j);
					}
				}
				priority[i]->setEnabled(true);
				priority[i]->setValue(_model->getPriority(i));

				break;
			case AI_GOAL_STAY_STILL:
				object* ptr;
				ptr = GET_FIRST(&obj_used_list);
				while (ptr != END_OF_LIST(&obj_used_list)) {
					if (ptr->type == OBJ_WAYPOINT) {
						objects[i]->addItem(object_name(OBJ_INDEX(ptr)), QVariant(int(OBJ_INDEX(ptr) | TYPE_WAYPOINT)));
						if ((_model->getObject(i) == (OBJ_INDEX(ptr) | TYPE_WAYPOINT)))
							objects[i]->setCurrentIndex(objects[i]->findData(QVariant(int(OBJ_INDEX(ptr) | TYPE_WAYPOINT))));
					}

					ptr = GET_NEXT(ptr);
				}
				priority[i]->setEnabled(true);
				priority[i]->setValue(_model->getPriority(i));

				break;
			}
			// for goals that deal with ship classes
			switch (mode) {
			case AI_GOAL_CHASE_SHIP_CLASS:
				int j;
				for (j = 0; j < ship_info_size(); j++) {
					objects[i]->addItem(Ship_info[j].name, QVariant(int(j | TYPE_SHIP_CLASS)));
					if (_model->getObject(i) == (j | TYPE_SHIP_CLASS))
						objects[i]->setCurrentIndex(j);
				}
				priority[i]->setEnabled(true);
				priority[i]->setValue(_model->getPriority(i));

				break;
			} // for goals that deal with individual ships
			switch (mode) {
			case AI_GOAL_DESTROY_SUBSYSTEM:
			case AI_GOAL_CHASE | AI_GOAL_CHASE_WING:
			case AI_GOAL_DOCK:
			case AI_GOAL_GUARD | AI_GOAL_GUARD_WING:
			case AI_GOAL_DISABLE_SHIP:
			case AI_GOAL_DISARM_SHIP:
			case AI_GOAL_EVADE_SHIP:
			case AI_GOAL_IGNORE:
			case AI_GOAL_IGNORE_NEW:
			case AI_GOAL_STAY_NEAR_SHIP:
			case AI_GOAL_STAY_STILL:
				object* ptr;
				int inst, t;
				ptr = GET_FIRST(&obj_used_list);
				while (ptr != END_OF_LIST(&obj_used_list)) {
					if ((ptr->type == OBJ_SHIP) || (ptr->type == OBJ_START)) {
						inst = ptr->instance;
						if (ptr->type == OBJ_SHIP)
							t = TYPE_SHIP;
						else
							t = TYPE_PLAYER;

						Assert(inst >= 0 && inst < MAX_SHIPS);
						// remove all marked ships from list
						if ((_model->getGoal() != nullptr) && (ptr->flags[Object::Object_Flags::Marked])) {
							inst = -1;
						}

						// when docking, remove invalid dock targets
						else if (mode == AI_GOAL_DOCK) {
							if (_model->getShip() < 0 || !ship_docking_valid(_model->getShip(), inst))
								inst = -1;
						}

						// disallow ship being its own target
						if (inst >= 0 && inst != _model->getShip()) {
							objects[i]->addItem(Ships[inst].ship_name, QVariant(int(inst | t)));
							if (_model->getObject(i) == -1)
								_model->setObject(i, (inst | t));
							if (_model->getObject(i) == (inst | t))
								objects[i]->setCurrentIndex(objects[i]->findData((inst | t)));
						}
					}
					priority[i]->setEnabled(true);
					priority[i]->setValue(_model->getPriority(i));

					ptr = GET_NEXT(ptr);
				}
				break;
			}
			// for goals that deal with wings
			switch (mode) {
			case AI_GOAL_CHASE | AI_GOAL_CHASE_WING:
			case AI_GOAL_GUARD | AI_GOAL_GUARD_WING:
				int j;
				for (j = 0; j < MAX_WINGS; j++) {
					if (Wings[j].wave_count && j != _model->getWing()) {
						objects[i]->addItem(Wings[j].name, QVariant(int(j | TYPE_WING)));
						if (_model->getObject(i) == (j | TYPE_WING))
							objects[i]->setCurrentIndex(objects[i]->findData((j | TYPE_WING)));
					}
				}
				priority[i]->setEnabled(true);
				priority[i]->setValue(_model->getPriority(i));

				break;
			}
			if (mode == AI_GOAL_DESTROY_SUBSYSTEM) {
				subsys[i]->setEnabled(true);
				docks[i]->setEnabled(false);
				subsys[i]->clear();
				_model->setDock(i, -1);
				ship_subsys* cur_subsys;
				auto subsysvalue = _model->getSubsys(i);
				cur_subsys = GET_FIRST(&Ships[i].subsys_list);
				while (cur_subsys != END_OF_LIST(&Ships[i].subsys_list)) {
					subsys[i]->addItem(cur_subsys->system_info->subobj_name,
						QVariant(QString(cur_subsys->system_info->subobj_name)));
					cur_subsys = GET_NEXT(cur_subsys);
				}
				if (subsysvalue.empty()) {
					subsys[i]->setCurrentIndex(0);
				}
				else {
					subsys[i]->setCurrentIndex(subsys[i]->findData(subsysvalue.c_str()));
				}
				priority[i]->setEnabled(true);
				priority[i]->setValue(_model->getPriority(i));

			} else if (mode == AI_GOAL_DOCK) {
				SCP_vector<SCP_string> docklist = _viewport->editor->get_docking_list(
					Ship_info[Ships[_viewport->editor->cur_ship].ship_info_index].model_num);
				subsys[i]->setEnabled(true);
				subsys[i]->clear();
				auto subsysvalue = _model->getSubsys(i);
				int j;
				for (j = 0; unsigned(j) < docklist.size(); j++) {
					subsys[i]->addItem(docklist[j].c_str(), QVariant(QString(docklist[j].c_str())));
				}
				subsys[i]->setCurrentIndex(subsys[i]->findData(subsysvalue.c_str()));

				docklist = _viewport->editor->get_docking_list(
					Ship_info[Ships[_model->getDock(i) & DATA_MASK].ship_info_index].model_num);
				docks[i]->setEnabled(true);
				auto dockvalue = _model->getDock(i);
				docks[i]->clear();
				for (j = 0; unsigned(j) < docklist.size(); j++) {
					docks[i]->addItem(docklist[j].c_str(), QVariant(QString(docklist[j].c_str())));
				}
				docks[i]->setCurrentIndex(docks[i]->findData(dockvalue));
				priority[i]->setEnabled(true);
				priority[i]->setValue(_model->getPriority(i));

			} else {
				subsys[i]->setEnabled(false);
				docks[i]->setEnabled(false);
				_model->setDock(i, -1);
				SCP_string blank;
				_model->setSubsys(i, blank);
				priority[i]->setEnabled(true);
				priority[i]->setValue(_model->getPriority(i));

			}
		}
	}
} // namespace dialogs
} // namespace dialogs
} // namespace fred
} // namespace fso