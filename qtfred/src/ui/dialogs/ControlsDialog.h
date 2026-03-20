#pragma once

#include <QDialog>

#include <map>

#include "ui/ControlBindings.h"

class QDialogButtonBox;
class QFormLayout;
class QKeySequenceEdit;

namespace fso::fred::dialogs {

class ControlsDialog : public QDialog {
 Q_OBJECT
 public:
	explicit ControlsDialog(QWidget* parent = nullptr);

 private:
	void applyChanges();
	void resetDefaults();

	std::map<ControlAction, QKeySequenceEdit*> _editors;
};

} // namespace fso::fred::dialogs
