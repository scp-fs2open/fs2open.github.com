#include "TransformToolBar.h"

#include <QApplication>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFocusEvent>
#include <QKeyEvent>
#include <QLabel>
#include <QSettings>
#include <QSignalBlocker>
#include <QToolBar>
#include <QToolButton>

#include <globalincs/linklist.h>
#include <iff_defs/iff_defs.h>
#include <math/vecmat.h>
#include <missioneditor/common.h>
#include <object/object.h>
#include <ship/ship.h>

#include "mission/Editor.h"
#include "mission/EditorViewport.h"
#include "mission/object.h"
#include "ui/Theme.h"

#include <cmath>
#include <functional>

namespace fso::fred {

namespace {

// Number of decimals in the boxes
constexpr int BOX_DECIMALS = 2;

constexpr double POSITION_LIMIT = 99999.99;
constexpr double ANGLE_LIMIT = 360.0;

const char* const SETTING_LOCAL_MOVE = "FredView/transformLocalMove";
const char* const SETTING_LOCAL_ROTATE = "FredView/transformLocalRotate";

bool isShipType(int type)
{
	return type == OBJ_SHIP || type == OBJ_START;
}

// Selects the item with the given data value, without a signal
void selectByData(QComboBox* combo, int value)
{
	QSignalBlocker blocker(combo);
	const int index = combo->findData(value);
	if (index >= 0) {
		combo->setCurrentIndex(index);
	}
}

// A spin box that reports each step that changes its value: the mouse wheel, the arrow buttons, and the Up, Down,
// Page Up, and Page Down keys. A step does not interpret typed text, so the toolbar can apply it at once.
class StepSpinBox : public QDoubleSpinBox {
  public:
	using QDoubleSpinBox::QDoubleSpinBox;

	std::function<void()> onStep;

	void stepBy(int steps) override
	{
		const double before = value();
		QDoubleSpinBox::stepBy(steps);
		if (value() != before && onStep) {
			onStep();
		}
	}
};

} // namespace

TransformToolBar::TransformToolBar(QToolBar* toolbar, Editor* editor, EditorViewport* viewport, QWidget* viewportWidget)
	: QObject(toolbar), _toolbar(toolbar), _editor(editor), _viewport(viewport), _viewportWidget(viewportWidget)
{
	QSettings settings;
	_localMove = settings.value(SETTING_LOCAL_MOVE, false).toBool();
	_localRotate = settings.value(SETTING_LOCAL_ROTATE, false).toBool();

	addWidgets();
}

void TransformToolBar::saveSettings() const
{
	QSettings settings;
	settings.setValue(SETTING_LOCAL_MOVE, _localMove);
	settings.setValue(SETTING_LOCAL_ROTATE, _localRotate);
}

void TransformToolBar::markLayersChanged()
{
	_layersChanged = true;
}

bool TransformToolBar::isTransformBox(const QObject* object) const
{
	return boxAxis(object) >= 0;
}

int TransformToolBar::boxAxis(const QObject* object) const
{
	if (object == nullptr) {
		return -1;
	}
	for (int axis = 0; axis < 3; ++axis) {
		if (object == _boxes[axis]) {
			return axis;
		}
	}
	return -1;
}

void TransformToolBar::addFixedSpacer(int width)
{
	auto* spacer = new QWidget(_toolbar);
	spacer->setFixedWidth(width);
	_toolbar->addWidget(spacer);
}

void TransformToolBar::addWidgets()
{
	_toolbar->setContextMenuPolicy(Qt::PreventContextMenu);
	_toolbar->setVisible(true);

	auto addLabel = [this](const QString& text) {
		auto* label = new QLabel(text, _toolbar);
		label->setContentsMargins(0, 0, 4, 0);
		_toolbar->addWidget(label);
		return label;
	};

	// ---- Left section: camera move and rotation speed selectors ---------------
	addFixedSpacer(8);

	addLabel(tr("Camera Move:"));
	_moveSpeedCombo = new QComboBox(_toolbar);
	_moveSpeedCombo->setFixedWidth(72);
	_moveSpeedCombo->setToolTip(tr("Camera movement speed (mirrors the Speed > Movement menu)"));
	for (int v : {1, 2, 3, 5, 8, 10, 50, 100})
		_moveSpeedCombo->addItem(tr("x%1").arg(v), v);
	_toolbar->addWidget(_moveSpeedCombo);
	connect(_moveSpeedCombo, QOverload<int>::of(&QComboBox::activated), this, [this](int idx) {
		_viewport->camera.setPhysicsSpeed(_moveSpeedCombo->itemData(idx).toInt());
	});

	addFixedSpacer(8);

	addLabel(tr("Camera Rot:"));
	_rotSpeedCombo = new QComboBox(_toolbar);
	_rotSpeedCombo->setFixedWidth(72);
	_rotSpeedCombo->setToolTip(tr("Camera rotation speed (mirrors the Speed > Rotation menu)"));
	// Labels match the existing menu (physics_rot / ~2 ≈ displayed multiplier)
	for (auto [label, val] : std::initializer_list<std::pair<const char*, int>>{
			 {"x1", 2}, {"x5", 10}, {"x12", 25}, {"x25", 50}, {"x50", 100}})
		_rotSpeedCombo->addItem(tr(label), val);
	_toolbar->addWidget(_rotSpeedCombo);
	connect(_rotSpeedCombo, QOverload<int>::of(&QComboBox::activated), this, [this](int idx) {
		_viewport->camera.setPhysicsRot(_rotSpeedCombo->itemData(idx).toInt());
	});

	addFixedSpacer(8);

	// ---- Single expanding spacer pushes everything to the right side -------
	auto* leftSpacer = new QWidget(_toolbar);
	leftSpacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	_toolbar->addWidget(leftSpacer);

	// ---- IFF / Team selector -----------------------------------------------
	// The IFF items are added in update(), after the tables fill Iff_info.
	addLabel(tr("IFF:"));
	_iffCombo = new QComboBox(_toolbar);
	_iffCombo->setFixedWidth(130);
	_iffCombo->setToolTip(tr("IFF of the selected ship(s). Changes apply to all marked ships."));
	_toolbar->addWidget(_iffCombo);
	connect(_iffCombo, QOverload<int>::of(&QComboBox::activated), this, [this](int idx) {
		if (idx < 0) return;
		for (object* p = GET_FIRST(&obj_used_list); p != END_OF_LIST(&obj_used_list); p = GET_NEXT(p)) {
			if (p->flags[Object::Object_Flags::Marked] && isShipType(p->type))
				Ships[p->instance].team = idx;
		}
		_editor->missionChanged();
	});

	// QToolBar aligns the tops of its widgets, and the widget types have different heights. Give the Local button
	// and the boxes the height of the combo boxes, so that the bottom edges also align.
	const int rowHeight = _iffCombo->sizeHint().height();

	addFixedSpacer(8);

	// ---- Local-axes toggle ----
	_localButton = new QToolButton(_toolbar);
	_localButton->setCheckable(true);
	_localButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
	_localButton->setFixedSize(28, rowHeight);
	_localButton->setToolTip(tr("Local mode: in multi-selection, apply position/orientation as a delta to each object rather than setting all to the same absolute value. (X)"));
	// FRED2 bound "Rotate Locally" to the X key; route it through the button so the per-mode local memory stays in sync.
	_localButton->setShortcut(QKeySequence(Qt::Key_X));
	bindThemeIcon(_localButton, QStringLiteral("rotlocal"));
	_toolbar->addWidget(_localButton);
	connect(_localButton, &QToolButton::toggled, this, [this](bool checked) {
		// Local ON  = each object rotates/moves individually -> Group_rotate = false
		// Local OFF = formation (group orbits leader)        -> Group_rotate = true
		_viewport->Group_rotate = !checked;
		if (_viewport->Editing_mode == CursorMode::Moving)
			_localMove = checked;
		else if (_viewport->Editing_mode == CursorMode::Rotating)
			_localRotate = checked;
	});

	addFixedSpacer(8);

	// ---- Spin boxes (position or orientation) ------------------------------
	_modeLabel = addLabel(tr("Pos"));
	_modeLabel->setMinimumWidth(24);

	for (auto& box : _boxes) {
		const int axis = static_cast<int>(&box - _boxes.data());
		auto*& label = _axisLabels[axis];
		label = new QLabel(_toolbar);
		label->setContentsMargins(4, 0, 2, 0);
		_toolbar->addWidget(label);

		auto* stepBox = new StepSpinBox(_toolbar);
		stepBox->onStep = [this, axis]() { applyStep(axis); };
		box = stepBox;
		box->setDecimals(BOX_DECIMALS);
		box->setRange(-POSITION_LIMIT, POSITION_LIMIT);
		box->setFixedWidth(90);
		box->setFixedHeight(rowHeight);
		box->setKeyboardTracking(false);
		_toolbar->addWidget(box);
		box->installEventFilter(this);
	}
	_axisLabels[0]->setText(tr("X"));
	_axisLabels[1]->setText(tr("Y"));
	_axisLabels[2]->setText(tr("Z"));

	// ---- Object radius (read-only) -----------------------------------------
	addFixedSpacer(8);

	addLabel(tr("Radius:"));
	_radiusLabel = new QLabel(tr("--"), _toolbar);
	_radiusLabel->setFixedWidth(64);
	_radiusLabel->setToolTip(tr("Bounding radius of the selected object"));
	_toolbar->addWidget(_radiusLabel);

	addFixedSpacer(8);

	// ---- Layer selector ----------------------------------------------------
	addLabel(tr("Layer:"));
	_layerCombo = new QComboBox(_toolbar);
	_layerCombo->setFixedWidth(130);
	_layerCombo->setToolTip(tr("Layer of the selected object(s). Choosing a layer moves all marked objects to it."));
	_toolbar->addWidget(_layerCombo);
	connect(_layerCombo, QOverload<int>::of(&QComboBox::activated), this, [this](int idx) {
		if (idx < 0) return;
		SCP_string layerName = _layerCombo->itemText(idx).toUtf8().constData();
		_viewport->moveMarkedObjectsToLayer(layerName, nullptr);
		_editor->missionChanged();
	});

	addFixedSpacer(12);
}

void TransformToolBar::update()
{
	const int  curObj     = _editor->currentObject;
	const int  numMarked  = _editor->getNumMarked();
	const bool valid      = query_valid_object(curObj);
	const bool rotateMode = _viewport->Editing_mode == CursorMode::Rotating;
	const bool selectMode = _viewport->Editing_mode == CursorMode::Selecting;
	const bool isShip     = valid && isShipType(Objects[curObj].type);
	const bool editable   = valid && !selectMode;

	updateBoxes(valid, rotateMode, editable);
	updateLocalButton(editable);
	updateCameraSpeeds();
	updateIff(valid, isShip, numMarked);
	updateRadius(valid, numMarked);
	updateLayer(valid, numMarked);
}

void TransformToolBar::updateBoxes(bool valid, bool rotateMode, bool editable)
{
	// A change of the current object or of the mode makes the typed value incorrect. Give the focus back to the
	// viewport before the code below changes the ranges, so that the box shows the new value.
	if (isTransformBox(QApplication::focusWidget()) && !editIsCurrent()) {
		_viewportWidget->setFocus(Qt::OtherFocusReason);
	}

	_modeLabel->setText(rotateMode ? tr("Ori") : tr("Pos"));
	const QString axisNames[3] = {rotateMode ? tr("H") : tr("X"), rotateMode ? tr("P") : tr("Y"),
		rotateMode ? tr("B") : tr("Z")};
	const double limit = rotateMode ? ANGLE_LIMIT : POSITION_LIMIT;

	float values[3] = {};
	if (valid) {
		if (rotateMode) {
			angles ang{};
			vm_extract_angles_matrix(&ang, &Objects[_editor->currentObject].orient);
			values[0] = fl_degrees(ang.h);
			values[1] = fl_degrees(ang.p);
			values[2] = fl_degrees(ang.b);
		} else {
			const vec3d& pos = Objects[_editor->currentObject].pos;
			values[0] = pos.xyz.x;
			values[1] = pos.xyz.y;
			values[2] = pos.xyz.z;
		}
	}

	for (int axis = 0; axis < 3; ++axis) {
		auto* box = _boxes[axis];
		_axisLabels[axis]->setText(axisNames[axis]);
		box->setRange(-limit, limit);
		box->setEnabled(editable);
		// Do not change the value while the user types in the box
		if (valid && !box->hasFocus()) {
			box->setValue(values[axis]);
		}
	}
}

void TransformToolBar::updateLocalButton(bool editable)
{
	// When the cursor mode changes, restore the last Local setting for that mode
	const int mode = static_cast<int>(_viewport->Editing_mode);
	if (mode != _cachedMode) {
		_cachedMode = mode;
		if (_viewport->Editing_mode == CursorMode::Moving)
			_viewport->Group_rotate = !_localMove;
		else if (_viewport->Editing_mode == CursorMode::Rotating)
			_viewport->Group_rotate = !_localRotate;
		// Select mode: leave Group_rotate unchanged
	}

	// Local ON = individual axes = Group_rotate false, so the button shows !Group_rotate
	{
		QSignalBlocker blocker(_localButton);
		_localButton->setChecked(!_viewport->Group_rotate);
	}
	_localButton->setEnabled(editable);
}

void TransformToolBar::updateCameraSpeeds()
{
	// Keyboard shortcuts and the Speed menu also change the speeds
	selectByData(_moveSpeedCombo, _viewport->camera.getPhysicsSpeed());
	selectByData(_rotSpeedCombo, _viewport->camera.getPhysicsRot());
}

void TransformToolBar::updateIff(bool valid, bool isShip, int numMarked)
{
	if (!_iffPopulated && !Iff_info.empty()) {
		_iffPopulated = true;
		QSignalBlocker blocker(_iffCombo);
		for (const auto& iff : Iff_info)
			_iffCombo->addItem(QString::fromUtf8(iff.iff_name));
	}

	// Find the common IFF of the marked ships. -1 means mixed or no ship.
	int iffIndex = -1;
	bool anyShip = isShip;
	if (numMarked > 1) {
		anyShip = false;
		int firstIff = -2;
		bool allSame = true;
		for (object* p = GET_FIRST(&obj_used_list); p != END_OF_LIST(&obj_used_list); p = GET_NEXT(p)) {
			if (!p->flags[Object::Object_Flags::Marked] || !isShipType(p->type)) continue;
			anyShip = true;
			const int team = Ships[p->instance].team;
			if (firstIff == -2) { firstIff = team; }
			else if (team != firstIff) { allSame = false; }
		}
		if (anyShip && allSame && firstIff >= 0) iffIndex = firstIff;
	} else if (valid && isShip) {
		iffIndex = Ships[Objects[_editor->currentObject].instance].team;
	}

	_iffCombo->setEnabled(anyShip);
	QSignalBlocker blocker(_iffCombo);
	_iffCombo->setCurrentIndex(iffIndex);  // -1 = blank for mixed
}

void TransformToolBar::updateRadius(bool valid, int numMarked)
{
	// The radius of the object for one object, and the bounding radius of the selection for more
	float radius = 0.0f;
	bool known = false;
	if (valid && numMarked <= 1) {
		radius = Objects[_editor->currentObject].radius;
		known = true;
	} else if (numMarked > 1 && obj_used_list.next != nullptr) {
		vec3d center;
		known = get_object_bounds(true, &center, &radius);
	}
	_radiusLabel->setText(known ? QString::number(static_cast<double>(radius), 'f', 1) : tr("--"));
}

void TransformToolBar::updateLayer(bool valid, int numMarked)
{
	if (_layersChanged) {
		QSignalBlocker blocker(_layerCombo);
		_layerCombo->clear();
		for (const auto& name : _viewport->getLayerNames())
			_layerCombo->addItem(QString::fromUtf8(name.c_str()));
		_layersChanged = false;
	}

	auto layerIndexOf = [this](int objnum) {
		return _layerCombo->findText(QString::fromUtf8(_viewport->getObjectLayerName(objnum).c_str()));
	};

	const bool anySelected = valid || numMarked > 0;
	_layerCombo->setEnabled(anySelected);

	// Find the common layer. -1 = blank for mixed layers.
	int layerIdx = -1;
	if (numMarked <= 1 && valid) {
		layerIdx = layerIndexOf(_editor->currentObject);
	} else if (numMarked > 1) {
		int firstLayer = -2;
		bool allSame = true;
		for (object* p = GET_FIRST(&obj_used_list); p != END_OF_LIST(&obj_used_list); p = GET_NEXT(p)) {
			if (!p->flags[Object::Object_Flags::Marked]) continue;
			const int idx = layerIndexOf(OBJ_INDEX(p));
			if (firstLayer == -2) { firstLayer = idx; }
			else if (idx != firstLayer) { allSame = false; break; }
		}
		if (allSame && firstLayer >= 0) layerIdx = firstLayer;
	}

	QSignalBlocker blocker(_layerCombo);
	_layerCombo->setCurrentIndex(layerIdx);
}

bool TransformToolBar::eventFilter(QObject* watched, QEvent* event)
{
	const int axis = boxAxis(watched);
	if (axis < 0) {
		return QObject::eventFilter(watched, event);
	}
	auto* box = _boxes[axis];

	switch (event->type()) {
	case QEvent::FocusIn: {
		// The focus also comes back after a popup or a window change. The edit then continues, so keep the record.
		const auto reason = static_cast<QFocusEvent*>(event)->reason();
		if (reason != Qt::PopupFocusReason && reason != Qt::ActiveWindowFocusReason) {
			_edit = {_editor->currentObject, _viewport->Editing_mode, box->value()};
		}
		break;
	}
	case QEvent::ShortcutOverride: {
		// Do not let a menu shortcut or a control binding take Enter or Escape from the box
		const int key = static_cast<QKeyEvent*>(event)->key();
		if (key == Qt::Key_Return || key == Qt::Key_Enter || key == Qt::Key_Escape) {
			event->accept();
			return true;
		}
		break;
	}
	case QEvent::KeyPress: {
		// Enter applies the value. Escape discards it. Both give the focus back to the viewport, so that the
		// keyboard controls work again. All other focus changes discard the value.
		const int key = static_cast<QKeyEvent*>(event)->key();
		if (key == Qt::Key_Return || key == Qt::Key_Enter) {
			box->interpretText();
			applyValue(axis, box->value());
		} else if (key != Qt::Key_Escape) {
			break;
		}
		_viewportWidget->setFocus(Qt::OtherFocusReason);
		return true;
	}
	default:
		break;
	}
	return QObject::eventFilter(watched, event);
}

bool TransformToolBar::editIsCurrent() const
{
	return _editor->currentObject == _edit.object && _viewport->Editing_mode == _edit.mode;
}

void TransformToolBar::applyStep(int axis)
{
	// The box shows the value of the current object: the update gives a box with the focus back to the viewport
	// when the object or the mode changes. Thus the step is for the current object, also if the box did not have
	// the focus before the step.
	_edit.object = _editor->currentObject;
	_edit.mode = _viewport->Editing_mode;
	applyValue(axis, _boxes[axis]->value());

	// A later Enter must not apply the same value again
	_edit.startValue = _boxes[axis]->value();
}

void TransformToolBar::applyValue(int axis, double value)
{
	// The selection or the mode can change while the box has the focus. Then the value is not for this object.
	if (!editIsCurrent()) return;

	const int curObj = _editor->currentObject;
	if (!query_valid_object(curObj)) return;

	// Change only the axis of the edited box. The other boxes show rounded values. If the code also wrote them,
	// the object would move on the axes that the user did not change.
	const bool rotateMode = _viewport->Editing_mode == CursorMode::Rotating;
	const bool isMulti    = _editor->getNumMarked() > 1;
	// Local ON (= !Group_rotate) with more than one object: add the change of the current object to
	// each marked object. Otherwise, set the value on each marked object.
	const bool relative   = isMulti && !_viewport->Group_rotate;

	// More than one object with Local OFF: Enter sets the value on all the marked objects, also if it did not change
	const bool alignAll   = isMulti && !relative;

	// If the user did not change the value, do not write the rounded value back to the object
	const double scale     = std::pow(10.0, BOX_DECIMALS);
	const bool   unchanged = std::abs(value - _edit.startValue) < 0.5 / scale;
	if (unchanged && !alignAll) return;

	auto angleOf = [axis](angles& a) -> float& { return (axis == 0) ? a.h : (axis == 1) ? a.p : a.b; };

	float current;
	if (rotateMode) {
		angles ang{};
		vm_extract_angles_matrix(&ang, &Objects[curObj].orient);
		current = fl_degrees(angleOf(ang));
	} else {
		current = Objects[curObj].pos.a1d[axis];
	}

	const auto newValue = static_cast<float>(value);
	const float delta   = newValue - current;

	auto apply = [&](object* p) {
		if (rotateMode) {
			angles a{};
			vm_extract_angles_matrix(&a, &p->orient);
			angleOf(a) = relative ? angleOf(a) + fl_radians(delta) : fl_radians(newValue);
			vm_angles_2_matrix(&p->orient, &a);
		} else {
			p->pos.a1d[axis] = relative ? p->pos.a1d[axis] + delta : newValue;
		}
	};

	// Change all the objects first, then move the docked partners
	if (isMulti) {
		for (object* p = GET_FIRST(&obj_used_list); p != END_OF_LIST(&obj_used_list); p = GET_NEXT(p)) {
			if (p->flags[Object::Object_Flags::Marked])
				apply(p);
		}
		marked_objects_moved();
	} else {
		apply(&Objects[curObj]);
		object_moved(&Objects[curObj]);
	}

	_editor->missionChanged();
}

} // namespace fso::fred
