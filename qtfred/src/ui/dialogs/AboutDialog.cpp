//
//

#include "AboutDialog.h"

#include "ui_AboutDialog.h"

#include <QtGui/QDesktopServices>
#include <QtCore/QUrl>

#include <project.h>
#include <graphics/2d.h>

namespace fso {
namespace fred {
namespace dialogs {

AboutDialog::AboutDialog(QWidget* parent) : QDialog(parent), ui(new Ui::AboutDialog()) {
	ui->setupUi(this);

	connect(ui->buttonReportBug, &QPushButton::pressed, this, &AboutDialog::onBugPressed);
	connect(ui->buttonVisitForums, &QPushButton::pressed, this, &AboutDialog::onForumsPressed);
	connect(ui->buttonAboutQt, &QPushButton::pressed, this, &AboutDialog::onAboutQtPressed);

	QString graphicsAPI;
	switch(gr_screen.mode)
	{
	case GR_OPENGL:
		graphicsAPI = QString::fromUtf8("OpenGL");
		break;
	case GR_VULKAN:
		graphicsAPI = QString::fromUtf8("Vulkan");
		break;
	}

	ui->labelVersion->setText(tr("qtFRED - FreeSpace Editor, Version %1 %2").arg(FS_VERSION_FULL, graphicsAPI));
}
AboutDialog::~AboutDialog() {
}
void AboutDialog::onBugPressed() {
	QDesktopServices::openUrl(QUrl("https://github.com/scp-fs2open/fs2open.github.com/issues", QUrl::TolerantMode));
}
void AboutDialog::onForumsPressed() {
	QDesktopServices::openUrl(QUrl("https://www.hard-light.net/forums/", QUrl::TolerantMode));
}
void AboutDialog::onAboutQtPressed() {
	QApplication::aboutQt();
}

}
}
}
