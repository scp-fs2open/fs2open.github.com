#pragma once

#include "mission/dialogs/MissionSpecs/CustomStringsDialogModel.h"

#include <ui/FredView.h>

#include <QDialog>
#include <QStandardItemModel>

namespace fso::fred::dialogs {

namespace Ui {
class CustomStringsDialog;
}

class CustomStringsDialog final : public QDialog {
	Q_OBJECT
  public:

	explicit CustomStringsDialog(QWidget* parent, EditorViewport* viewport);
	~CustomStringsDialog() override;

	void accept() override;
	void reject() override;

	void setInitial(const SCP_vector<custom_string>& items);

	const SCP_vector<custom_string>& items() const { return _model->items(); }

  protected:
	void closeEvent(QCloseEvent*) override;

  private slots:
	// Top-row buttons
	void on_addButton_clicked();
	void on_updateButton_clicked();
	void on_removeButton_clicked();

	// Dialog buttons
	void on_okAndCancelButtons_accepted();
	void on_okAndCancelButtons_rejected();

  private: // NOLINT(readability-redundant-access-specifiers)
	void buildView();
	void refreshTable();
	void selectRow(int row);
	void loadRowIntoEditors(int row);
	custom_string editorsToEntry() const;
	void clearEditors();

	std::unique_ptr<Ui::CustomStringsDialog> ui;
	std::unique_ptr<CustomStringsDialogModel> _model;
	EditorViewport* _viewport;

	QStandardItemModel* _tableModel = nullptr;
};

} // namespace fso::fred::dialogs
