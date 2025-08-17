#include <QDialog>
#include <QCloseEvent>

#include <ui/FredView.h>

#include "mission/dialogs/MissionSpecs/CustomWingNamesDialogModel.h"

namespace fso::fred::dialogs {

namespace Ui {
class CustomWingNamesDialog;
}

class CustomWingNamesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CustomWingNamesDialog(QWidget* parent, EditorViewport* viewport);
    ~CustomWingNamesDialog() override;

	void accept() override;
	void reject() override;

	void setInitialStartingWings(const std::array<SCP_string, MAX_STARTING_WINGS>& startingWings);
	void setInitialSquadronWings(const std::array<SCP_string, MAX_SQUADRON_WINGS>& squadronWings);
	void setInitialTvTWings(const std::array<SCP_string, MAX_TVT_WINGS>& tvtWings);

	const std::array<SCP_string, MAX_STARTING_WINGS>& getStartingWings() const;
	const std::array<SCP_string, MAX_SQUADRON_WINGS>& getSquadronWings() const;
	const std::array<SCP_string, MAX_TVT_WINGS>& getTvTWings() const;

protected:
	void closeEvent(QCloseEvent* e) override;

private slots:
	void on_okAndCancelButtons_accepted();
	void on_okAndCancelButtons_rejected();
	void on_startingWing_1_textChanged(const QString& text);
	void on_startingWing_2_textChanged(const QString& text);
	void on_startingWing_3_textChanged(const QString& text);
	void on_squadronWing_1_textChanged(const QString& text);
	void on_squadronWing_2_textChanged(const QString& text);
	void on_squadronWing_3_textChanged(const QString& text);
	void on_squadronWing_4_textChanged(const QString& text);
	void on_squadronWing_5_textChanged(const QString& text);
	void on_dogfightWing_1_textChanged(const QString& text);
	void on_dogfightWing_2_textChanged(const QString& text);

private: // NOLINT(readability-redundant-access-specifiers)
    std::unique_ptr<Ui::CustomWingNamesDialog> ui;
	std::unique_ptr<CustomWingNamesDialogModel> _model;
	EditorViewport* _viewport;

	void updateUi();

	void startingWingChanged(const QString&, int);
	void squadronWingChanged(const QString&, int);
	void dogfightWingChanged(const QString&, int);
};

} // namespace fso::fred::dialogs
