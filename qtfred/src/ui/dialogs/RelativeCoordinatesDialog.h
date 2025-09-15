#pragma once

#include <QDialog>

#include "mission/dialogs/RelativeCoordinatesDialogModel.h"
#include <ui/FredView.h>

namespace fso::fred::dialogs {

namespace Ui {
class RelativeCoordinatesDialog;
}

class RelativeCoordinatesDialog final : public QDialog {
	Q_OBJECT
  public:
	explicit RelativeCoordinatesDialog(FredView* parent, EditorViewport* viewport);
	~RelativeCoordinatesDialog() override;

  private slots:
	void on_originListWidget_currentRowChanged(int row);
	void on_satelliteListWidget_currentRowChanged(int row);

  private: // NOLINT(readability-redundant-access-specifiers)
	void initializeUi();
	void updateUi();

	// Boilerplate
	EditorViewport* _viewport = nullptr;
	std::unique_ptr<Ui::RelativeCoordinatesDialog> ui;
	std::unique_ptr<RelativeCoordinatesDialogModel> _model;
};

} // namespace fred::dialogs
