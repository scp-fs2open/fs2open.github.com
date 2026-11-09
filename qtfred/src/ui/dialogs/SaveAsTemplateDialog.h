#pragma once

#include <QDialog>

#include "missioneditor/missionsave.h"

namespace fso::fred::dialogs {

namespace Ui {
class SaveAsTemplateDialog;
}

class SaveAsTemplateDialog : public QDialog {
	Q_OBJECT

  public:
	explicit SaveAsTemplateDialog(QWidget* parent, const SCP_string& defaultAuthor);
	~SaveAsTemplateDialog() override;

	MissionTemplateInfo templateInfo() const;

  private:
	void onTitleChanged(const QString& text);
	void onAccepted();

	std::unique_ptr<Ui::SaveAsTemplateDialog> ui;
};

} // namespace fso::fred::dialogs
