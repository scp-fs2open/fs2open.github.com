#include "ShipAltShipClass.h"

#include "ui_ShipAltShipClass.h"

#include <mission/util.h>
#include <ui/util/SignalBlockers.h>

#include <QCloseEvent>

namespace fso::fred::dialogs {
ShipAltShipClass::ShipAltShipClass(QDialog* parent, EditorViewport* viewport)
	: QDialog(parent), ui(new Ui::ShipAltShipClass()),
	  _model(new ShipAltShipClassModel(this, viewport)), _viewport(viewport)
{
	this->setFocus();
	ui->setupUi(this);
	initUI();
}

ShipAltShipClass::~ShipAltShipClass() = default;

void ShipAltShipClass::accept()
{ // If apply() returns true, close the dialog
	sync_data();
	if (_model->apply()) {
		QDialog::accept();
	}
	// else: validation failed, don’t close
}

void ShipAltShipClass::reject()
{ // Asks the user if they want to save changes, if any
	// If they do, it runs _model->apply() and returns the success value
	// If they don't, it runs _model->reject() and returns true
	sync_data();
	if (rejectOrCloseHandler(this, _model.get(), _viewport)) {
		QDialog::reject(); // actually close
	}
	// else: do nothing, don't close
}

void ShipAltShipClass::closeEvent(QCloseEvent* e)
{
	reject();
	e->ignore(); // Don't let the base class close the window
}

void ShipAltShipClass::on_buttonBox_accepted()
{
	accept();
}
void ShipAltShipClass::on_buttonBox_rejected()
{
	reject();
}

void ShipAltShipClass::on_addButton_clicked()
{
	auto item = generate_item(ui->shipCombo->currentData(Qt::UserRole).toInt(),
		ui->variableCombo->currentData(Qt::UserRole).toInt(),
		ui->defaultCheckbox->isChecked());
	if (item != nullptr) {
		dynamic_cast<QStandardItemModel*>(ui->classList->model())->appendRow(item);
	}
}

void ShipAltShipClass::on_insertButton_clicked()
{
	auto current = ui->classList->currentIndex();
	if (current.isValid()) {
		auto item = generate_item(ui->shipCombo->currentData(Qt::UserRole).toInt(),
			ui->variableCombo->currentData(Qt::UserRole).toInt(),
			ui->defaultCheckbox->isChecked());
		if (item != nullptr) {
			dynamic_cast<QStandardItemModel*>(ui->classList->model())->insertRow(current.row(), item);
			ui->classList->selectionModel()->clearSelection();
			ui->classList->setCurrentIndex(QModelIndex());
		}
	} else {
		on_addButton_clicked();
	}
}

void ShipAltShipClass::on_deleteButton_clicked()
{
	auto current = ui->classList->currentIndex();
	if (current.isValid()) {
		dynamic_cast<QStandardItemModel*>(ui->classList->model())->removeRow(current.row());
	}
}

void ShipAltShipClass::on_upButton_clicked()
{
	auto current = ui->classList->currentIndex();
	if (current.isValid()) {
		int row = current.row();
		if (row != 0) {
			auto oldrow = dynamic_cast<QStandardItemModel*>(ui->classList->model())->takeRow(row);
			dynamic_cast<QStandardItemModel*>(ui->classList->model())->insertRow(row - 1, oldrow.first());
			ui->classList->selectionModel()->clearSelection();
			ui->classList->setCurrentIndex(QModelIndex());
		}
	}
}

void ShipAltShipClass::on_downButton_clicked()
{
	auto current = ui->classList->currentIndex();
	if (current.isValid()) {
		int row = current.row();
		if (row != ui->classList->model()->rowCount() - 1) {
			auto oldrow = dynamic_cast<QStandardItemModel*>(ui->classList->model())->takeRow(row);
			dynamic_cast<QStandardItemModel*>(ui->classList->model())->insertRow(row + 1, oldrow.first());
			ui->classList->selectionModel()->clearSelection();
			ui->classList->setCurrentIndex(QModelIndex());
		}
	}
}

void ShipAltShipClass::on_shipCombo_currentIndexChanged(int index)
{
	auto current = ui->classList->currentIndex();
	if (current.isValid()) {
		if (ui->shipCombo->itemData(index, Qt::UserRole).toInt() == -1 && ui->variableCombo->model()->rowCount() > 1) {
			if (ui->variableCombo->currentData(Qt::UserRole) != -1) {
				on_variableCombo_currentIndexChanged(ui->variableCombo->currentIndex());
			} else {
				on_variableCombo_currentIndexChanged(1);
			}
		} else {
			QString classname = generate_name(ui->shipCombo->itemData(index, Qt::UserRole).toInt(), -1);
			ui->classList->model()->setData(current, classname, Qt::DisplayRole);
			ui->classList->model()->setData(current, ui->shipCombo->itemData(index, Qt::UserRole), Qt::UserRole + 1);
			ui->classList->model()->setData(current, -1, Qt::UserRole + 2);
		}
	}
	ui->classList->selectionModel()->clearSelection();
	ui->classList->setCurrentIndex(QModelIndex());
}

void ShipAltShipClass::on_variableCombo_currentIndexChanged(int index)
{
	auto current = ui->classList->currentIndex();
	if (current.isValid()) {
		if (ui->variableCombo->itemData(index, Qt::UserRole).toInt() == -1) {
			if (ui->shipCombo->currentData(Qt::UserRole) != -1) {
				on_shipCombo_currentIndexChanged(ui->shipCombo->currentIndex());
			} else {
				on_shipCombo_currentIndexChanged(1);
			}
		} else {
			int ship_class =
				ship_info_lookup(Sexp_variables[ui->variableCombo->itemData(index, Qt::UserRole).toInt()].text);
			QString classname = generate_name(ship_class, ui->variableCombo->itemData(index, Qt::UserRole).toInt());
			ui->classList->model()->setData(current, classname, Qt::DisplayRole);
			ui->classList->model()->setData(current, ship_class, Qt::UserRole + 1);
			ui->classList->model()->setData(current,
				ui->variableCombo->itemData(index, Qt::UserRole),
				Qt::UserRole + 2);
		}
	}
	ui->classList->selectionModel()->clearSelection();
	ui->classList->setCurrentIndex(QModelIndex());
}

void ShipAltShipClass::on_defaultCheckbox_toggled(bool toggled)
{
	auto current = ui->classList->currentIndex();
	if (current.isValid()) {
		ui->classList->model()->setData(current, toggled, Qt::UserRole);
	}
}
void ShipAltShipClass::initUI()
{
	alt_pool = new QStandardItemModel();
	for (auto& alt_class : _model->get_pool()) {
		auto item = generate_item(alt_class.ship_class, alt_class.variable_index, alt_class.default_to_this_class);
		if (item != nullptr) {
			alt_pool->appendRow(item);
		}
	}

	ui->classList->setModel(alt_pool);
	connect(ui->classList->selectionModel(),
		&QItemSelectionModel::currentChanged,
		this,
		&ShipAltShipClass::classListChanged);

	auto ship_pool = new QStandardItemModel();
	for (auto& ship : _model->get_classes()) {
		QString classname = ship.first.c_str();
		auto item = new QStandardItem(classname);
		item->setData(ship.second, Qt::UserRole);
		ship_pool->appendRow(item);
	}
	auto shipproxyModel = new InverseSortFilterProxyModel(this);
	shipproxyModel->setSourceModel(ship_pool);
	ui->shipCombo->setModel(shipproxyModel);
	auto variable_pool = new QStandardItemModel();
	for (auto& variable : _model->get_variables()) {
		QString classname = variable.first.c_str();
		auto item = new QStandardItem(classname);
		item->setData(variable.second, Qt::UserRole);
		variable_pool->appendRow(item);
	}
	ui->variableCombo->setModel(variable_pool);
	updateUI();
}

void ShipAltShipClass::updateUI()
{
	util::SignalBlockers blockers(this); // block signals while we set up the UI
	QModelIndexList* list;
	auto current = ui->classList->currentIndex();
	auto ship_class = -1;
	auto variable = -1;
	auto default_ship = false;
	if (current.isValid()) {
		ship_class = current.data(Qt::UserRole + 1).toInt();
		variable = current.data(Qt::UserRole + 2).toInt();
		default_ship = current.data(Qt::UserRole).toBool();
	}
	if (ui->variableCombo->model()->rowCount() <= 1) {
		dynamic_cast<InverseSortFilterProxyModel*>(ui->shipCombo->model())->setFilterFixedString("Set From Variable");
	}
	list = new QModelIndexList(
		ui->shipCombo->model()->match(ui->shipCombo->model()->index(0, 0), Qt::UserRole, ship_class));
	if (!list->empty()) {
		ui->shipCombo->setCurrentIndex(list->first().row());
	} else {
		if (ui->classList->model()->rowCount() != 0 && ship_class != -1) {
			_viewport->dialogProvider->showButtonDialog(DialogType::Error,
				"Error",
				"Illegal ship class.\n Resetting to -1",
				{DialogButton::Ok});
		}
		ui->shipCombo->setCurrentIndex(0);
	}

	auto varlist = new QModelIndexList(
		ui->variableCombo->model()->match(ui->variableCombo->model()->index(0, 0), Qt::UserRole, variable));
	if (!varlist->empty()) {
		ui->variableCombo->setCurrentIndex(varlist->first().row());
	} else {
		if (ui->classList->model()->rowCount() != 0) {
			_viewport->dialogProvider->showButtonDialog(DialogType::Error,
				"Error",
				"Illegal variable index.\n Resetting to -1",
				{DialogButton::Ok});
		}
		ui->variableCombo->setCurrentIndex(0);
	}

	if (ui->variableCombo->model()->rowCount() <= 1) {
		ui->variableCombo->setEnabled(false);
	}
	ui->defaultCheckbox->setChecked(default_ship);
}
void ShipAltShipClass::classListChanged(const QModelIndex& current)
{
	SCP_UNUSED(current);
	updateUI();
}
QStandardItem* ShipAltShipClass::generate_item(const int classid, const int variable, const bool default_ship)
{
	QString classname = generate_name(classid, variable);
	if (!classname.isEmpty()) {
		auto item = new QStandardItem(classname);
		item->setData(default_ship, Qt::UserRole);
		item->setData(classid, Qt::UserRole + 1);
		item->setData(variable, Qt::UserRole + 2);
		return item;
	} else {
		Warning(LOCATION,
			"Unable to generate item name.\n [%i] was the class id and [%i] the variable index.",
			classid,
			variable);
		return nullptr;
	}
}
QString ShipAltShipClass::generate_name(const int classid, const int variable)
{
	QString classname;
	if (variable != -1) {
		// NOLINTBEGIN(readability-simplify-boolean-expr)
		Assertion(variable > -1 && variable < MAX_SEXP_VARIABLES,
			"Variable index out of bounds!");
		Assertion(Sexp_variables[variable].type & SEXP_VARIABLE_STRING, "Variable type is not a string.");
		// NOLINTEND(readability-simplify-boolean-expr)
		classname = Sexp_variables[variable].variable_name;
		classname = classname + '[' + Sexp_variables[variable].text + ']';
	} else {
		if (classid >= 0 && classid < MAX_SHIP_CLASSES) {
			classname = Ship_info[classid].name;
		} else {
			classname = "Invalid Ship Class";
			Warning(LOCATION, "Invalid Ship Class Index [%i]", classid);
		}
	}
	return classname;
}
void ShipAltShipClass::sync_data() {
	SCP_vector<alt_class> new_pool;
	int n = ui->classList->model()->rowCount();
	for (int i = 0; i < n; i++) {
		alt_class new_list_item;
		new_list_item.default_to_this_class =
			dynamic_cast<QStandardItemModel*>(ui->classList->model())->index(i,0).data(Qt::UserRole).toInt();
		new_list_item.ship_class =
			dynamic_cast<QStandardItemModel*>(ui->classList->model())->index(i, 0).data(Qt::UserRole + 1).toInt();
		new_list_item.variable_index =
			dynamic_cast<QStandardItemModel*>(ui->classList->model())->index(i, 0).data(Qt::UserRole + 2).toInt();
		new_pool.push_back(new_list_item);
	}
	_model->sync_data(new_pool);
}
InverseSortFilterProxyModel::InverseSortFilterProxyModel(QObject* parent) : QSortFilterProxyModel(parent) {}
bool InverseSortFilterProxyModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
	bool accept = QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
	return !accept;
}
} // namespace fso::fred::dialogs