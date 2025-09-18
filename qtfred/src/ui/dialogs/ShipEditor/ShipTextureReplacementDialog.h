#pragma once

#include <mission/dialogs/ShipEditor/ShipTextureReplacementDialogModel.h>

#include <QAbstractListModel>
#include <QItemSelectionModel>
#include <QtWidgets/QDialog>

namespace fso::fred::dialogs {

namespace Ui {
class ShipTextureReplacementDialog;
}
// Model for mapping data to listview in Texture Replace dialog
class MapModel : public QAbstractListModel {
	Q_OBJECT
  public:
	MapModel(ShipTextureReplacementDialogModel*, QObject* parent);
	int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	ShipTextureReplacementDialogModel* _model;
};

class ShipTextureReplacementDialog : public QDialog {

	Q_OBJECT

  public:
	explicit ShipTextureReplacementDialog(QDialog* parent, EditorViewport* viewport, bool multiEdit);
	~ShipTextureReplacementDialog() override;

	void accept() override;
	void reject() override;

  protected:
	void closeEvent(QCloseEvent*) override;
  private slots:
	void on_buttonBox_accepted();
	void on_buttonBox_rejected();

	void on_newTextureLineEdit_editingFinished();
	void on_MiscTextureLineEdit_editingFinished();
	void on_GlowTextureLineEdit_editingFinished();
	void on_ShineTextureLineEdit_editingFinished();
	void on_NormalTextureLineEdit_editingFinished();
	void on_HeightTextureLineEdit_editingFinished();
	void on_AmbiantTextureLineEdit_editingFinished();
	void on_ReflectTextureLineEdit_editingFinished();

	void on_useMiscTexturecheckbox_toggled(bool);
	void on_useGlowTexturecheckbox_toggled(bool);
	void on_useShineTexturecheckbox_toggled(bool);
	void on_useNormalTexturecheckbox_toggled(bool);
	void on_useHeightTexturecheckbox_toggled(bool);
	void on_useAmbiantTexturecheckbox_toggled(bool);
	void on_useReflectTexturecheckbox_toggled(bool);

	void on_inheritMiscTexturecheckbox_toggled(bool);
	void on_inheritGlowTexturecheckbox_toggled(bool);
	void on_inheritShineTexturecheckbox_toggled(bool);
	void on_inheritNormalTexturecheckbox_toggled(bool);
	void on_inheritHeightTexturecheckbox_toggled(bool);
	void on_inheritAmbiantTexturecheckbox_toggled(bool);
	void on_inheritReflectTexturecheckbox_toggled(bool);

  private:// NOLINT(readability-redundant-access-specifiers)
	std::unique_ptr<Ui::ShipTextureReplacementDialog> ui;
	std::unique_ptr<ShipTextureReplacementDialogModel> _model;
	EditorViewport* _viewport;
	int row = 0;
	MapModel* listmodel;
	void updateUI();
	void updateUIFull();
};
} // namespace fso::fred::dialogs