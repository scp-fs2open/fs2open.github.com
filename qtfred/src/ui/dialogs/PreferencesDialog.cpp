#include "PreferencesDialog.h"
#include "ui_PreferencesDialog.h"

#include <QFormLayout>
#include <QKeyEvent>
#include <QKeySequenceEdit>

#include "ui/util/SignalBlockers.h"

namespace fso::fred::dialogs {
namespace {

class ControlKeySequenceEdit : public QKeySequenceEdit {
public:
	explicit ControlKeySequenceEdit(const QKeySequence& sequence, QWidget* parent)
		: QKeySequenceEdit(sequence, parent) {}

protected:
	void keyPressEvent(QKeyEvent* event) override {
		if (event->isAutoRepeat()) {
			event->accept();
			return;
		}

		const auto key = event->key();
		if (key == Qt::Key_unknown) {
			event->accept();
			return;
		}

		// Ignore modifier-only presses until a non-modifier key is pressed
		if (key == Qt::Key_Control || key == Qt::Key_Shift || key == Qt::Key_Alt || key == Qt::Key_Meta) {
			event->accept();
			return;
		}

		const auto mods = event->modifiers() & (Qt::ShiftModifier | Qt::ControlModifier | Qt::AltModifier | Qt::MetaModifier | Qt::KeypadModifier);
		setKeySequence(QKeySequence(static_cast<int>(key | mods)));
		event->accept();
	}
};

} // namespace

PreferencesDialog::PreferencesDialog(FredView* parent, EditorViewport* viewport)
	: QDialog(parent)
	, ui(new Ui::PreferencesDialog())
	, _model(new PreferencesDialogModel(this, viewport))
{
	ui->setupUi(this);

	initializeUi();
	updateUi();

	connect(_model.get(), &PreferencesDialogModel::modelChanged, this, &PreferencesDialog::updateUi);
}

PreferencesDialog::~PreferencesDialog() = default;

void PreferencesDialog::accept() {
	if (_model->apply()) {
		QDialog::accept();
	}
}

void PreferencesDialog::reject() {
	_model->reject();
	QDialog::reject();
}

void PreferencesDialog::initializeUi() {
	// Build the controls key-binding form dynamically from the registered bindings
	auto* form = new QFormLayout(ui->controlsFormWidget);
	auto& bindings = ControlBindings::instance();
	for (const auto& def : bindings.definitions()) {
		auto* edit = new ControlKeySequenceEdit(_model->getControlKey(def.action), ui->controlsFormWidget);
		_controlEditors.emplace(def.action, edit);
		form->addRow(def.label + ':', edit);
		connect(edit, &QKeySequenceEdit::keySequenceChanged, this, [this, action = def.action](const QKeySequence& seq) {
			_model->setControlKey(action, seq);
		});
	}
}

void PreferencesDialog::updateUi() {
	util::SignalBlockers blockers(this);

	// General
	ui->moveShipsWhenUndocking->setChecked(_model->getMoveShipsWhenUndocking());
	ui->alwaysSaveDisplayNames->setChecked(_model->getAlwaysSaveDisplayNames());
	ui->errorCheckerChecksForPotentialIssues->setChecked(_model->getErrorCheckerChecksForPotentialIssues());
	ui->showSexpHelpMissionEvents->setChecked(_model->getShowSexpHelpMissionEvents());
	ui->showSexpHelpMissionGoals->setChecked(_model->getShowSexpHelpMissionGoals());
	ui->showSexpHelpMissionCutscenes->setChecked(_model->getShowSexpHelpMissionCutscenes());
	ui->showSexpHelpShipEditor->setChecked(_model->getShowSexpHelpShipEditor());
	ui->showSexpHelpWingEditor->setChecked(_model->getShowSexpHelpWingEditor());

	// Grid plane selection and enabled state
	const auto plane = _model->getGridPlane();
	ui->xyPlaneRadio->setChecked(plane == GridPlane::XY);
	ui->xzPlaneRadio->setChecked(plane == GridPlane::XZ);
	ui->yzPlaneRadio->setChecked(plane == GridPlane::YZ);

	// Only the axis perpendicular to the selected plane is editable
	ui->gridCenterX->setEnabled(plane == GridPlane::YZ);
	ui->gridCenterY->setEnabled(plane == GridPlane::XZ);
	ui->gridCenterZ->setEnabled(plane == GridPlane::XY);

	ui->gridCenterX->setValue(_model->getGridCenterX());
	ui->gridCenterY->setValue(_model->getGridCenterY());
	ui->gridCenterZ->setValue(_model->getGridCenterZ());

	// Controls
	for (const auto& entry : _controlEditors) {
		entry.second->setKeySequence(_model->getControlKey(entry.first));
	}
}

void PreferencesDialog::on_moveShipsWhenUndocking_toggled(bool checked) {
	_model->setMoveShipsWhenUndocking(checked);
}

void PreferencesDialog::on_alwaysSaveDisplayNames_toggled(bool checked) {
	_model->setAlwaysSaveDisplayNames(checked);
}

void PreferencesDialog::on_errorCheckerChecksForPotentialIssues_toggled(bool checked) {
	_model->setErrorCheckerChecksForPotentialIssues(checked);
}

void PreferencesDialog::on_showSexpHelpMissionEvents_toggled(bool checked) {
	_model->setShowSexpHelpMissionEvents(checked);
}
void PreferencesDialog::on_showSexpHelpMissionGoals_toggled(bool checked) {
	_model->setShowSexpHelpMissionGoals(checked);
}
void PreferencesDialog::on_showSexpHelpMissionCutscenes_toggled(bool checked) {
	_model->setShowSexpHelpMissionCutscenes(checked);
}
void PreferencesDialog::on_showSexpHelpShipEditor_toggled(bool checked) {
	_model->setShowSexpHelpShipEditor(checked);
}
void PreferencesDialog::on_showSexpHelpWingEditor_toggled(bool checked) {
	_model->setShowSexpHelpWingEditor(checked);
}

void PreferencesDialog::on_xyPlaneRadio_toggled(bool checked) {
	if (checked) {
		_model->setGridPlane(GridPlane::XY);
	}
}

void PreferencesDialog::on_xzPlaneRadio_toggled(bool checked) {
	if (checked) {
		_model->setGridPlane(GridPlane::XZ);
	}
}

void PreferencesDialog::on_yzPlaneRadio_toggled(bool checked) {
	if (checked) {
		_model->setGridPlane(GridPlane::YZ);
	}
}

void PreferencesDialog::on_gridCenterX_valueChanged(int value) {
	_model->setGridCenterX(value);
}

void PreferencesDialog::on_gridCenterY_valueChanged(int value) {
	_model->setGridCenterY(value);
}

void PreferencesDialog::on_gridCenterZ_valueChanged(int value) {
	_model->setGridCenterZ(value);
}

void PreferencesDialog::on_resetGridButton_clicked() {
	_model->resetGrid();
}

void PreferencesDialog::on_resetDefaultsButton_clicked() {
	_model->resetControlDefaults();
}

} // namespace fso::fred::dialogs
