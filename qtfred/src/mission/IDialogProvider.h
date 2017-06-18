#pragma once


#include <globalincs/flagset.h>

namespace fso {
namespace fred {

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
};

}
}

