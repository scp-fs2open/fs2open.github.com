#pragma once

#include <mission/dialogs/MusicTBLViewerModel.h>

#include <QtWidgets/QDialog>

namespace fso::fred::dialogs {

namespace Ui {
class ShipTBLViewer;
}
class MusicTBLViewer : public QDialog {
	Q_OBJECT

  public:
	explicit MusicTBLViewer(QWidget* parent, EditorViewport* viewport);
	~MusicTBLViewer() override;

  protected:
	void closeEvent(QCloseEvent*) override;

  private:
	std::unique_ptr<Ui::ShipTBLViewer> ui;
	std::unique_ptr<MusicTBLViewerModel> _model;
	EditorViewport* _viewport;

	void updateUi();
};
} // namespace fso::fred::dialogs