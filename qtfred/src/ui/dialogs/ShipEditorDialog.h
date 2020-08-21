#ifndef SHIPDEDITORDIALOG_H
#define SHIPDEDITORDIALOG_H

#include <mission/dialogs/ShipEditorDialogModel.h>
#include <ui/FredView.h>

#include <QAbstractButton>
#include <QtWidgets/QDialog>

namespace fso {
namespace fred {
namespace dialogs {

namespace Ui {
class ShipEditorDialog;
}

class ShipEditorDialog : public QDialog {
	Q_OBJECT

  public:
	explicit ShipEditorDialog(FredView* parent, EditorViewport* viewport);
	~ShipEditorDialog() override;

  protected:
	void closeEvent(QCloseEvent*) override;

  private slots:
	//void on_textureReplacementButton_clicked();
	//void on_miscButton_clicked();
	//void on_initialStatusButton_clicked();
	//void on_initialOrdersButton_clicked();
	//void on_tblInfoButton_clicked();

  private:
	std::unique_ptr<Ui::ShipEditorDialog> ui;
	std::unique_ptr<ShipEditorDialogModel> _model;
	EditorViewport* _viewport;

	void updateUI();
	void updateColumnOne();
	void updateColumnTwo();
	void enableDisable();

	void shipNameChanged(const QString&);
	void shipClassChanged(int);
	void aiClassChanged(int);
	void teamChanged(int);
	void cargoChanged(int);

	void altNameChanged(int);
	void callsignChanged(int);
};
} // namespace dialogs
} // namespace fred
} // namespace fso

#endif // SHIPDEDITORDIALOG_H