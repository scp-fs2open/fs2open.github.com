#pragma once

#include <QDialog>
#include <QTreeWidget>
#include <QLineEdit>

namespace fso::fred::dialogs {

class BriefingEditorDialogModel;

class IconFromShipDialog : public QDialog {
	Q_OBJECT

public:
	explicit IconFromShipDialog(QWidget* parent, BriefingEditorDialogModel* model);

	int selectedShipIndex() const;

private slots:
	void onFilterTextChanged(const QString& text);
	void onItemDoubleClicked(QTreeWidgetItem* item, int column);

private:
	void populateTree();
	void applyFilter(const QString& text);

	BriefingEditorDialogModel* _model;
	QTreeWidget* _tree;
	QLineEdit* _filter;
	int _selectedShipIndex = -1;
};

} // namespace fso::fred::dialogs
