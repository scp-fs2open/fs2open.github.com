#pragma once

#include <QDialog>

#include <mission/dialogs/ReorderDialogModel.h>
#include <ui/FredView.h>

class QListWidget;
class QAbstractButton;

namespace fso::fred::dialogs {

namespace Ui {
class ReorderDialog;
}

// Direct-edit dialog for reordering ships, wings, props, waypoint lists and jump
// nodes.  Each tab is a plain list plus a move-to-top / up / down / to-bottom
// button strip; every move is applied to the mission immediately.
class ReorderDialog : public QDialog {
	Q_OBJECT

public:
	ReorderDialog(FredView* parent, EditorViewport* viewport);
	~ReorderDialog() override;

private: // NOLINT(readability-redundant-access-specifiers)
	struct Tab {
		ReorderDialogModel::Type type;
		QListWidget* list;
		QAbstractButton* top;
		QAbstractButton* up;
		QAbstractButton* down;
		QAbstractButton* bottom;
	};

	void setupTab(const Tab& tab);
	void rebuildList(const Tab& tab);
	static void updateButtons(const Tab& tab);
	void move(const Tab& tab, bool up, bool all_the_way);

	EditorViewport* _viewport = nullptr;
	std::unique_ptr<Ui::ReorderDialog> ui;
	std::unique_ptr<ReorderDialogModel> _model;
	SCP_vector<Tab> _tabs;
};

} // namespace fso::fred::dialogs
