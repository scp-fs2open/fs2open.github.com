#pragma once

#include <QDialog>
#include <memory>

namespace fso {
namespace fred {
namespace dialogs {

namespace Ui {
class AboutDialog;
}

class AboutDialog: public QDialog {
 Q_OBJECT

 public:
	explicit AboutDialog(QWidget* parent);
	~AboutDialog();
 private:
	void onBugPressed();
	void onForumsPressed();
	void onAboutQtPressed();

	std::unique_ptr<Ui::AboutDialog> ui;
};

}
}
}

