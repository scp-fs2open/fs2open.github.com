#include "PreferencesDialog.h"
#include "ui_PreferencesDialog.h"

#include <QFormLayout>
#include <QKeyEvent>
#include <QKeySequenceEdit>

#include "ui/util/SignalBlockers.h"
#include "ui/widgets/sexp_tree_view.h"

#include <options/Option.h>
#include <options/OptionsManager.h>

namespace fso::fred::dialogs {
namespace {

// The engine already describes each of these settings once, with its own value list and display
// names (Graphics.Shadows in shadows.cpp, Graphics.AAMode and Graphics.TextureFilter in 2d.cpp and
// gropengltexture.cpp). Reading the labels back out of those definitions keeps this dialog from
// carrying a second, silently-diverging copy of them.
//
// The values are enumerated in declaration order, which for these three is enum order, so a combo
// index is the enum value. Returns empty if the option isn't registered, in which case the caller
// leaves the combo alone rather than showing a half-populated list.
const options::OptionBase* findEngineOption(const char* configKey)
{
	for (const auto& option : options::OptionsManager::instance()->getOptions()) {
		if (option->getConfigKey() == configKey && option->getType() == options::OptionType::Selection) {
			return option.get();
		}
	}

	return nullptr;
}

SCP_vector<options::ValueDescription> engineOptionValues(const char* configKey)
{
	const auto* option = findEngineOption(configKey);
	return option ? option->getValidValues() : SCP_vector<options::ValueDescription>{};
}

// The combo index of the value that the engine uses now. This index agrees with the list that
// engineOptionValues() gives. Returns the fallback if the option is not registered, or if its
// value is not in the list.
int engineOptionCurrentIndex(const char* configKey, int fallback)
{
	const auto* option = findEngineOption(configKey);
	if (!option) {
		return fallback;
	}

	const auto current = option->getCurrentValueDescription();
	const auto values = option->getValidValues();
	for (size_t i = 0; i < values.size(); ++i) {
		if (values[i].serialized == current.serialized) {
			return static_cast<int>(i);
		}
	}

	return fallback;
}

void populateFromEngineOption(QComboBox* combo, const char* configKey)
{
	const auto values = engineOptionValues(configKey);

	for (const auto& value : values) {
		combo->addItem(QString::fromStdString(value.display));
	}

	// Nothing to choose from means the option isn't registered in this build (a renderer compiled
	// out, say). Grey the control out rather than leaving an empty combo that looks broken.
	combo->setEnabled(!values.empty());
}

// The model stores graphics settings as one struct, so an edit is read-modify-write. Doing it this
// way keeps every control's slot to the one line that actually differs.
template <typename F>
void editGraphics(PreferencesDialogModel* model, F&& mutate)
{
	GraphicsSettings graphics = model->getGraphics();
	mutate(graphics);
	model->setGraphics(graphics);
}

int themeModeToIndex(ThemeMode mode)
{
	switch (mode) {
	case ThemeMode::Light:
		return 1;
	case ThemeMode::Dark:
		return 2;
	case ThemeMode::System:
		break;
	}
	return 0;
}

ThemeMode themeModeFromIndex(int index)
{
	switch (index) {
	case 1:
		return ThemeMode::Light;
	case 2:
		return ThemeMode::Dark;
	default:
		return ThemeMode::System;
	}
}

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
	, _fredView(parent)
	, _viewport(viewport)
{
	ui->setupUi(this);

	initializeUi();
	updateUi();

	connect(_model.get(), &PreferencesDialogModel::modelChanged, this, &PreferencesDialog::updateUi);
	connect(_model.get(), &PreferencesDialogModel::modelChanged, this, &PreferencesDialog::applyChanges);
}

PreferencesDialog::~PreferencesDialog() = default;

void PreferencesDialog::applyChanges() {
	_model->apply();
	_fredView->setIconSize(QSize(_model->getToolbarIconSize(), _model->getToolbarIconSize()));
	_fredView->applyMenuLayout(_model->getClassicMenuLayout());
	_fredView->restartAutosaveTimer();
	_viewport->needsUpdate();
}

void PreferencesDialog::initializeUi() {
	// setupUi() has already run connectSlotsByName(), so filling a combo here would fire its
	// currentIndexChanged slot and mark the model modified before the user has touched anything.
	// (Items declared in the .ui file were added before the connections existed and so were safe.)
	util::SignalBlockers blockers(this);

	populateFromEngineOption(ui->shadowQualityCombo, "Graphics.Shadows");

	// Contact hardening needs a GPU feature. Without it, the setting has no effect.
	ui->shadowContactHardening->setEnabled(shadow_contact_hardening_supported());
	populateFromEngineOption(ui->aaModeCombo, "Graphics.AAMode");
	populateFromEngineOption(ui->textureFilterCombo, "Graphics.TextureFilter");

	// Anisotropy levels are hardware-dependent, so ask the engine for the same list its own
	// options screen offers. Empty means the hardware can't do it; leave the combo disabled.
	_anisotropyLevels = gr_get_supported_anisotropy_levels();
	for (float level : _anisotropyLevels) {
		// The levels are powers of two, so show them as integers. The 'g' format with precision 0
		// shows 16 as "2e+01".
		ui->anisotropyCombo->addItem(level <= 1.0f ? tr("Off") : tr("%1x").arg(static_cast<int>(level)));
	}
	ui->anisotropyCombo->setEnabled(!_anisotropyLevels.empty());

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
	ui->offerAutosaveRecovery->setChecked(_model->getOfferAutosaveRecovery());
	ui->autosaveIntervalSeconds->setValue(_model->getAutosaveIntervalSeconds());
	ui->sexpNumberEveryN->setValue(_model->getSexpNumberEveryN());
	ui->createBakOnSave->setChecked(_model->getCreateBakOnSave());
	ui->moveShipsWhenUndocking->setChecked(_model->getMoveShipsWhenUndocking());
	ui->alwaysSaveDisplayNames->setChecked(_model->getAlwaysSaveDisplayNames());
	ui->checkPotentialIssues->setChecked(_model->getCheckPotentialIssues());
	ui->applyAutoCorrections->setChecked(_model->getApplyAutoCorrections());
	ui->themeCombo->setCurrentIndex(themeModeToIndex(_model->getThemeMode()));
	ui->dataMenuStyleCombo->setCurrentIndex(static_cast<int>(_model->getDataMenuStyle()));
	ui->classicMenuLayout->setChecked(_model->getClassicMenuLayout());

	const int iconSize = _model->getToolbarIconSize();
	ui->toolbarIconSizeCombo->setCurrentIndex(iconSize <= 16 ? 0 : iconSize >= 32 ? 2 : 1);
	ui->outlineLodCombo->setCurrentIndex(_model->getOutlineLod());
	ui->labelFontScaleSpin->setValue(_model->getLabelFontScale());

	// Graphics
	const GraphicsSettings& graphics = _model->getGraphics();

	ui->enablePostProcessing->setChecked(graphics.enablePostProcessing);
	ui->shadowQualityCombo->setCurrentIndex(static_cast<int>(graphics.shadowQuality));
	ui->shadowContactHardening->setChecked(graphics.shadowContactHardening);
	ui->aaModeCombo->setCurrentIndex(static_cast<int>(graphics.aaMode));
	ui->gammaSpin->setValue(graphics.gamma);

	// The user can leave these two settings unset. Then the engine selects the value. Show the
	// value that the engine uses, not a value that qtFRED guesses.
	ui->textureFilterCombo->setCurrentIndex(graphics.textureFilter == GraphicsSettings::NO_TEXTURE_FILTER_CHOICE
		? engineOptionCurrentIndex("Graphics.TextureFilter", 1)
		: graphics.textureFilter);

	if (!_anisotropyLevels.empty()) {
		// The engine option gets its list from gr_get_supported_anisotropy_levels(), the same as
		// _anisotropyLevels. Thus the indices agree.
		const int lastIndex = static_cast<int>(_anisotropyLevels.size()) - 1;
		int index = engineOptionCurrentIndex("Graphics.Anisotropy", lastIndex);
		if (graphics.anisotropy != GraphicsSettings::NO_ANISOTROPY_CHOICE) {
			for (size_t i = 0; i < _anisotropyLevels.size(); ++i) {
				if (_anisotropyLevels[i] == graphics.anisotropy) {
					index = static_cast<int>(i);
					break;
				}
			}
		}
		ui->anisotropyCombo->setCurrentIndex(index);
	}

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
	ui->invertOrbitX->setChecked(_model->getInvertOrbitX());
	ui->invertOrbitY->setChecked(_model->getInvertOrbitY());
	ui->orbitSelectionMidpoint->setChecked(_model->getOrbitSelectionMidpoint());
	for (const auto& entry : _controlEditors) {
		entry.second->setKeySequence(_model->getControlKey(entry.first));
	}
}

void PreferencesDialog::on_offerAutosaveRecovery_toggled(bool checked) {
	_model->setOfferAutosaveRecovery(checked);
}

void PreferencesDialog::on_autosaveIntervalSeconds_valueChanged(int value) {
	_model->setAutosaveIntervalSeconds(value);
}

void PreferencesDialog::on_sexpNumberEveryN_valueChanged(int value) {
	_model->setSexpNumberEveryN(value);
	// setSexpNumberEveryN -> modelChanged -> applyChanges -> apply() has already committed the
	// new value to the viewport by now, so re-icon any open trees to renumber them live.
	sexp_tree_view::refreshAllInstances();
}

void PreferencesDialog::on_createBakOnSave_toggled(bool checked) {
	_model->setCreateBakOnSave(checked);
}

void PreferencesDialog::on_moveShipsWhenUndocking_toggled(bool checked) {
	_model->setMoveShipsWhenUndocking(checked);
}

void PreferencesDialog::on_alwaysSaveDisplayNames_toggled(bool checked) {
	_model->setAlwaysSaveDisplayNames(checked);
}

void PreferencesDialog::on_checkPotentialIssues_toggled(bool checked) {
	_model->setCheckPotentialIssues(checked);
}

void PreferencesDialog::on_applyAutoCorrections_toggled(bool checked) {
	_model->setApplyAutoCorrections(checked);
}

void PreferencesDialog::on_toolbarIconSizeCombo_currentIndexChanged(int index) {
	static constexpr int sizes[] = { 16, 24, 32 };
	_model->setToolbarIconSize(sizes[index]);
}

void PreferencesDialog::on_outlineLodCombo_currentIndexChanged(int index) {
	_model->setOutlineLod(index);
}

void PreferencesDialog::on_labelFontScaleSpin_valueChanged(double value) {
	_model->setLabelFontScale(value);
}

void PreferencesDialog::on_themeCombo_currentIndexChanged(int index) {
	_model->setThemeMode(themeModeFromIndex(index));
}

void PreferencesDialog::on_enablePostProcessing_toggled(bool checked) {
	editGraphics(_model.get(), [=](GraphicsSettings& g) { g.enablePostProcessing = checked; });
}

void PreferencesDialog::on_shadowQualityCombo_currentIndexChanged(int index) {
	editGraphics(_model.get(), [=](GraphicsSettings& g) { g.shadowQuality = static_cast<ShadowQuality>(index); });
}

void PreferencesDialog::on_shadowContactHardening_toggled(bool checked) {
	editGraphics(_model.get(), [=](GraphicsSettings& g) { g.shadowContactHardening = checked; });
}

void PreferencesDialog::on_aaModeCombo_currentIndexChanged(int index) {
	editGraphics(_model.get(), [=](GraphicsSettings& g) { g.aaMode = static_cast<AntiAliasMode>(index); });
}

void PreferencesDialog::on_textureFilterCombo_currentIndexChanged(int index) {
	editGraphics(_model.get(), [=](GraphicsSettings& g) { g.textureFilter = index; });
}

void PreferencesDialog::on_anisotropyCombo_currentIndexChanged(int index) {
	if (index < 0 || static_cast<size_t>(index) >= _anisotropyLevels.size()) {
		return;
	}

	editGraphics(_model.get(), [=](GraphicsSettings& g) { g.anisotropy = _anisotropyLevels[index]; });
}

void PreferencesDialog::on_gammaSpin_valueChanged(double value) {
	editGraphics(_model.get(), [=](GraphicsSettings& g) { g.gamma = static_cast<float>(value); });
}

void PreferencesDialog::on_dataMenuStyleCombo_currentIndexChanged(int index) {
	_model->setDataMenuStyle(static_cast<DataMenuStyle>(index));
}

void PreferencesDialog::on_classicMenuLayout_toggled(bool checked) {
	_model->setClassicMenuLayout(checked);
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

void PreferencesDialog::on_invertOrbitX_toggled(bool checked) {
	_model->setInvertOrbitX(checked);
}

void PreferencesDialog::on_invertOrbitY_toggled(bool checked) {
	_model->setInvertOrbitY(checked);
}

void PreferencesDialog::on_orbitSelectionMidpoint_toggled(bool checked) {
	_model->setOrbitSelectionMidpoint(checked);
}

void PreferencesDialog::on_resetDefaultsButton_clicked() {
	_model->resetControlDefaults();
}

} // namespace fso::fred::dialogs
