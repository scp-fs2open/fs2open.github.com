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

	connect(this, &QDialog::accepted, _model.get(), &ShipFlagsDialogModel::apply);
	connect(ui->cancelButton, &QPushButton::clicked, this, &ShipFlagsDialog::rejectHandler);
	connect(this, &QDialog::rejected, _model.get(), &ShipFlagsDialogModel::reject);

	// Column One

		connect(ui->flagList, &fso::fred::FlagListWidget::flagToggled, this, [this](const QString& name, bool checked) {
		_model->setFlag(name.toUtf8().constData(), checked);
		updateUI();
	});

	const auto flags = _model->getFlagsList();

	QVector<std::pair<QString, int>> toWidget;
	toWidget.reserve(static_cast<int>(flags.size()));
	for (const auto& p : flags) {
		QString name = QString::fromUtf8(p.first.c_str());
		toWidget.append({name, p.second});
	}

	ui->flagList->setFlags(toWidget);
	updateUI();
	// Resize the dialog to the minimum size
	resize(QDialog::sizeHint());
}

ShipFlagsDialog::~ShipFlagsDialog() = default;

void ShipFlagsDialog::closeEvent(QCloseEvent* e)
{
	if (!rejectOrCloseHandler(this, _model.get(), _viewport)) {
		e->ignore();
	};
}
void ShipFlagsDialog::rejectHandler()
{
	this->close();
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
	ui->destroyedlabel->setVisible(_model->getFlag("Destroy before Mission")->second);
	ui->destroySecondsSpinBox->setVisible(_model->getFlag("Destroy before Mission")->second);
	ui->destroySecondsLabel->setVisible(_model->getFlag("Destroy before Mission")->second);

	ui->escortPrioritySpinBox->setValue(_model->getEscortPriority());
	ui->escortLabel->setVisible(_model->getFlag("escort")->second);
	ui->escortPrioritySpinBox->setVisible(_model->getFlag("escort")->second);

	ui->kamikazeDamageSpinBox->setValue(_model->getKamikazeDamage());
	ui->kamikazeLabel->setVisible(_model->getFlag("kamikaze")->second);
	ui->kamikazeDamageSpinBox->setVisible(_model->getFlag("kamikaze")->second);
}
} // namespace fso::fred::dialogs