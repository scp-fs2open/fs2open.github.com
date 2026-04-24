#include "WingFlagsDialog.h"

#include "ui_WingFlagsDialog.h"

#include <ui/widgets/FlagList.h>

namespace fso::fred::dialogs {

WingFlagsDialog::WingFlagsDialog(QWidget* parent, const std::vector<std::pair<SCP_string, bool>>& flags,
                                 const std::vector<std::pair<SCP_string, SCP_string>>& descriptions)
	: QDialog(parent), ui(new Ui::WingFlagsDialog())
{
	ui->setupUi(this);

	QVector<std::pair<QString, int>> qtFlags;
	qtFlags.reserve(static_cast<int>(flags.size()));
	for (const auto& f : flags)
		qtFlags.append({QString::fromUtf8(f.first.c_str()), f.second ? Qt::Checked : Qt::Unchecked});
	ui->flagList->setFlags(qtFlags);

	if (!descriptions.empty()) {
		QVector<std::pair<QString, QString>> qtDescs;
		qtDescs.reserve(static_cast<int>(descriptions.size()));
		for (const auto& d : descriptions)
			qtDescs.append({QString::fromUtf8(d.first.c_str()), QString::fromUtf8(d.second.c_str())});
		ui->flagList->setFlagDescriptions(qtDescs);
	}

	resize(QDialog::sizeHint());
}

WingFlagsDialog::~WingFlagsDialog() = default;

std::vector<std::pair<SCP_string, bool>> WingFlagsDialog::getFlags() const
{
	std::vector<std::pair<SCP_string, bool>> result;
	for (const auto& f : ui->flagList->getFlags())
		result.emplace_back(f.first.toUtf8().constData(), f.second == Qt::Checked);
	return result;
}

} // namespace fso::fred::dialogs
