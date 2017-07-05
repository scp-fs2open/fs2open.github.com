#ifndef MISSIONSPECDIALOG_H
#define MISSIONSPECDIALOG_H

#include <QtWidgets/QDialog>
#include <QAbstractButton>

#include <ui/FredView.h>

#include <mission/dialogs/MissionSpecDialogModel.h>

namespace fso {
namespace fred {
namespace dialogs {

namespace Ui {
class MissionSpecDialog;
}

class MissionSpecDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MissionSpecDialog(FredView* parent, EditorViewport* viewport);
    ~MissionSpecDialog();

protected:
	void closeEvent(QCloseEvent*) override;

private:
    std::unique_ptr<Ui::MissionSpecDialog> ui;
	std::unique_ptr<MissionSpecDialogModel> _model;
	EditorViewport* _viewport;

	void updateUI();

	void updateMissionType();
	void updateFlags();
	void updateTextEditors();

	void missionTitleChanged();
	void missionDesignerChanged();

	void singleRadioToggled(bool);
	void multiRadioToggled(bool);
	void trainingRadioToggled(bool);
	void coopRadioToggled(bool);
	void multiTeamRadioToggled(bool);
	void dogfightRadioToggled(bool);

	void flagToggled(bool enabled, Mission::Mission_Flags flag);

	void missionDescChanged();
	void designerNotesChanged();

	void aiProfileIndexChanged(int index);
	
};

}
}
}

#endif // MISSIONSPECDIALOG_H
