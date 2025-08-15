#include "ui/dialogs/General/CheckBoxListDialog.h"

#include "ui_CheckBoxListDialog.h"

#include <QCheckBox>
#include <QScrollArea>
#include <QVBoxLayout>

namespace fso::fred::dialogs {

CheckBoxListDialog::CheckBoxListDialog(QWidget* parent) : QDialog(parent), ui(new Ui::CheckBoxListDialog)
{
	ui->setupUi(this);

	// Allow resizing
	this->setSizeGripEnabled(true);

	// clear placeholder layout contents if any
	if (ui->checkboxContainer->layout()) {
		QLayoutItem* item;
		while ((item = ui->checkboxContainer->layout()->takeAt(0)) != nullptr) {
			delete item->widget();
			delete item;
		}
		delete ui->checkboxContainer->layout();
	}

	// Set a fresh layout
	auto* layout = new QVBoxLayout(ui->checkboxContainer);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->setSpacing(4);
}

void CheckBoxListDialog::setCaption(const QString& text)
{
	this->setWindowTitle(text);
}

void CheckBoxListDialog::setOptions(const QVector<std::pair<QString, bool>>& options)
{
	// Clear previous checkboxes
	for (auto* cb : _checkboxes) {
		cb->deleteLater();
	}
	_checkboxes.clear();

	auto* layout = qobject_cast<QVBoxLayout*>(ui->checkboxContainer->layout());
	if (!layout) {
		return;
	}

	for (const auto& [label, checked] : options) {
		auto* cb = new QCheckBox(label, this);
		cb->setChecked(checked);
		layout->addWidget(cb);
		_checkboxes.append(cb);
	}
	// Add spacer to push items to top
	layout->addStretch();
}

QVector<bool> CheckBoxListDialog::getCheckedStates() const
{
	QVector<bool> states;
	for (auto* cb : _checkboxes) {
		states.append(cb->isChecked());
	}
	return states;
}

} // namespace fso::fred::dialogs
