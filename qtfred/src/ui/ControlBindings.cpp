#include "ControlBindings.h"

#include <QSettings>

namespace fso::fred {
namespace {
constexpr auto SETTINGS_GROUP = "ControlBindings";
}

ControlBindings& ControlBindings::instance() {
	static ControlBindings bindings;
	return bindings;
}

ControlBindings::ControlBindings() :
	_definitions({
			{ControlAction::MoveLeft, "move_left", "Move Left", QKeySequence(Qt::Key_1 | Qt::KeypadModifier)},
			{ControlAction::MoveRight, "move_right", "Move Right", QKeySequence(Qt::Key_3 | Qt::KeypadModifier)},
			{ControlAction::MoveForward, "move_forward", "Move Forward", QKeySequence(Qt::Key_A)},
			{ControlAction::MoveBackward, "move_backward", "Move Backward", QKeySequence(Qt::Key_Z)},
			{ControlAction::MoveUp, "move_up", "Move Up", QKeySequence(Qt::Key_Minus | Qt::KeypadModifier)},
			{ControlAction::MoveDown, "move_down", "Move Down", QKeySequence(Qt::Key_Plus | Qt::KeypadModifier)},
			{ControlAction::YawLeft, "yaw_left", "Yaw Left", QKeySequence(Qt::Key_4 | Qt::KeypadModifier)},
			{ControlAction::YawRight, "yaw_right", "Yaw Right", QKeySequence(Qt::Key_6 | Qt::KeypadModifier)},
			{ControlAction::PitchUp, "pitch_up", "Pitch Up", QKeySequence(Qt::Key_2 | Qt::KeypadModifier)},
			{ControlAction::PitchDown, "pitch_down", "Pitch Down", QKeySequence(Qt::Key_8 | Qt::KeypadModifier)},
			{ControlAction::ToggleSelectionLock, "toggle_selection_lock", "Toggle Selection Lock", QKeySequence(Qt::Key_Space)},
		}) {
	resetToDefaults();
	load();
}

const std::vector<ControlBindingDefinition>& ControlBindings::definitions() const {
	return _definitions;
}

QKeySequence ControlBindings::keyFor(ControlAction action) const {
	auto it = _bindings.find(action);
	return it != _bindings.end() ? it->second : QKeySequence();
}

void ControlBindings::setKey(ControlAction action, const QKeySequence& sequence) {
	_bindings[action] = sequence;
	rebuildReverseMap();
}

void ControlBindings::resetToDefaults() {
	_bindings.clear();
	for (const auto& def : _definitions) {
		_bindings.emplace(def.action, def.defaultKey);
	}
	rebuildReverseMap();
}

void ControlBindings::load() {
	QSettings settings;
	settings.beginGroup(SETTINGS_GROUP);
	for (const auto& def : _definitions) {
		auto value = settings.value(def.id, def.defaultKey.toString(QKeySequence::PortableText)).toString();
		_bindings[def.action] = QKeySequence::fromString(value, QKeySequence::PortableText);
	}
	settings.endGroup();
	rebuildReverseMap();
}

void ControlBindings::save() const {
	QSettings settings;
	settings.beginGroup(SETTINGS_GROUP);
	for (const auto& def : _definitions) {
		auto key = _bindings.find(def.action);
		if (key != _bindings.end()) {
			settings.setValue(def.id, key->second.toString(QKeySequence::PortableText));
		}
	}
	settings.endGroup();
}

bool ControlBindings::handleKeyPress(QKeyEvent* event) {
	if (event->isAutoRepeat()) {
		return false;
	}
	auto code = normalizedCode(event);
	auto it = _actionByCode.find(code);
	if (it == _actionByCode.end()) {
		return false;
	}
	for (auto action : it->second) {
		if (_pressedActions.insert(action).second) {
			_triggeredActions.insert(action);
		}
	}
	return true;
}

bool ControlBindings::handleKeyRelease(QKeyEvent* event) {
	if (event->isAutoRepeat()) {
		return false;
	}
	auto code = normalizedCode(event);
	auto it = _actionByCode.find(code);
	if (it == _actionByCode.end()) {
		return false;
	}
	for (auto action : it->second) {
		_pressedActions.erase(action);
	}
	return true;
}

bool ControlBindings::matches(QKeyEvent* event) const {
	auto code = normalizedCode(event);
	return _actionByCode.find(code) != _actionByCode.end();
}

bool ControlBindings::isPressed(ControlAction action) const {
	return _pressedActions.find(action) != _pressedActions.end();
}

bool ControlBindings::takeTriggered(ControlAction action) {
	auto it = _triggeredActions.find(action);
	if (it == _triggeredActions.end()) {
		return false;
	}
	_triggeredActions.erase(it);
	return true;
}

int ControlBindings::normalizedCode(const QKeySequence& sequence) {
	if (sequence.isEmpty()) {
		return 0;
	}
	auto combined = sequence[0];
	const auto key = combined & ~Qt::KeyboardModifierMask;
	const auto mods = combined & Qt::KeyboardModifierMask;
	constexpr auto relevant_mods = Qt::ShiftModifier | Qt::ControlModifier | Qt::AltModifier | Qt::MetaModifier | Qt::KeypadModifier;
	return key | (mods & relevant_mods);
}

int ControlBindings::normalizedCode(const QKeyEvent* event) {
	const auto key = static_cast<int>(event->key());
	const auto mods = static_cast<int>(event->modifiers() & (Qt::ShiftModifier | Qt::ControlModifier | Qt::AltModifier | Qt::MetaModifier | Qt::KeypadModifier));
	return key | mods;
}

void ControlBindings::rebuildReverseMap() {
	_actionByCode.clear();
	for (const auto& entry : _bindings) {
		auto code = normalizedCode(entry.second);
		if (code != 0) {
			_actionByCode[code].push_back(entry.first);
		}
	}
}

} // namespace fso::fred
