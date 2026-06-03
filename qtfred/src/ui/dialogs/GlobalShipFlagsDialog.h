#pragma once

#include <QtWidgets/QDialog>

#include <mission/commands/FredCommands.h>
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

protected:
	void focusInEvent(QFocusEvent* e) override;

private slots:
	void on_noShieldsButton_clicked();
	void on_noSubspaceDriveButton_clicked();
	void on_primitiveSensorsButton_clicked();
	void on_affectedByGravityButton_clicked();

private: // NOLINT(readability-redundant-access-specifiers)
	// Captures the post-op state and pushes a BatchFlagCommand, unless the
	// operation turned out to be a no-op (nothing to undo).
	void pushBatchFlagCommand(SCP_vector<BatchFlagCommand::ShipSnapshot> before, const QString& text);

	FredView*        _fredView  = nullptr;
	EditorViewport*  _viewport  = nullptr;
    std::unique_ptr<Ui::GlobalShipFlagsDialog> ui;
	std::unique_ptr<GlobalShipFlagsDialogModel> _model;
};

} // namespace fso::fred::dialogs
