#pragma once

#include <QKeyEvent>
#include <QKeySequence>
#include <QString>

#include <map>
#include <set>
#include <unordered_map>
#include <vector>

namespace fso::fred {

enum class ControlAction {
	MoveLeft,
	MoveRight,
	MoveForward,
	MoveBackward,
	MoveUp,
	MoveDown,
	YawLeft,
	YawRight,
	PitchUp,
	PitchDown,
	ToggleSelectionLock,
};

struct ControlBindingDefinition {
	ControlAction action;
	QString id;
	QString label;
	QKeySequence defaultKey;
};

class ControlBindings {
 public:
	static ControlBindings& instance();

	const std::vector<ControlBindingDefinition>& definitions() const;
	QKeySequence keyFor(ControlAction action) const;
	void setKey(ControlAction action, const QKeySequence& sequence);
	void resetToDefaults();
	void load();
	void save() const;

	bool handleKeyPress(QKeyEvent* event);
	bool handleKeyRelease(QKeyEvent* event);
	bool matches(QKeyEvent* event) const;

	bool isPressed(ControlAction action) const;
	bool takeTriggered(ControlAction action);

 private:
	ControlBindings();

	static int normalizedCode(const QKeySequence& sequence);
	static int normalizedCode(const QKeyEvent* event);
	void rebuildReverseMap();

	std::vector<ControlBindingDefinition> _definitions;
	std::map<ControlAction, QKeySequence> _bindings;
	std::unordered_map<int, std::vector<ControlAction>> _actionByCode;
	std::set<ControlAction> _pressedActions;
	std::set<ControlAction> _triggeredActions;
};

} // namespace fso::fred
