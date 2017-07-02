#include "MissionSpecDialog.h"

#include "ui_MissionSpecDialog.h"

#include <QCloseEvent>

namespace fso {
namespace fred {
namespace dialogs {

MissionSpecDialog::MissionSpecDialog(FredView* parent, EditorViewport* viewport) :
	QDialog(parent), ui(new Ui::MissionSpecDialog()), _model(new MissionSpecDialogModel(this, viewport)),
	_viewport(viewport) {
    ui->setupUi(this);

	connect(this, &QDialog::accepted, _model.get(), &MissionSpecDialogModel::apply);
	connect(this, &QDialog::rejected, _model.get(), &MissionSpecDialogModel::reject);

	connect(_model.get(), &AbstractDialogModel::modelChanged, this, &MissionSpecDialog::updateUI);

	connect(ui->missionDesigner, static_cast<void (QLineEdit::*)(const QString &)>(&QLineEdit::textEdited), this, &MissionSpecDialog::updateDesigner);
	updateUI();

	// Resize the dialog to the minimum size
	resize(QDialog::sizeHint());
}

MissionSpecDialog::~MissionSpecDialog() {
}

void MissionSpecDialog::closeEvent(QCloseEvent* event) {
	if (_model->query_modified()) {
		auto button = _viewport->dialogProvider->showButtonDialog(DialogType::Question, "Changes detected", "Do you want to keep your changes?",
		{ DialogButton::Yes, DialogButton::No, DialogButton::Cancel });

		if (button == DialogButton::Cancel) {
			event->ignore();
			return;
		}

		if (button == DialogButton::Yes) {
			accept();
			return;
		}
	}

	QDialog::closeEvent(event);
}

void MissionSpecDialog::updateUI() {
}

void MissionSpecDialog::updateDesigner(const QString &designerName) {
	_model->setDesigner(designerName.toStdString());
}

}
}
}
