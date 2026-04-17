#include "ErrorCheckerDialogModel.h"

#include "mission/EditorViewport.h"

namespace fso::fred::dialogs {


ErrorCheckerDialogModel::ErrorCheckerDialogModel(QObject* parent, EditorViewport* viewport)
	: AbstractDialogModel(parent, viewport)
	, _checker(std::make_unique<ErrorChecker>(viewport))
{
}

bool ErrorCheckerDialogModel::apply() {
	return true;
}

void ErrorCheckerDialogModel::reject() {
}

bool ErrorCheckerDialogModel::runCheck() {
	_checker = std::make_unique<ErrorChecker>(_viewport);
	bool noErrors = _checker->runFullCheck();
	_hasBeenRun = true;
	modelChanged();
	return !noErrors;
}

void ErrorCheckerDialogModel::clearErrors() {
	_hasBeenRun = false;
	modelChanged();
}

bool ErrorCheckerDialogModel::hasBeenRun() const {
	return _hasBeenRun;
}

const SCP_vector<ErrorEntry>& ErrorCheckerDialogModel::getErrors() const {
	return _checker->getErrors();
}

bool ErrorCheckerDialogModel::getCheckPotentialIssues() const {
	return _viewport->Error_checker_checks_potential_issues;
}

void ErrorCheckerDialogModel::setCheckPotentialIssues(bool value) {
	_viewport->Error_checker_checks_potential_issues = value;
	_viewport->saveSettings();
}

bool ErrorCheckerDialogModel::getApplyAutoCorrections() const {
	return _viewport->Error_checker_apply_auto_corrections;
}

void ErrorCheckerDialogModel::setApplyAutoCorrections(bool value) {
	_viewport->Error_checker_apply_auto_corrections = value;
	_viewport->saveSettings();
}

} // namespace fso::fred::dialogs
