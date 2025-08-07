#pragma once

#include <QDialog>

#include <mission/dialogs/AsteroidEditorDialogModel.h>
#include <ui/FredView.h>

namespace fso {
namespace fred {
namespace dialogs {

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
    void closeEvent(QCloseEvent* e) override;

private slots:

	void on_okAndCancelButtons_accepted();
	void on_okAndCancelButtons_rejected();

	void on_enabled_toggled(bool enabled);
	void on_innerBoxEnabled_toggled(bool enabled);
	void on_enhancedFieldEnabled_toggled(bool enabled);

	void on_radioButtonActiveField_toggled(bool checked);
	void on_radioButtonPassiveField_toggled(bool checked);
	void on_radioButtonAsteroid_toggled(bool checked);
	void on_radioButtonDebris_toggled(bool checked);

	void on_spinBoxNumber_valueChanged(int num_asteroids);
	void on_lineEditAvgSpeed_textEdited(const QString& text);

	void on_asteroidSelectButton_clicked();
	void on_debrisSelectButton_clicked();
	void on_shipSelectButton_clicked();

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

private:

	// Boilerplate
	EditorViewport* _viewport = nullptr;
	Editor* _editor = nullptr;
	std::unique_ptr<Ui::AsteroidEditorDialog> ui;
	std::unique_ptr<AsteroidEditorDialogModel> _model;

	// Validators
	QDoubleValidator _box_validator;
	QIntValidator _speed_validator;

	void initializeUi();
	void updateUi();
};


} // namespace dialogs
} // namespace fred
} // namespace fso
