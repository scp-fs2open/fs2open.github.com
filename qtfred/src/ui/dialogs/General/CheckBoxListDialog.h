#pragma once


#include <QDialog>
#include <QCheckBox>

namespace fso {
namespace fred {
namespace dialogs {

namespace Ui {
class CheckBoxListDialog;
}

class CheckBoxListDialog : public QDialog {
	Q_OBJECT
  public:
	explicit CheckBoxListDialog(QWidget* parent = nullptr);

	void setCaption(const QString& text);
	void setOptions(const QVector<std::pair<QString, bool>>& options);
	QVector<bool> getCheckedStates() const;

  private:
	Ui::CheckBoxListDialog* ui;
	QVector<QCheckBox*> _checkboxes;
};

} // namespace dialogs
} // namespace fred
} // namespace fso