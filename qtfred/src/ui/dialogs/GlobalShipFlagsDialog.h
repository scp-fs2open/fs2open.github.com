#pragma once

#include <QtWidgets/QDialog>

#include <mission/dialogs/GlobalShipFlagsDialogModel.h>
#include <ui/FredView.h>

namespace fso::fred::dialogs {

namespace Ui {
class GlobalShipFlagsDialog;
}

class GlobalShipFlagsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GlobalShipFlagsDialog(FredView* parent, EditorViewport* viewport);
	~GlobalShipFlagsDialog() override;

private slots:
	void on_noShieldsButton_clicked();
	void on_noSubspaceDriveButton_clicked();
	void on_primitiveSensorsButton_clicked();
	void on_affectedByGravityButton_clicked();

private: // NOLINT(readability-redundant-access-specifiers)
	EditorViewport * _viewport = nullptr;
    std::unique_ptr<Ui::GlobalShipFlagsDialog> ui;
	std::unique_ptr<GlobalShipFlagsDialogModel> _model;
};

} // namespace fso::fred::dialogs
