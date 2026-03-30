#pragma once

#include <QDialog>
#include <memory>

#include <mission/EditorViewport.h>
#include <mission/dialogs/MissionStatsDialogModel.h>

namespace fso::fred::dialogs {

namespace Ui {
class MissionStatsDialog;
}

class MissionStatsDialog : public QDialog {
	Q_OBJECT

public:
	explicit MissionStatsDialog(QWidget* parent, EditorViewport* viewport);
	~MissionStatsDialog() override;

private:
	void populateSummaryTab();
	void populateShipsTab();
	void populateEscortTab();
	void populateHotkeysTab();

	std::unique_ptr<Ui::MissionStatsDialog> ui;
	std::unique_ptr<MissionStatsDialogModel> _model;
};

} // namespace fso::fred::dialogs
