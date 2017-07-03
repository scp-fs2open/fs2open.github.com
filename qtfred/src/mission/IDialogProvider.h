#pragma once


#include <globalincs/flagset.h>

#include <memory>

namespace fso {
namespace fred {

namespace dialogs {
class FormWingDialogModel;
}

class IBaseDialog {
 public:
	virtual ~IBaseDialog() {}
};

template<typename TModel>
class IDialog : public IBaseDialog {
 public:
	virtual ~IDialog() {
	}

	virtual TModel* getModel() = 0;
};

FLAG_LIST(DialogButton) {
	Yes = 0, No, Cancel, Ok, NUM_VALUES
};

enum class DialogType {
	Error, Warning, Information, Question
};

class IDialogProvider {
 public:
	virtual ~IDialogProvider() {
	}

	virtual DialogButton showButtonDialog(DialogType type,
										  const SCP_string& title,
										  const SCP_string& message,
										  const flagset<DialogButton>& buttons) = 0;

	virtual std::unique_ptr<IDialog<dialogs::FormWingDialogModel>> createFormWingDialog() = 0;

	virtual bool showModalDialog(IBaseDialog* dlg) = 0;
};

}
}

