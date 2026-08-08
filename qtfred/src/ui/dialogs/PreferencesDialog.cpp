#include "PreferencesDialog.h"
#include "ui_PreferencesDialog.h"

#include <QFormLayout>
#include <QKeyEvent>
#include <QKeySequenceEdit>

#include "ui/QtGraphicsOperations.h"
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
// Returns empty if the option isn't registered, in which case the caller leaves the combo alone
// rather than showing a half-populated list.
SCP_vector<options::ValueDescription> engineOptionValues(const char* configKey)
{
	for (const auto& option : options::OptionsManager::instance()->getOptions()) {
		if (option->getConfigKey() == configKey && option->getType() == options::OptionType::Selection) {
			return option->getValidValues();
		}
	}

	return {};
}

void populateFromEngineOption(QComboBox* combo, const char* configKey)
{
	const auto values = engineOptionValues(configKey);

	for (const auto& value : values) {
		// Carry the value on the item rather than inferring it from the item's position. Enum- and
		// int-valued options serialize to their underlying integer (set_defaults() in
		// options/Option.h), so ValueDescription::serialized is the value itself.
		//
		// Position is emphatically not the same thing: the engine filters these lists at runtime --
		// shadows_remove_unsupported_options() already drops Graphics.ShadowRenderMethod's raytraced
		// entry on hardware that can't do it -- so the moment any of these gains a filtered entry,
		// index-as-value would silently start selecting the wrong thing.
		bool parsed = false;
		const int itemValue = QString::fromStdString(value.serialized).toInt(&parsed);
		if (!parsed) {
			mprintf(("PreferencesDialog: option %s has a non-integer value '%s'; skipping it.\n",
				configKey, value.serialized.c_str()));
			continue;
		}

		combo->addItem(QString::fromStdString(value.display), itemValue);
	}

	// Nothing to choose from means the option isn't registered in this build (a renderer compiled
	// out, say). Grey the control out rather than leaving an empty combo that looks broken.
	combo->setEnabled(combo->count() > 0);
}

// Select the item carrying @p value, leaving the combo alone if nothing does -- which is what a
// setting the engine filtered out of this session's list looks like.
void selectComboValue(QComboBox* combo, int value)
{
	const int index = combo->findData(value);
	if (index >= 0) {
		combo->setCurrentIndex(index);
	}
}

// The value the user picked, or @p fallback while the combo is empty or has no selection.
int currentComboValue(const QComboBox* combo, int fallback)
{
	const auto data = combo->currentData();
	return data.isValid() ? data.toInt() : fallback;
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
	_fredView->restartAutosaveTimer();
	_viewport->needsUpdate();
}

void PreferencesDialog::initializeUi() {
	// setupUi() has already run connectSlotsByName(), so filling a combo here would fire its
	// currentIndexChanged slot and mark the model modified before the user has touched anything.
	// (Items declared in the .ui file were added before the connections existed and so were safe.)
	util::SignalBlockers blockers(this);

	// Every combo on this tab carries its value as item data, so nothing below depends on an item's
	// position -- see populateFromEngineOption().
	ui->backendCombo->addItem(tr("OpenGL"), static_cast<int>(GraphicsAPI::OpenGL));
#if QTFRED_HAS_VULKAN
	ui->backendCombo->addItem(tr("Vulkan"), static_cast<int>(GraphicsAPI::Vulkan));
#endif

	populateFromEngineOption(ui->shadowQualityCombo, "Graphics.Shadows");
	// Only offers Raytraced on a Vulkan build/session with raytraced-shadow hardware support --
	// shadows_remove_unsupported_options() removes Graphics.ShadowRenderMethod entirely otherwise,
	// so engineOptionValues() comes back empty and the combo greys itself out. That is also why
	// there is no separate Vulkan-mode check here: this option already only exists when Vulkan (and
	// the hardware) can actually honour it.
	populateFromEngineOption(ui->shadowMethodCombo, "Graphics.ShadowRenderMethod");
	// Raytraced shadows need Vulkan ray-query hardware support; shadows_remove_unsupported_options()
	// responds by dropping Graphics.ShadowRenderMethod from the options list entirely rather than
	// just filtering its Raytraced value, so populateFromEngineOption() above leaves the combo with
	// no items at all (e.g. whenever qtFRED is running under OpenGL). An empty, greyed-out combo
	// reads as broken; show the one method that's actually in effect instead, locked in place.
	if (ui->shadowMethodCombo->count() == 0) {
		ui->shadowMethodCombo->addItem(tr("Shadow Maps"), static_cast<int>(ShadowRenderMethod::ShadowMap));
		ui->shadowMethodCombo->setEnabled(false);
	}
	populateFromEngineOption(ui->aaModeCombo, "Graphics.AAMode");
	populateFromEngineOption(ui->textureFilterCombo, "Graphics.TextureFilter");

	// Anisotropy levels are hardware-dependent, so ask the engine for the same list its own
	// options screen offers. Empty means the hardware can't do it; leave the combo disabled.
	for (float level : gr_get_supported_anisotropy_levels()) {
		ui->anisotropyCombo->addItem(level <= 1.0f ? tr("Off") : tr("%1x").arg(level, 0, 'f', 0), level);
	}
	ui->anisotropyCombo->setEnabled(ui->anisotropyCombo->count() > 0);

	for (int samples : GraphicsSettings::validMsaaSampleCounts()) {
		ui->msaaCombo->addItem(samples == 0 ? tr("Off") : tr("%1x").arg(samples), samples);
	}

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

	const int iconSize = _model->getToolbarIconSize();
	ui->toolbarIconSizeCombo->setCurrentIndex(iconSize <= 16 ? 0 : iconSize >= 32 ? 2 : 1);
	ui->outlineLodCombo->setCurrentIndex(_model->getOutlineLod());

	// Graphics
	const GraphicsSettings& graphics = _model->getGraphics();

	// selectComboValue() leaves the combo alone when nothing carries the value, which covers both
	// "not chosen" sentinels and a choice this session can't offer (Vulkan on a build without it,
	// Raytraced on hardware without it). Each combo keeps its first item selected in that case --
	// and because updateUi() runs under a SignalBlockers guard, that display fallback never fires a
	// slot and never turns itself into an explicit choice. See GraphicsSettings::backend.
	selectComboValue(ui->backendCombo, static_cast<int>(graphics.backend));
	ui->enablePostProcessing->setChecked(graphics.enablePostProcessing);
	selectComboValue(ui->shadowQualityCombo, static_cast<int>(graphics.shadowQuality));
	selectComboValue(ui->shadowMethodCombo, static_cast<int>(graphics.shadowMethod));
	selectComboValue(ui->aaModeCombo, static_cast<int>(graphics.aaMode));
	selectComboValue(ui->msaaCombo, graphics.msaaSamples);

	ui->gammaSpin->setValue(graphics.gamma);

	// Both of these can be "not chosen", in which case the engine picks: trilinear, and the
	// hardware's maximum anisotropy. Show what will actually be used rather than a blank.
	selectComboValue(ui->textureFilterCombo,
		graphics.textureFilter == GraphicsSettings::NO_TEXTURE_FILTER_CHOICE ? 1 : graphics.textureFilter);

	if (ui->anisotropyCombo->count() > 0) {
		const int level = ui->anisotropyCombo->findData(graphics.anisotropy);
		ui->anisotropyCombo->setCurrentIndex(level >= 0 ? level : ui->anisotropyCombo->count() - 1);
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

void PreferencesDialog::on_themeCombo_currentIndexChanged(int index) {
	_model->setThemeMode(themeModeFromIndex(index));
}

// Every graphics combo carries its value as item data, so each slot reads currentData() rather than
// mapping from the index. The fallbacks below are only reached if a slot somehow fires while the
// combo has no selection, in which case leaving the setting as it was is the right answer.
void PreferencesDialog::on_backendCombo_currentIndexChanged(int /*index*/) {
	const auto backend = static_cast<GraphicsAPI>(
		currentComboValue(ui->backendCombo, static_cast<int>(_model->getGraphics().backend)));
	editGraphics(_model.get(), [=](GraphicsSettings& g) { g.backend = backend; });
}

void PreferencesDialog::on_enablePostProcessing_toggled(bool checked) {
	editGraphics(_model.get(), [=](GraphicsSettings& g) { g.enablePostProcessing = checked; });
}

void PreferencesDialog::on_shadowQualityCombo_currentIndexChanged(int /*index*/) {
	const auto quality = static_cast<ShadowQuality>(
		currentComboValue(ui->shadowQualityCombo, static_cast<int>(_model->getGraphics().shadowQuality)));
	editGraphics(_model.get(), [=](GraphicsSettings& g) { g.shadowQuality = quality; });
}

void PreferencesDialog::on_shadowMethodCombo_currentIndexChanged(int /*index*/) {
	const auto method = static_cast<ShadowRenderMethod>(
		currentComboValue(ui->shadowMethodCombo, static_cast<int>(_model->getGraphics().shadowMethod)));
	editGraphics(_model.get(), [=](GraphicsSettings& g) { g.shadowMethod = method; });
}

void PreferencesDialog::on_aaModeCombo_currentIndexChanged(int /*index*/) {
	const auto mode = static_cast<AntiAliasMode>(
		currentComboValue(ui->aaModeCombo, static_cast<int>(_model->getGraphics().aaMode)));
	editGraphics(_model.get(), [=](GraphicsSettings& g) { g.aaMode = mode; });
}

void PreferencesDialog::on_msaaCombo_currentIndexChanged(int /*index*/) {
	const int samples = currentComboValue(ui->msaaCombo, _model->getGraphics().msaaSamples);
	editGraphics(_model.get(), [=](GraphicsSettings& g) { g.msaaSamples = samples; });
}

void PreferencesDialog::on_textureFilterCombo_currentIndexChanged(int /*index*/) {
	const int filter = currentComboValue(ui->textureFilterCombo, _model->getGraphics().textureFilter);
	editGraphics(_model.get(), [=](GraphicsSettings& g) { g.textureFilter = filter; });
}

void PreferencesDialog::on_anisotropyCombo_currentIndexChanged(int /*index*/) {
	const auto selected = ui->anisotropyCombo->currentData();
	if (!selected.isValid()) {
		return;
	}

	const float level = selected.toFloat();
	editGraphics(_model.get(), [=](GraphicsSettings& g) { g.anisotropy = level; });
}

void PreferencesDialog::on_gammaSpin_valueChanged(double value) {
	editGraphics(_model.get(), [=](GraphicsSettings& g) { g.gamma = static_cast<float>(value); });
}

void PreferencesDialog::on_dataMenuStyleCombo_currentIndexChanged(int index) {
	_model->setDataMenuStyle(static_cast<DataMenuStyle>(index));
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

void PreferencesDialog::on_resetDefaultsButton_clicked() {
	_model->resetControlDefaults();
}

} // namespace fso::fred::dialogs
