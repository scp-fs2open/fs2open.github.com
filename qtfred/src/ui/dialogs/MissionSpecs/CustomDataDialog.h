#pragma once

#include "mission/dialogs/MissionSpecs/CustomDataDialogModel.h"

#include <ui/FredView.h>

#include <QDialog>
#include <QStandardItemModel>

namespace fso::fred::dialogs {

namespace Ui {
class CustomDataDialog;
}

class CustomDataDialog final : public QDialog {
	Q_OBJECT
  public:
	explicit CustomDataDialog(QWidget* parent, EditorViewport* viewport);
	~CustomDataDialog() override;

	void accept() override;
	void reject() override;

	void setInitial(const SCP_map<SCP_string, SCP_string>& items);

	const SCP_map<SCP_string, SCP_string>& items() const
	{
		return _model->items();
	}

  protected:
	void closeEvent(QCloseEvent* e) override;

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
	std::pair<SCP_string, SCP_string> editorsToEntry() const;
	void clearEditors();

	std::unique_ptr<Ui::CustomDataDialog> ui;
	std::unique_ptr<CustomDataDialogModel> _model;
	EditorViewport* _viewport;

	QStandardItemModel* _tableModel = nullptr;
};

} // namespace fso::fred::dialogs
