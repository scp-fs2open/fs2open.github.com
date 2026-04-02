#include "TableViewerDialog.h"

#include "ui_TableViewerDialog.h"

#include <ui/util/SignalBlockers.h>

#include <QCloseEvent>

namespace fso::fred::dialogs {

TableViewerDialog::TableViewerDialog(QWidget* parent, EditorViewport* viewport, const QString& title,
                                     const char* table_filename, const char* modular_pattern)
	: QDialog(parent), ui(new Ui::TableViewerDialog()),
	  _model(new TableViewerModel(this, viewport, table_filename, modular_pattern)),
	  _viewport(viewport)
{
	init(title);
}

TableViewerDialog::TableViewerDialog(QWidget* parent, EditorViewport* viewport, const QString& title,
                                     const char* table_filename, const char* modular_pattern,
                                     const char* entry_name)
	: QDialog(parent), ui(new Ui::TableViewerDialog()),
	  _model(new TableViewerModel(this, viewport, table_filename, modular_pattern, entry_name)),
	  _viewport(viewport)
{
	init(title);
}

void TableViewerDialog::init(const QString& title)
{
	ui->setupUi(this);
	setWindowTitle(title);
	connect(_model.get(), &AbstractDialogModel::modelChanged, this, &TableViewerDialog::updateUI);
	updateUI();
}

TableViewerDialog::~TableViewerDialog() = default;

void TableViewerDialog::closeEvent(QCloseEvent* event)
{
	QDialog::closeEvent(event);
}

void TableViewerDialog::updateUI()
{
	util::SignalBlockers blockers(this);
	ui->TBLData->setPlainText(_model->getText().c_str());
}

} // namespace fso::fred::dialogs
