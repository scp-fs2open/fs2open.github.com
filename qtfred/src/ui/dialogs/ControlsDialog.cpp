#include "ControlsDialog.h"

#include <QDialogButtonBox>
#include <QFormLayout>
#include <QKeyEvent>
#include <QKeySequenceEdit>
#include <QPushButton>
#include <QVBoxLayout>

namespace fso::fred::dialogs {
namespace {

class ControlKeySequenceEdit : public QKeySequenceEdit {
 public:
	explicit ControlKeySequenceEdit(const QKeySequence& sequence, QWidget* parent) : QKeySequenceEdit(sequence, parent) {}

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

ControlsDialog::ControlsDialog(QWidget* parent) : QDialog(parent) {
	setWindowTitle(tr("Controls"));
	resize(500, 400);

	auto* layout = new QVBoxLayout(this);
	auto* form = new QFormLayout();
	layout->addLayout(form);

	auto& bindings = ControlBindings::instance();
	for (const auto& def : bindings.definitions()) {
		auto* edit = new ControlKeySequenceEdit(bindings.keyFor(def.action), this);
		_editors.emplace(def.action, edit);
		form->addRow(def.label + ':', edit);
	}

	auto* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
	auto* resetButton = buttonBox->addButton(tr("Reset to Defaults"), QDialogButtonBox::ResetRole);

	connect(buttonBox, &QDialogButtonBox::accepted, this, [this]() {
		applyChanges();
		accept();
	});
	connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
	connect(resetButton, &QPushButton::clicked, this, &ControlsDialog::resetDefaults);

	layout->addWidget(buttonBox);
}

void ControlsDialog::applyChanges() {
	auto& bindings = ControlBindings::instance();
	for (const auto& editor : _editors) {
		bindings.setKey(editor.first, editor.second->keySequence());
	}
	bindings.save();
}

void ControlsDialog::resetDefaults() {
	auto& bindings = ControlBindings::instance();
	bindings.resetToDefaults();
	for (const auto& def : bindings.definitions()) {
		auto it = _editors.find(def.action);
		if (it != _editors.end()) {
			it->second->setKeySequence(bindings.keyFor(def.action));
		}
	}
}

} // namespace fso::fred::dialogs
