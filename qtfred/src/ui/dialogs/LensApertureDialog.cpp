#include "LensApertureDialog.h"
#include "ui/util/SignalBlockers.h"
#include "ui_LensApertureDialog.h"

#include <graphics/lens_flare.h>

#include <QtWidgets/QCheckBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QSlider>

#include <cmath>

namespace fso::fred::dialogs {

namespace {

using graphics::lens_settings;

// One row of the dialog: what it is called, how it is presented, and where its
// value lives in a lens_settings.
//
// This table is the dialog. The controls are built from it, read from it and
// written back through it, so a knob added to lens_settings reaches the editor by
// adding one line here rather than by being spelled out in a .ui file, a build
// list, a load function and a save function that can all drift apart.
//
// The accessors are captureless lambdas (hence plain function pointers) rather
// than member pointers because most of these live inside a nested struct, and a
// pointer-to-member chain through lens_aperture::grating::density is far less
// readable than just naming it.
struct lens_field {
	// Non-null starts a new group box, with this as its title.
	const char* section;
	const char* label;

	enum class kind {
		Real,    // a slider carrying a float, quantized to `decimals` places
		Integer, // a slider carrying a whole number
		Boolean, // a checkbox
	};
	kind type;

	float min;
	float max;
	int decimals; // Real only; also fixes the slider's step size

	float (*get)(const lens_settings&);
	void (*set)(lens_settings&, float);
};

// QSlider is integer-only, so a Real field rides a slider scaled by 10^decimals.
// Deriving the scale from the displayed precision rather than picking one per
// field is what keeps a value read back out of a slider equal to the value put
// in: anything the dialog can show, it can represent exactly.
float field_scale(const lens_field& f)
{
	return (f.type == lens_field::kind::Real) ? std::pow(10.0f, static_cast<float>(f.decimals)) : 1.0f;
}

// clang-format off
const lens_field Lens_fields[] = {
	{"Iris",        "Blades",                lens_field::kind::Integer,  0.0f,  64.0f, 0,
		[](const lens_settings& s) { return static_cast<float>(s.aperture.blades); },
		[](lens_settings& s, float v) { s.aperture.blades = static_cast<int>(std::lround(v)); }},
	{nullptr,       "Blade rotation (deg)",  lens_field::kind::Real,     0.0f, 180.0f, 1,
		[](const lens_settings& s) { return s.aperture.rotation; },
		[](lens_settings& s, float v) { s.aperture.rotation = v; }},
	{nullptr,       "Blade curvature",       lens_field::kind::Real,    -1.0f,   1.0f, 2,
		[](const lens_settings& s) { return s.aperture.curvature; },
		[](lens_settings& s, float v) { s.aperture.curvature = v; }},
	{nullptr,       "Edge softness",         lens_field::kind::Real,     0.0f,   0.5f, 4,
		[](const lens_settings& s) { return s.aperture.softness; },
		[](lens_settings& s, float v) { s.aperture.softness = v; }},

	{"Rim grating", "Strength",              lens_field::kind::Real,     0.0f,   1.0f, 2,
		[](const lens_settings& s) { return s.aperture.grating.strength; },
		[](lens_settings& s, float v) { s.aperture.grating.strength = v; }},
	{nullptr,       "Density",               lens_field::kind::Real,     0.0f,   1.0f, 2,
		[](const lens_settings& s) { return s.aperture.grating.density; },
		[](lens_settings& s, float v) { s.aperture.grating.density = v; }},
	{nullptr,       "Length",                lens_field::kind::Real,     0.0f,   1.0f, 2,
		[](const lens_settings& s) { return s.aperture.grating.length; },
		[](lens_settings& s, float v) { s.aperture.grating.length = v; }},
	{nullptr,       "Width",                 lens_field::kind::Real,     0.0f,   1.0f, 2,
		[](const lens_settings& s) { return s.aperture.grating.width; },
		[](lens_settings& s, float v) { s.aperture.grating.width = v; }},
	{nullptr,       "Softness",              lens_field::kind::Real,     0.0f,   0.5f, 4,
		[](const lens_settings& s) { return s.aperture.grating.softness; },
		[](lens_settings& s, float v) { s.aperture.grating.softness = v; }},

	{"Scratches",   "Strength",              lens_field::kind::Real,     0.0f,   1.0f, 2,
		[](const lens_settings& s) { return s.aperture.scratches.strength; },
		[](lens_settings& s, float v) { s.aperture.scratches.strength = v; }},
	{nullptr,       "Density",               lens_field::kind::Real,     0.0f,   1.0f, 2,
		[](const lens_settings& s) { return s.aperture.scratches.density; },
		[](lens_settings& s, float v) { s.aperture.scratches.density = v; }},
	{nullptr,       "Length",                lens_field::kind::Real,     0.0f,   1.0f, 2,
		[](const lens_settings& s) { return s.aperture.scratches.length; },
		[](lens_settings& s, float v) { s.aperture.scratches.length = v; }},
	{nullptr,       "Width",                 lens_field::kind::Real,     0.0f,   1.0f, 2,
		[](const lens_settings& s) { return s.aperture.scratches.width; },
		[](lens_settings& s, float v) { s.aperture.scratches.width = v; }},
	{nullptr,       "Rotation (deg)",        lens_field::kind::Real,     0.0f, 180.0f, 1,
		[](const lens_settings& s) { return s.aperture.scratches.rotation; },
		[](lens_settings& s, float v) { s.aperture.scratches.rotation = v; }},
	{nullptr,       "Rotation variation",    lens_field::kind::Real,     0.0f,   1.0f, 2,
		[](const lens_settings& s) { return s.aperture.scratches.rotation_variation; },
		[](lens_settings& s, float v) { s.aperture.scratches.rotation_variation = v; }},
	{nullptr,       "Softness",              lens_field::kind::Real,     0.0f,   0.5f, 4,
		[](const lens_settings& s) { return s.aperture.scratches.softness; },
		[](lens_settings& s, float v) { s.aperture.scratches.softness = v; }},

	{"Dust",        "Strength",              lens_field::kind::Real,     0.0f,   1.0f, 2,
		[](const lens_settings& s) { return s.aperture.dust.strength; },
		[](lens_settings& s, float v) { s.aperture.dust.strength = v; }},
	{nullptr,       "Density",               lens_field::kind::Real,     0.0f,   1.0f, 2,
		[](const lens_settings& s) { return s.aperture.dust.density; },
		[](lens_settings& s, float v) { s.aperture.dust.density = v; }},
	{nullptr,       "Radius",                lens_field::kind::Real,     0.0f,   1.0f, 2,
		[](const lens_settings& s) { return s.aperture.dust.radius; },
		[](lens_settings& s, float v) { s.aperture.dust.radius = v; }},
	{nullptr,       "Softness",              lens_field::kind::Real,     0.0f,   0.5f, 4,
		[](const lens_settings& s) { return s.aperture.dust.softness; },
		[](lens_settings& s, float v) { s.aperture.dust.softness = v; }},

	{"Anamorphic",  "Squeeze",               lens_field::kind::Real,     1.0f,   3.0f, 2,
		[](const lens_settings& s) { return s.anamorphic.squeeze; },
		[](lens_settings& s, float v) { s.anamorphic.squeeze = v; }},
	{nullptr,       "Streak strength",       lens_field::kind::Real,     0.0f,   2.0f, 2,
		[](const lens_settings& s) { return s.anamorphic.streak.strength; },
		[](lens_settings& s, float v) { s.anamorphic.streak.strength = v; }},
	{nullptr,       "Streak length",         lens_field::kind::Real,     0.0f,   4.0f, 2,
		[](const lens_settings& s) { return s.anamorphic.streak.length; },
		[](lens_settings& s, float v) { s.anamorphic.streak.length = v; }},
	{nullptr,       "Streak thickness",      lens_field::kind::Real,     0.0f,   0.2f, 4,
		[](const lens_settings& s) { return s.anamorphic.streak.thickness; },
		[](lens_settings& s, float v) { s.anamorphic.streak.thickness = v; }},
	{nullptr,       "Streak tint (red)",     lens_field::kind::Real,     0.0f,   1.0f, 2,
		[](const lens_settings& s) { return s.anamorphic.streak.tint[0]; },
		[](lens_settings& s, float v) { s.anamorphic.streak.tint[0] = v; }},
	{nullptr,       "Streak tint (green)",   lens_field::kind::Real,     0.0f,   1.0f, 2,
		[](const lens_settings& s) { return s.anamorphic.streak.tint[1]; },
		[](lens_settings& s, float v) { s.anamorphic.streak.tint[1] = v; }},
	{nullptr,       "Streak tint (blue)",    lens_field::kind::Real,     0.0f,   1.0f, 2,
		[](const lens_settings& s) { return s.anamorphic.streak.tint[2]; },
		[](lens_settings& s, float v) { s.anamorphic.streak.tint[2] = v; }},

	// The knobs below cost nothing to change, unlike everything above the
	// anamorphic section: none of them touches the iris mask or its transform.
	{"Strength",    "Lens intensity",        lens_field::kind::Real,     0.0f,  10.0f, 3,
		[](const lens_settings& s) { return s.intensity; },
		[](lens_settings& s, float v) { s.intensity = v; }},
	{nullptr,       "Ghost brightness",      lens_field::kind::Real,     0.0f, 500.0f, 1,
		[](const lens_settings& s) { return s.ghost_brightness; },
		[](lens_settings& s, float v) { s.ghost_brightness = v; }},
	{nullptr,       "Starburst brightness",  lens_field::kind::Real,     0.0f,  10.0f, 2,
		[](const lens_settings& s) { return s.starburst_brightness; },
		[](lens_settings& s, float v) { s.starburst_brightness = v; }},
	{nullptr,       "Draw starburst",        lens_field::kind::Boolean,  0.0f,   1.0f, 0,
		[](const lens_settings& s) { return s.starburst ? 1.0f : 0.0f; },
		[](lens_settings& s, float v) { s.starburst = (v != 0.0f); }},
	{nullptr,       "Starburst scale",       lens_field::kind::Real,     0.0f,   4.0f, 2,
		[](const lens_settings& s) { return s.starburst_scale; },
		[](lens_settings& s, float v) { s.starburst_scale = v; }},
	{nullptr,       "Max ghosts",            lens_field::kind::Integer,  0.0f,
		static_cast<float>(graphics::MAX_LENS_FLARE_GHOSTS), 0,
		[](const lens_settings& s) { return static_cast<float>(s.max_ghosts); },
		[](lens_settings& s, float v) { s.max_ghosts = static_cast<int>(std::lround(v)); }},
};
// clang-format on

constexpr int Num_lens_fields = static_cast<int>(sizeof(Lens_fields) / sizeof(Lens_fields[0]));

} // namespace

LensApertureDialog::LensApertureDialog(QWidget* parent, BackgroundEditorDialogModel* model)
	: QDialog(parent), ui(new Ui::LensApertureDialog()), _model(model)
{
	ui->setupUi(this);

	buildFields();

	connect(ui->resetButton, &QPushButton::clicked, this, [this]() {
		_model->resetLensSettings();
		updateUi();
	});

	updateUi();
}

LensApertureDialog::~LensApertureDialog() = default;

// Build one control per table row, grouping consecutive rows under whichever
// section header last named one.
void LensApertureDialog::buildFields()
{
	auto* outer = ui->scrollAreaContents->layout();
	QFormLayout* form = nullptr;

	_rows.resize(Num_lens_fields);

	for (int i = 0; i < Num_lens_fields; i++) {
		const lens_field& f = Lens_fields[i];
		row& r = _rows[i];

		if (f.section != nullptr) {
			auto* group = new QGroupBox(QString::fromUtf8(f.section), ui->scrollAreaContents);
			form = new QFormLayout(group);
			outer->addWidget(group);
		}
		Assertion(form != nullptr, "The first lens field must open a section!");

		if (f.type == lens_field::kind::Boolean) {
			r.check = new QCheckBox();
			connect(r.check, &QCheckBox::toggled, this, [this]() { applyFromControls(); });
			form->addRow(QString::fromUtf8(f.label), r.check);
			continue;
		}

		const float scale = field_scale(f);
		r.slider = new QSlider(Qt::Horizontal);
		r.slider->setMinimum(static_cast<int>(std::lround(f.min * scale)));
		r.slider->setMaximum(static_cast<int>(std::lround(f.max * scale)));
		r.value = new QLabel();
		// Wide enough that the row doesn't jump about as digits come and go
		r.value->setMinimumWidth(60);
		r.value->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

		connect(r.slider, &QSlider::valueChanged, this, [this]() { applyFromControls(); });

		auto* rowLayout = new QHBoxLayout();
		rowLayout->addWidget(r.slider);
		rowLayout->addWidget(r.value);
		form->addRow(QString::fromUtf8(f.label), rowLayout);
	}

	outer->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));
}

// The value label for a row, formatted at that row's own precision.
void LensApertureDialog::showValue(int index, float value)
{
	const lens_field& f = Lens_fields[index];
	if (_rows[index].value == nullptr) {
		return;
	}
	_rows[index].value->setText(QString::number(value, 'f', f.decimals));
}

void LensApertureDialog::updateUi()
{
	util::SignalBlockers blockers(this);

	ui->noLensWarningLabel->setVisible(!BackgroundEditorDialogModel::getLensMounted());

	const graphics::lens_settings settings = _model->getLensSettings();

	for (int i = 0; i < Num_lens_fields; i++) {
		const lens_field& f = Lens_fields[i];
		const float value = f.get(settings);

		if (f.type == lens_field::kind::Boolean) {
			_rows[i].check->setChecked(value != 0.0f);
			continue;
		}

		_rows[i].slider->setValue(static_cast<int>(std::lround(value * field_scale(f))));
		showValue(i, value);
	}
}

void LensApertureDialog::applyFromControls()
{
	graphics::lens_settings settings;

	for (int i = 0; i < Num_lens_fields; i++) {
		const lens_field& f = Lens_fields[i];

		if (f.type == lens_field::kind::Boolean) {
			f.set(settings, _rows[i].check->isChecked() ? 1.0f : 0.0f);
			continue;
		}

		const float value = static_cast<float>(_rows[i].slider->value()) / field_scale(f);
		f.set(settings, value);
		showValue(i, value);
	}

	_model->setLensSettings(settings);
}

} // namespace fso::fred::dialogs
