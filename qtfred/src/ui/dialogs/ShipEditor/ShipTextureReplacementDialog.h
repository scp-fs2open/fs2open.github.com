#pragma once

#include <mission/dialogs/ShipEditor/ShipTextureReplacementDialogModel.h>

#include <QAbstractListModel>
#include <QCheckBox>
#include <QItemSelectionModel>
#include <QLabel>
#include <QLineEdit>
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

  private: // NOLINT(readability-redundant-access-specifiers)
	struct TextureTypeRow {
		QLabel*    label;
		QCheckBox* useCheckbox;
		QCheckBox* inheritCheckbox;
		QLineEdit* lineEdit;
	};

	std::unique_ptr<Ui::ShipTextureReplacementDialog> ui;
	std::unique_ptr<ShipTextureReplacementDialogModel> _model;
	EditorViewport* _viewport;
	int _selectedRow = 0;
	MapModel* _listModel;
	SCP_map<SCP_string, TextureTypeRow> _textureRows;
	void updateUi();
	void updateUiFull();
};
} // namespace fso::fred::dialogs