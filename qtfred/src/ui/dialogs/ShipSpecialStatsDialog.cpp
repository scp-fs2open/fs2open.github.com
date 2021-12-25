#include "ShipSpecialStatsDialog.h"

#include "ui_ShipSpecialStatsDialog.h"

#include <ui/util/SignalBlockers.h>

#include <QCloseEvent>

namespace fso {
	namespace fred {
		namespace dialogs {

			ShipSpecialStatsDialog::ShipSpecialStatsDialog(QWidget* parent, EditorViewport* viewport) :
				QDialog(parent), ui(new Ui::ShipSpecialStatsDialog()), _model(new ShipSpecialStatsDialogModel(this, viewport)),
				_viewport(viewport)
			{
				ui->setupUi(this);

				connect(this, &QDialog::accepted, _model.get(), &ShipSpecialStatsDialogModel::apply);
				connect(this, &QDialog::rejected, _model.get(), &ShipSpecialStatsDialogModel::reject);
				connect(_model.get(), &AbstractDialogModel::modelChanged, this, &ShipSpecialStatsDialog::updateUI);

				connect(ui->explodeCheckBox, &QCheckBox::toggled, _model.get(), &ShipSpecialStatsDialogModel::setSpecialExp);
				connect(ui->damageSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), _model.get(), &ShipSpecialStatsDialogModel::setDamage);
				connect(ui->blastSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), _model.get(), &ShipSpecialStatsDialogModel::setBlast);
				connect(ui->iRadiusSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), _model.get(), &ShipSpecialStatsDialogModel::setInnerRadius);
				connect(ui->oRadiusSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), _model.get(), &ShipSpecialStatsDialogModel::setOuterRadius);
				connect(ui->createShockwaveCheckBox, &QCheckBox::toggled, _model.get(), &ShipSpecialStatsDialogModel::setShockwave);
				connect(ui->shockwaveSpeedSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), _model.get(), &ShipSpecialStatsDialogModel::setShockwaveSpeed);
				connect(ui->deathRollCheckBox, &QCheckBox::toggled, _model.get(), &ShipSpecialStatsDialogModel::setDeathRoll);
				connect(ui->rollTimeSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), _model.get(), &ShipSpecialStatsDialogModel::setRollDuration);

				connect(ui->specialShieldCheckBox, &QCheckBox::toggled, _model.get(), &ShipSpecialStatsDialogModel::setSpecialShield);
				connect(ui->shieldSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), _model.get(), &ShipSpecialStatsDialogModel::setShield);
				connect(ui->specialHullCheckbox, &QCheckBox::toggled, _model.get(), &ShipSpecialStatsDialogModel::setSpecialHull);
				connect(ui->hullSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), _model.get(), &ShipSpecialStatsDialogModel::setHull);

				updateUI();

				// Resize the dialog to the minimum size
				resize(QDialog::sizeHint());
			}

			ShipSpecialStatsDialog::~ShipSpecialStatsDialog() = default;

			void ShipSpecialStatsDialog::closeEvent(QCloseEvent* event)
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

			void ShipSpecialStatsDialog::updateUI()
			{
				util::SignalBlockers blockers(this);
				ui->explodeCheckBox->setChecked(_model->getSpecialExp());
				ui->damageSpinBox->setValue(_model->getDamage());
				ui->blastSpinBox->setValue(_model->getBlast());
				ui->iRadiusSpinBox->setValue(_model->getInnerRadius());
				ui->oRadiusSpinBox->setValue(_model->getOuterRadius());
				ui->createShockwaveCheckBox->setChecked(_model->getShockwave());
				ui->shockwaveSpeedSpinBox->setValue(_model->getShockwaveSpeed());
				ui->deathRollCheckBox->setChecked(_model->getDeathRoll());
				ui->rollTimeSpinBox->setValue(_model->getRollDuration());

				ui->specialShieldCheckBox->setChecked(_model->getSpecialShield());
				ui->shieldSpinBox->setValue(_model->getShield());
				ui->specialHullCheckbox->setChecked(_model->getSpecialHull());
				ui->hullSpinBox->setValue(_model->getHull());

				//Enable/Disable
				if (_model->getSpecialExp()) {
					ui->damageSpinBox->setEnabled(true);
					ui->blastSpinBox->setEnabled(true);
					ui->iRadiusSpinBox->setEnabled(true);
					ui->oRadiusSpinBox->setEnabled(true);
					ui->createShockwaveCheckBox->setEnabled(true);
					ui->deathRollCheckBox->setEnabled(true);
					if (_model->getShockwave()) {
						ui->shockwaveSpeedSpinBox->setEnabled(true);
					}
					else { ui->shockwaveSpeedSpinBox->setEnabled(false); }
					if (_model->getDeathRoll()) {
						ui->rollTimeSpinBox->setEnabled(true);
					}
					else { ui->rollTimeSpinBox->setEnabled(false); }
				}
				else {
					ui->damageSpinBox->setEnabled(false);
					ui->blastSpinBox->setEnabled(false);
					ui->iRadiusSpinBox->setEnabled(false);
					ui->oRadiusSpinBox->setEnabled(false);
					ui->createShockwaveCheckBox->setEnabled(false);
					ui->deathRollCheckBox->setEnabled(false);
					ui->shockwaveSpeedSpinBox->setEnabled(false);
					ui->rollTimeSpinBox->setEnabled(false);
				}
				ui->shieldSpinBox->setEnabled(_model->getSpecialShield());
				ui->hullSpinBox->setEnabled(_model->getSpecialHull());
			}
		}
	}
}