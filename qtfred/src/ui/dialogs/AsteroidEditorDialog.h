#pragma once

#include <QDialog>

#include <mission/dialogs/AsteroidEditorDialogModel.h>
#include <ui/FredView.h>

namespace fso::fred::dialogs {

namespace Ui {
class AsteroidEditorDialog;
}

class AsteroidEditorDialog : public QDialog
{
	Q_OBJECT
public:
	AsteroidEditorDialog(FredView* parent, EditorViewport* viewport);
	~AsteroidEditorDialog() override;

	void accept() override;
	void reject() override;

protected:
	void closeEvent(QCloseEvent* e) override; // funnel all Window X presses through reject()

// Utilize Qt's "slots" feature to automatically connect UI elements to functions with less code in the initializer
// As a benefit this also requires zero manual signal setup in the .ui file (which is less obvious to those unfamiliar with Qt)
// The naming convention here is on_<object name>_<signal name>(). Easy to read and understand.
private slots:
	// dialog controls
	void on_okAndCancelButtons_accepted();
	void on_okAndCancelButtons_rejected();

	// toggles
	void on_enabled_toggled(bool enabled);
	void on_innerBoxEnabled_toggled(bool enabled);
	void on_enhancedFieldEnabled_toggled(bool enabled);

	// field types
	void on_radioButtonActiveField_toggled(bool checked);
	void on_radioButtonPassiveField_toggled(bool checked);
	void on_radioButtonAsteroid_toggled(bool checked);
	void on_radioButtonDebris_toggled(bool checked);

	// basic values
	void on_spinBoxNumber_valueChanged(int num_asteroids);
	void on_lineEditAvgSpeed_textEdited(const QString& text);

	// box values
	void on_lineEdit_obox_minX_textEdited(const QString& text);
	void on_lineEdit_obox_minY_textEdited(const QString& text);
	void on_lineEdit_obox_minZ_textEdited(const QString& text);
	void on_lineEdit_obox_maxX_textEdited(const QString& text);
	void on_lineEdit_obox_maxY_textEdited(const QString& text);
	void on_lineEdit_obox_maxZ_textEdited(const QString& text);
	void on_lineEdit_ibox_minX_textEdited(const QString& text);
	void on_lineEdit_ibox_minY_textEdited(const QString& text);
	void on_lineEdit_ibox_minZ_textEdited(const QString& text);
	void on_lineEdit_ibox_maxX_textEdited(const QString& text);
	void on_lineEdit_ibox_maxY_textEdited(const QString& text);
	void on_lineEdit_ibox_maxZ_textEdited(const QString& text);

	// object selections
	void on_asteroidSelectButton_clicked();
	void on_debrisSelectButton_clicked();
	void on_shipSelectButton_clicked();

private: // NOLINT(readability-redundant-access-specifiers)
	void initializeUi();
	void updateUi();

	// Boilerplate
	EditorViewport* _viewport = nullptr;
	Editor* _editor = nullptr;
	std::unique_ptr<Ui::AsteroidEditorDialog> ui;
	std::unique_ptr<AsteroidEditorDialogModel> _model;

	// Validators
	QDoubleValidator _box_validator;
	QIntValidator _speed_validator;
};


} // namespace fso::fred::dialogs
