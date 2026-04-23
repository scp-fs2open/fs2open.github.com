#pragma once

#include <globalincs/pstypes.h>
#include <mission/dialogs/WingEditorDialogModel.h>

#include <QDialog>
#include <memory>
#include <vector>

namespace fso::fred::dialogs {

namespace Ui {
class WingFlagsDialog;
}

class WingFlagsDialog : public QDialog {
	Q_OBJECT
  public:
	explicit WingFlagsDialog(QWidget* parent, const std::vector<std::pair<SCP_string, bool>>& flags,
	                         const std::vector<std::pair<SCP_string, SCP_string>>& descriptions = {});
	~WingFlagsDialog() override;

	std::vector<std::pair<SCP_string, bool>> getFlags() const;

  private:
	std::unique_ptr<Ui::WingFlagsDialog> ui;
	std::vector<std::pair<SCP_string, bool>> _flags;
};

} // namespace fso::fred::dialogs
