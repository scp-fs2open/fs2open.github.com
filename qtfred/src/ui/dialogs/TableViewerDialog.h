#pragma once

#include <mission/dialogs/TableViewerModel.h>

#include <QtWidgets/QDialog>

namespace fso::fred::dialogs {

namespace Ui {
class TableViewerDialog;
}

class TableViewerDialog : public QDialog {
	Q_OBJECT

  public:
	// For a full table (e.g. music.tbl)
	explicit TableViewerDialog(QWidget* parent, EditorViewport* viewport, const QString& title,
	                           const char* table_filename, const char* modular_pattern);
	// For a specific entry (e.g. a ship or weapon by name)
	explicit TableViewerDialog(QWidget* parent, EditorViewport* viewport, const QString& title,
	                           const char* table_filename, const char* modular_pattern,
	                           const char* entry_name);
	~TableViewerDialog() override;

  protected:
	void closeEvent(QCloseEvent*) override;

  private:
	std::unique_ptr<Ui::TableViewerDialog> ui;
	std::unique_ptr<TableViewerModel> _model;
	EditorViewport* _viewport;

	void init(const QString& title);
	void updateUI();
};

} // namespace fso::fred::dialogs
