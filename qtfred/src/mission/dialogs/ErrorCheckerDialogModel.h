#pragma once

#include "AbstractDialogModel.h"
#include "ui/util/ErrorChecker.h"

#include <memory>

namespace fso::fred::dialogs {

class ErrorCheckerDialogModel : public AbstractDialogModel {
	Q_OBJECT

public:
	ErrorCheckerDialogModel(QObject* parent, EditorViewport* viewport);
	~ErrorCheckerDialogModel() override = default;

	bool apply() override;
	void reject() override;

	bool runCheck();
	void clearErrors();

	bool hasBeenRun() const;
	const SCP_vector<ErrorEntry>& getErrors() const;

	bool getCheckPotentialIssues() const;
	void setCheckPotentialIssues(bool value);

	bool getApplyAutoCorrections() const;
	void setApplyAutoCorrections(bool value);

private:
	std::unique_ptr<ErrorChecker> _checker;
	bool _hasBeenRun = false;
};

} // namespace fso::fred::dialogs
