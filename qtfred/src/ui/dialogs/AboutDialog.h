#pragma once

#include <mission/EditorViewport.h>
#include <mission/dialogs/AboutDialogModel.h>

#include <QDialog>
#include <memory>

namespace fso::fred::dialogs {

namespace Ui {
class AboutDialog;
}

class AboutDialog : public QDialog {
	Q_OBJECT

public:
	explicit AboutDialog(QWidget* parent, EditorViewport* viewport);
	~AboutDialog() override;

private slots:
	void on_scpCreditsButton_clicked();
	void on_reportBugButton_clicked();
	void on_visitForumsButton_clicked();
	void on_aboutQtButton_clicked();

private: // NOLINT(readability-redundant-access-specifiers)
	std::unique_ptr<Ui::AboutDialog> ui;
	std::unique_ptr<AboutDialogModel> _model;
};

} // namespace fso::fred::dialogs
