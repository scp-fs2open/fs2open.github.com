#include "ShipFlagsDialog.h"

#include "ui_ShipFlagsDialog.h"

#include <mission/util.h>
#include <ui/util/SignalBlockers.h>

#include <QCloseEvent>

namespace fso::fred::dialogs {

ShipFlagsDialog::ShipFlagsDialog(QWidget* parent, EditorViewport* viewport)
	: QDialog(parent), ui(new Ui::ShipFlagsDialog()), _model(new ShipFlagsDialogModel(this, viewport)),
	  _viewport(viewport)
{
	ui->setupUi(this);
	setAttribute(Qt::WA_AlwaysShowToolTips);

	connect(ui->flagList, &fso::fred::FlagListWidget::flagsChanged, this,
		[this](const QVector<std::pair<QString, int>>& snapshot) {
			for (const auto& [name, state] : snapshot) {
				_model->setFlag(name.toUtf8().constData(), state);
			}
			updateUI();
		});

	const auto& flags = _model->getFlagsList();

	QVector<std::pair<QString, int>> toWidget;
	toWidget.reserve(static_cast<int>(flags.size()));
	for (const auto& p : flags) {
		QString name = QString::fromUtf8(p.first.c_str());
		toWidget.append({name, p.second});
	}

	ui->flagList->setFlags(toWidget);

	const auto descs = _model->getShipFlagDescriptions();
	QVector<std::pair<QString, QString>> qtDescs;
	qtDescs.reserve(static_cast<int>(descs.size()));
	for (const auto& d : descs)
		qtDescs.append({QString::fromUtf8(d.first.c_str()), QString::fromUtf8(d.second.c_str())});
	ui->flagList->setFlagDescriptions(qtDescs);

	updateUI();
	// Resize the dialog to the minimum size
	resize(QDialog::sizeHint());
}

ShipFlagsDialog::~ShipFlagsDialog() = default;

void ShipFlagsDialog::accept() {
	// If apply() returns true, close the dialog
	if (_model->apply()) {
		QDialog::accept();
	}
	// else: validation failed, don't close
}

void ShipFlagsDialog::reject() {
	// Asks the user if they want to save changes, if any
	// If they do, it runs _model->apply() and returns the success value
	// If they don't, it runs _model->reject() and returns true
	if (rejectOrCloseHandler(this, _model.get(), _viewport)) {
		QDialog::reject(); // actually close
	}
	// else: do nothing, don't close
}

void ShipFlagsDialog::closeEvent(QCloseEvent* e)
{
	reject();
	e->ignore(); // Don't let the base class close the window
}
void ShipFlagsDialog::on_okAndCancelButtons_accepted()
{
	accept();
}
void ShipFlagsDialog::on_okAndCancelButtons_rejected()
{
	reject();
}
void ShipFlagsDialog::on_destroySecondsSpinBox_valueChanged(int value)
{
	_model->setDestroyTime(value);
}
void ShipFlagsDialog::on_escortPrioritySpinBox_valueChanged(int value)
{
	_model->setEscortPriority(value);
}
void ShipFlagsDialog::on_kamikazeDamageSpinBox_valueChanged(int value)
{
	_model->setKamikazeDamage(value);
}
void ShipFlagsDialog::updateUI()
{
	util::SignalBlockers blockers(this);
	ui->destroySecondsSpinBox->setValue(_model->getDestroyTime());
	auto* destroyFlag = _model->getFlag("Destroy before Mission");
	ui->destroyedlabel->setVisible(destroyFlag && destroyFlag->second);
	ui->destroySecondsSpinBox->setVisible(destroyFlag && destroyFlag->second);
	ui->destroySecondsLabel->setVisible(destroyFlag && destroyFlag->second);

	auto* escortFlag = _model->getFlag("escort");
	ui->escortPrioritySpinBox->setValue(_model->getEscortPriority());
	ui->escortLabel->setVisible(escortFlag && escortFlag->second);
	ui->escortPrioritySpinBox->setVisible(escortFlag && escortFlag->second);

	auto* kamikazeFlag = _model->getFlag("kamikaze");
	ui->kamikazeDamageSpinBox->setValue(_model->getKamikazeDamage());
	ui->kamikazeLabel->setVisible(kamikazeFlag && kamikazeFlag->second);
	ui->kamikazeDamageSpinBox->setVisible(kamikazeFlag && kamikazeFlag->second);
}
} // namespace fso::fred::dialogs