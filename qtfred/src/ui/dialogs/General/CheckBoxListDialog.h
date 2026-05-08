#pragma once

#include <ui/widgets/FlagList.h>

#include <QDialog>
#include <QString>
#include <QVector>

namespace fso::fred::dialogs {

namespace Ui {
class CheckBoxListDialog;
}

class CheckBoxListDialog : public QDialog {
	Q_OBJECT
  public:
	explicit CheckBoxListDialog(QWidget* parent = nullptr);

	void setCaption(const QString& text);

	// Binary mode (backwards-compatible)
	void setOptions(const QVector<std::pair<QString, bool>>& options);
	QVector<bool> getCheckedStates() const;

	// Tristate / int-state mode
	void setOptions(const QVector<std::pair<QString, int>>& options);
	QVector<std::pair<QString, int>> getFlags() const;

	void setOptionDescriptions(const QVector<std::pair<QString, QString>>& descriptions);
	void setTristate(bool tristate);

  private:
	Ui::CheckBoxListDialog* ui;
};

} // namespace fso::fred::dialogs
