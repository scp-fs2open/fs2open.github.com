#pragma once

#include <mission/dialogs/ShipTBLViewerModel.h>
#include <QtWidgets/QDialog>

#include "ShipEditorDialog.h"

namespace fso {
namespace fred {
namespace dialogs {

namespace Ui {
class ShipTBLViewer;
}
class ShipEditorDialog;
class ShipTBLViewer : public QDialog {
	Q_OBJECT

  public:
	explicit ShipTBLViewer(QWidget* parent, EditorViewport* viewport);
	~ShipTBLViewer() override;

  protected:
	void closeEvent(QCloseEvent*) override;
	void showEvent(QShowEvent* e) override;

  private:
	std::unique_ptr<Ui::ShipTBLViewer> ui;
	std::unique_ptr<ShipTBLViewerModel> _model;
	EditorViewport* _viewport;

	ShipEditorDialog* parentDialog = nullptr;

	int sc;

	void updateUI();
};
} // namespace dialogs
} // namespace fred
} // namespace fso