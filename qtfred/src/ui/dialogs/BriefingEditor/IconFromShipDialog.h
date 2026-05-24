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

	enum class SelectionKind {
		None,
		Ship,
		Wing
	};

	int selectedShipIndex() const;
	int selectedWingIndex() const;
	SelectionKind selectedKind() const;

private slots:
	void onFilterTextChanged(const QString& text);
	void onItemDoubleClicked(QTreeWidgetItem* item, int column);

private: // NOLINT(readability-redundant-access-specifiers)
	void populateTree();
	void applyFilter(const QString& text);

	BriefingEditorDialogModel* _model;
	QTreeWidget* _tree;
	QLineEdit* _filter;
	SelectionKind _selectedKind = SelectionKind::None;
	int _selectedShipIndex = -1;
	int _selectedWingIndex = -1;
};

} // namespace fso::fred::dialogs
