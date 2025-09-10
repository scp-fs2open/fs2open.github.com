#pragma once

#include <QDialog>

namespace fso::fred::dialogs {

namespace Ui {
class BriefingEditorDialog;
}

class BriefingEditorDialog : public QDialog {
	Q_OBJECT

  public:
	explicit BriefingEditorDialog(QWidget* parent = 0);
	~BriefingEditorDialog() override;

  private:
	Ui::BriefingEditorDialog* ui;
};

} // namespace fso::fred::dialogs
