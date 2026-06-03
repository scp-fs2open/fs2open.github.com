#pragma once

#include <QtWidgets/QDialog>

#include <mission/dialogs/ShieldSystemDialogModel.h>
#include <ui/FredView.h>

namespace fso::fred::dialogs {

namespace Ui {
class ShieldSystemDialog;
}

class ShieldSystemDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ShieldSystemDialog(FredView* parent, EditorViewport* viewport);
	~ShieldSystemDialog() override;

private slots:
	void on_shipTypeCombo_currentIndexChanged(int index);
	void on_shipTeamCombo_currentIndexChanged(int index);

	void on_typeHasShieldRadio_toggled(bool checked);
	void on_typeNoShieldRadio_toggled(bool checked);
	void on_teamHasShieldRadio_toggled(bool checked);
	void on_teamNoShieldRadio_toggled(bool checked);

	void on_applyTypeButton_clicked();
	void on_applyTeamButton_clicked();

protected:
	void focusInEvent(QFocusEvent* e) override;

private:  // NOLINT(readability-redundant-access-specifiers)
	void initializeUi();
	void updateUi();

	FredView*        _fredView  = nullptr;
	EditorViewport*  _viewport  = nullptr;
    std::unique_ptr<Ui::ShieldSystemDialog> ui;
	std::unique_ptr<ShieldSystemDialogModel> _model;
};

} // namespace fso::fred::dialogs
