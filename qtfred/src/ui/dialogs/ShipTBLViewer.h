#pragma once

#include <mission/dialogs/ShipTBLViewerModel.h>

#include <QtWidgets/QDialog>

namespace fso {
namespace fred {
namespace dialogs {

namespace Ui {
class ShipTBLViewer;
}

class ShipTBLViewer : public QDialog {
	Q_OBJECT

  public:
	explicit ShipTBLViewer(QWidget* parent, EditorViewport* viewport, int sc);
	~ShipTBLViewer() override;

  protected:
	void closeEvent(QCloseEvent*) override;

  private:
	std::unique_ptr<Ui::ShipTBLViewer> ui;
	std::unique_ptr<ShipTBLViewerModel> _model;
	EditorViewport* _viewport;

	void updateUI();
};
} // namespace dialogs
} // namespace fred
} // namespace fso