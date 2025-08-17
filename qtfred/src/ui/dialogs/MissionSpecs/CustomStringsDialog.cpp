#include "CustomStringsDialog.h"

#include "ui_CustomStringsDialog.h"

#include <ui/util/SignalBlockers.h>
#include "mission/util.h"

#include <QHeaderView>
#include <QItemSelectionModel>
#include <QMessageBox>

namespace fso::fred::dialogs {

namespace {
enum Columns {
	ColKey = 0,
	ColValue = 1,
	ColumnCount = 2
};
} // namespace

CustomStringsDialog::CustomStringsDialog(QWidget* parent, EditorViewport* viewport)
	: QDialog(parent), ui(new Ui::CustomStringsDialog()), _model(new CustomStringsDialogModel(this, viewport)),
	  _viewport(viewport)
{
	ui->setupUi(this);

	buildView();
	refreshTable();

	// Initial selection if any rows exist
	if (_tableModel->rowCount() > 0) {
		selectRow(0);
	}
}

CustomStringsDialog::~CustomStringsDialog() = default;

void CustomStringsDialog::accept()
{
	// If apply() returns true, close the dialog
	if (_model->apply()) {
		QDialog::accept();
	}
	// else: validation failed, don’t close
}

void CustomStringsDialog::reject()
{
	// Custom reject or close logic because we need to handle talkback to Mission Specs
	if (_model->query_modified()) {
		auto button = _viewport->dialogProvider->showButtonDialog(fso::fred::DialogType::Question,
			"Changes detected",
			"Do you want to keep your changes?",
			{fso::fred::DialogButton::Yes, fso::fred::DialogButton::No, fso::fred::DialogButton::Cancel});

		if (button == fso::fred::DialogButton::Yes) {
			accept();
		}
		if (button == fso::fred::DialogButton::No) {
			_model->reject();
			QDialog::reject(); // actually close
		}
	} else {
		_model->reject();
		QDialog::reject();
	}
}

void CustomStringsDialog::closeEvent(QCloseEvent* e)
{
	reject();
	e->ignore(); // Don't let the base class close the window
}

void CustomStringsDialog::setInitial(const SCP_vector<custom_string>& items)
{
	_model->setInitial(items);

	// Rebuild view from the new working copy
	refreshTable();
	if (_tableModel->rowCount() > 0) {
		selectRow(0);
	} else {
		clearEditors();
	}
}

void CustomStringsDialog::buildView()
{
	_tableModel = new QStandardItemModel(this);
	_tableModel->setColumnCount(ColumnCount);
	_tableModel->setHorizontalHeaderLabels({tr("Key"), tr("Value")});

	ui->stringsTableView->setModel(_tableModel);
	ui->stringsTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui->stringsTableView->setSelectionMode(QAbstractItemView::SingleSelection);
	ui->stringsTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui->stringsTableView->verticalHeader()->setVisible(false);
	ui->stringsTableView->horizontalHeader()->setStretchLastSection(true);
	ui->stringsTableView->setSortingEnabled(false);

	// Make sure headers are not interactive
	auto* hdr = ui->stringsTableView->horizontalHeader();
	hdr->setSectionsClickable(false);        // no click/press behavior
	hdr->setSortIndicatorShown(false);       // hide sort arrow
	hdr->setHighlightSections(false);        // don’t change look when selected
	hdr->setSectionsMovable(false);          // no drag-to-reorder columns
	hdr->setFocusPolicy(Qt::NoFocus);   

	// When a selection is made then load the editors
	connect(ui->stringsTableView->selectionModel(),
		&QItemSelectionModel::selectionChanged,
		this,
		[this](const QItemSelection&, const QItemSelection&) {
			const auto idx = ui->stringsTableView->currentIndex();
			loadRowIntoEditors(idx.isValid() ? idx.row() : -1);
		});
}

void CustomStringsDialog::refreshTable()
{
	_tableModel->setRowCount(0);

	const auto& rows = _model->items();
	_tableModel->insertRows(0, static_cast<int>(rows.size()));
	for (int i = 0; i < static_cast<int>(rows.size()); ++i) {
		const auto& e = rows[static_cast<size_t>(i)];
		auto* keyItem = new QStandardItem(QString::fromStdString(e.name));
		auto* valItem = new QStandardItem(QString::fromStdString(e.value));
		keyItem->setEditable(false);
		valItem->setEditable(false);
		_tableModel->setItem(i, ColKey, keyItem);
		_tableModel->setItem(i, ColValue, valItem);
	}
}

void CustomStringsDialog::selectRow(int row)
{
	if (row < 0 || row >= _tableModel->rowCount()) {
		ui->stringsTableView->clearSelection();
		loadRowIntoEditors(-1);
		return;
	}
	ui->stringsTableView->selectRow(row);
	loadRowIntoEditors(row);
}

void CustomStringsDialog::loadRowIntoEditors(int row)
{
	util::SignalBlockers blockers(this);

	if (row < 0 || row >= _tableModel->rowCount()) {
		clearEditors();
		return;
	}

	const auto& e = _model->items()[static_cast<size_t>(row)];
	ui->keyLineEdit->setText(QString::fromStdString(e.name));
	ui->valueLineEdit->setText(QString::fromStdString(e.value));
	ui->stringTextEdit->setPlainText(QString::fromStdString(e.text));
}

custom_string CustomStringsDialog::editorsToEntry() const
{
	custom_string e;
	e.name = ui->keyLineEdit->text().toUtf8().constData();
	e.value = ui->valueLineEdit->text().toUtf8().constData();
	e.text = ui->stringTextEdit->toPlainText().toUtf8().constData();
	return e;
}

void CustomStringsDialog::clearEditors()
{
	util::SignalBlockers blockers(this);
	
	ui->keyLineEdit->clear();
	ui->valueLineEdit->clear();
	ui->stringTextEdit->clear();
}

void CustomStringsDialog::on_addButton_clicked()
{
	auto e = editorsToEntry();
	SCP_string err;
	if (!_model->add(e, &err)) {
		QMessageBox::warning(this, tr("Invalid Entry"), QString::fromStdString(err));
		return;
	}

	refreshTable();
	//selectRow(_tableModel->rowCount() - 1);
	clearEditors();
}

void CustomStringsDialog::on_updateButton_clicked()
{
	const auto idx = ui->stringsTableView->currentIndex();
	if (!idx.isValid())
		return;

	auto e = editorsToEntry();
	SCP_string err;
	if (!_model->updateAt(static_cast<size_t>(idx.row()), e, &err)) {
		QMessageBox::warning(this, tr("Invalid Entry"), QString::fromStdString(err));
		return;
	}

	// Reflect updated values in table
	_tableModel->item(idx.row(), ColKey)->setText(QString::fromStdString(e.name));
	_tableModel->item(idx.row(), ColValue)->setText(QString::fromStdString(e.value));
}

void CustomStringsDialog::on_removeButton_clicked()
{
	const auto idx = ui->stringsTableView->currentIndex();
	if (!idx.isValid())
		return;

	const auto key = _tableModel->item(idx.row(), ColKey)->text();
	if (QMessageBox::question(this, tr("Remove Entry"), tr("Remove key \"%1\"?").arg(key)) != QMessageBox::Yes) {
		return;
	}

	if (_model->removeAt(static_cast<size_t>(idx.row()))) {
		refreshTable();
		selectRow(std::min(idx.row(), _tableModel->rowCount() - 1));
	}
}

void CustomStringsDialog::on_okAndCancelButtons_accepted()
{
	accept(); // Mission Specs will read model->items() and commit during its own Apply
}

void CustomStringsDialog::on_okAndCancelButtons_rejected()
{
	reject();
}

} // namespace fso::fred::dialogs
