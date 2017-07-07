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

private slots:
	void on_squadronLogoButton_clicked();
	void on_lowResScreenButton_clicked();
	void on_highResScreenButton_clicked();

private:
    std::unique_ptr<Ui::MissionSpecDialog> ui;
	std::unique_ptr<MissionSpecDialogModel> _model;
	EditorViewport* _viewport;

	void updateUI();

	void updateMissionType();
	void updateCmdMessage();
	void updateMusic();
	void updateFlags();
	void updateAIProfiles();
	void updateTextEditors();

	void missionTitleChanged(const QString &);
	void missionDesignerChanged(const QString &);

	void missionTypeToggled(bool, int);

	void maxRespawnChanged(int);
	void respawnDelayChanged(int);

	void squadronNameChanged(const QString &);

	void disallowSupportChanged(bool);
	void hullRepairMaxChanged(double);
	void subsysRepairMaxChanged(double);

	void trailDisplaySpeedToggled(bool);
	void minTrailDisplaySpeedChanged(int);

	void cmdSenderChanged(int);
	void cmdPersonaChanged(int);

	void eventMusicChanged(int);
	void subEventMusicChanged(int);

	void flagToggled(bool enabled, Mission::Mission_Flags flag);

	void missionDescChanged();
	void designerNotesChanged();

	void aiProfileIndexChanged(int index);
	
};

}
}
}

#endif // MISSIONSPECDIALOG_H
