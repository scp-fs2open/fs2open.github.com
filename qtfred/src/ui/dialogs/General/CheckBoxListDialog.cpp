#include "ui/dialogs/General/CheckBoxListDialog.h"

#include "ui_CheckBoxListDialog.h"

namespace fso::fred::dialogs {

CheckBoxListDialog::CheckBoxListDialog(QWidget* parent) : QDialog(parent), ui(new Ui::CheckBoxListDialog)
{
	ui->setupUi(this);
	setSizeGripEnabled(true);
}

void CheckBoxListDialog::setCaption(const QString& text)
{
	setWindowTitle(text);
}

void CheckBoxListDialog::setOptions(const QVector<std::pair<QString, bool>>& options)
{
	QVector<std::pair<QString, int>> intOptions;
	intOptions.reserve(options.size());
	for (const auto& [name, checked] : options)
		intOptions.append({name, checked ? Qt::Checked : Qt::Unchecked});
	ui->flagList->setFlags(intOptions);
}

void CheckBoxListDialog::setOptions(const QVector<std::pair<QString, int>>& options)
{
	ui->flagList->setFlags(options);
}

void CheckBoxListDialog::setOptionDescriptions(const QVector<std::pair<QString, QString>>& descriptions)
{
	ui->flagList->setFlagDescriptions(descriptions);
}

void CheckBoxListDialog::setTristate(bool tristate)
{
	ui->flagList->setTristate(tristate);
}

QVector<bool> CheckBoxListDialog::getCheckedStates() const
{
	QVector<bool> states;
	for (const auto& [name, state] : ui->flagList->getFlags())
		states.append(state == Qt::Checked);
	return states;
}

QVector<std::pair<QString, int>> CheckBoxListDialog::getFlags() const
{
	return ui->flagList->getFlags();
}

} // namespace fso::fred::dialogs
