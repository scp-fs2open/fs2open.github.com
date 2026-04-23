#pragma once

#include <QDialog>
#include <QPushButton>

#include "mission/dialogs/ErrorCheckerDialogModel.h"

class QVBoxLayout;

namespace fso::fred::dialogs {

namespace Ui {
class ErrorCheckerDialog;
}

class ErrorCheckerDialog final : public QDialog {
	Q_OBJECT

public:
	enum class Mode { Normal, PreSave };

	// Action chosen by the designer in PreSave mode.
	// Only meaningful after exec() returns in PreSave mode.
	enum class PreSaveAction { Cancel, SaveAsIs, FixAndSave };

	explicit ErrorCheckerDialog(QWidget* parent, EditorViewport* viewport, Mode mode = Mode::Normal);
	~ErrorCheckerDialog() override;

	PreSaveAction preSaveAction() const { return _preSaveAction; }

	// Number of entries in the most recent check result (0 if not yet run).
	int getErrorCount() const;
	const SCP_vector<ErrorEntry>& getErrors() const;

	// Force potential issues to be displayed on the next updateUi pass even if
	// the user's saved preference has them hidden. Does not persist and does
	// not affect checkbox state. Intended for contexts that want to draw
	// attention to potentials without overwriting user preferences (e.g. after
	// a data migration). Caller can clear by passing false.
	void setForcePotentialsDisplay(bool force);

public slots: // NOLINT(readability-redundant-access-specifiers)
	bool runCheck(); // returns true if no errors were found
	void clearErrors();

private slots:
	void on_runButton_clicked();
	void on_closeButton_clicked();
	void on_checkPotentialIssues_toggled(bool checked);
	void on_checkApplyAutoCorrections_toggled(bool checked);

protected:
	void changeEvent(QEvent* event) override;

private: // NOLINT(readability-redundant-access-specifiers)
	void initializeUi();
	void updateUi();

	std::unique_ptr<Ui::ErrorCheckerDialog> ui;
	std::unique_ptr<ErrorCheckerDialogModel> _model;
	QVBoxLayout* _errorLayout = nullptr;

	Mode          _mode           = Mode::Normal;
	PreSaveAction _preSaveAction  = PreSaveAction::Cancel;
	QPushButton*  _fixSaveButton  = nullptr; // PreSave mode only; used in updateUi
	bool          _forcePotentialsDisplay = false;
};

} // namespace fso::fred::dialogs
