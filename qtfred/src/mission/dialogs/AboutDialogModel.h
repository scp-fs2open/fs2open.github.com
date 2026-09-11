#pragma once

#include "AbstractDialogModel.h"

#include <globalincs/pstypes.h>

namespace fso::fred::dialogs {

class AboutDialogModel : public AbstractDialogModel {
	Q_OBJECT

public:
	AboutDialogModel(QObject* parent, EditorViewport* viewport);

	bool apply() override;
	void reject() override;

	static SCP_string getVersionString();
	static SCP_string getCopyrightString();
	static SCP_vector<SCP_string> getQtFREDCredits();
	static SCP_vector<SCP_string> getGraphicsCredits();
	static SCP_string getSCPCreditsText();
	static SCP_string getQuoteString();
};

} // namespace fso::fred::dialogs
