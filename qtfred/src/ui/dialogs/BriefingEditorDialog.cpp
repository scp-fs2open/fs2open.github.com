#include "BriefingEditorDialog.h"
#include "ui_BriefingEditorDialog.h"
#include <QtWidgets/QMenuBar>

namespace fso::fred::dialogs {

BriefingEditorDialog::BriefingEditorDialog(QWidget* parent) : QDialog(parent), ui(new Ui::BriefingEditorDialog)
{
	ui->setupUi(this);
}

BriefingEditorDialog::~BriefingEditorDialog() = default;

} // namespace fso::fred::dialogs
