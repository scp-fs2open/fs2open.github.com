#pragma once

#include <mission/dialogs/ShipEditor/WeaponsTBLViewerModel.h>

#include <QtWidgets/QDialog>

namespace fso::fred::dialogs {

namespace Ui {
class ShipTBLViewer;
}
class WeaponsTBLViewer : public QDialog {
	Q_OBJECT

  public:
	explicit WeaponsTBLViewer(QWidget* parent, EditorViewport* viewport, int wc);
	~WeaponsTBLViewer() override;

  protected:
	void closeEvent(QCloseEvent*) override;

  private:
	std::unique_ptr<Ui::ShipTBLViewer> ui;
	std::unique_ptr<WeaponsTBLViewerModel> _model;
	EditorViewport* _viewport;

	int sc;

	void updateUI();
};
} // namespace fso::fred::dialogs