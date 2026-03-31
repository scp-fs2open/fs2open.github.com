//

#include "MissionStatsDialog.h"

#include "ui_MissionStatsDialog.h"

#include <QFont>
#include <QProgressBar>
#include <QStringList>
#include <QTreeWidgetItem>

namespace fso::fred::dialogs {

MissionStatsDialog::MissionStatsDialog(QWidget* parent, EditorViewport* viewport)
	: QDialog(parent), ui(new Ui::MissionStatsDialog()),
	  _model(new MissionStatsDialogModel(this, viewport))
{
	ui->setupUi(this);
	populateSummaryTab();
	populateShipsTab();
	populateEscortTab();
	populateHotkeysTab();
}

MissionStatsDialog::~MissionStatsDialog() = default;

static void setBarValue(QProgressBar* bar, int value, int maximum)
{
	bar->setRange(0, maximum);
	bar->setValue(value);
	bar->setFormat(QStringLiteral("%v / %m"));

	float ratio = (maximum > 0) ? static_cast<float>(value) / maximum : 0.0f;
	const char* color;
	if (ratio >= 0.9f) {
		color = "#cc2222";
	} else if (ratio >= 0.6f) {
		color = "#e6a800";
	} else {
		color = "#2ecc71";
	}
	bar->setStyleSheet(QString("QProgressBar::chunk { background-color: %1; }").arg(color));
}

void MissionStatsDialog::populateSummaryTab()
{
	setBarValue(ui->objectsBar, _model->getNumObjects(), _model->getMaxObjects());
	setBarValue(ui->shipsBar,   _model->getShipCount(),  _model->getMaxShips());

	ui->labelShips->setText(QString::number(_model->getShipCount()));
	ui->labelProps->setText(QString::number(_model->getPropCount()));
	ui->labelWings->setText(QString::number(_model->getWingCount()));
	ui->labelWaypoints->setText(QString::number(_model->getWaypointPathCount()));
	ui->labelJumpNodes->setText(QString::number(_model->getJumpNodeCount()));
	ui->labelMessages->setText(QString::number(_model->getMessageCount()));
	ui->labelEvents->setText(QString::number(_model->getEventCount()));
	ui->labelGoals->setText(QString::number(_model->getGoalCount()));
	ui->labelVariables->setText(QString::number(_model->getVariableCount()));
	ui->labelContainers->setText(QString::number(_model->getContainerCount()));
}

void MissionStatsDialog::populateShipsTab()
{
	QTreeWidget* tree = ui->shipsTree;
	QFont boldFont;
	boldFont.setBold(true);

	for (const auto& entry : _model->getShipsByIFF()) {
		const auto& iffName = entry.first;
		const auto& rc      = entry.second;

		auto* iffItem = new QTreeWidgetItem(tree);
		iffItem->setText(0, QString::fromUtf8(iffName.c_str()));
		iffItem->setText(1, QString::number(rc.total()));
		iffItem->setFont(0, boldFont);
		iffItem->setFont(1, boldFont);

		auto addRole = [&](const QString& name, int count) {
			if (count <= 0)
				return;
			auto* item = new QTreeWidgetItem(iffItem);
			item->setText(0, name);
			item->setText(1, QString::number(count));
		};

		addRole(tr("Fighters"), rc.fighter);
		addRole(tr("Bombers"),  rc.bomber);
		addRole(tr("Capitals"), rc.capital);
		addRole(tr("Other"),    rc.other);

		iffItem->setExpanded(true);
	}

	tree->resizeColumnToContents(0);
	tree->resizeColumnToContents(1);
}

void MissionStatsDialog::populateEscortTab()
{
	QTreeWidget* tree = ui->escortTree;

	const auto escortList = _model->getEscortList();
	if (escortList.empty()) {
		auto* noneItem = new QTreeWidgetItem(tree);
		noneItem->setText(0, tr("(none)"));
	} else {
		for (const auto& e : escortList) {
			auto* item = new QTreeWidgetItem(tree);
			item->setText(0, QString::fromUtf8(e.name.c_str()));
			item->setText(1, QString::number(e.priority));
		}
	}

	tree->resizeColumnToContents(0);
	tree->resizeColumnToContents(1);
}

void MissionStatsDialog::populateHotkeysTab()
{
	QTreeWidget* tree = ui->hotkeysTree;

	const auto hotkeyMap = _model->getHotkeyMap();
	if (hotkeyMap.empty()) {
		auto* noneItem = new QTreeWidgetItem(tree);
		noneItem->setText(0, tr("(none)"));
	} else {
		for (const auto& kv : hotkeyMap) {
			auto* item = new QTreeWidgetItem(tree);
			item->setText(0, tr("F%1").arg(kv.first + 5));
			QStringList names;
			for (const auto& name : kv.second)
				names.append(QString::fromUtf8(name.c_str()));
			item->setText(1, names.join(QStringLiteral(", ")));
		}
	}

	tree->resizeColumnToContents(0);
	tree->resizeColumnToContents(1);
}

} // namespace fso::fred::dialogs
