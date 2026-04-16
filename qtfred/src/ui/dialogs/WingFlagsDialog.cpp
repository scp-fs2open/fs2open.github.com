#include "WingFlagsDialog.h"

#include "ui_WingFlagsDialog.h"

#include <ui/widgets/FlagList.h>

namespace fso::fred::dialogs {

WingFlagsDialog::WingFlagsDialog(QWidget* parent, const std::vector<std::pair<SCP_string, bool>>& flags)
	: QDialog(parent), ui(new Ui::WingFlagsDialog()), _flags(flags)
{
	ui->setupUi(this);

	QVector<std::pair<QString, int>> qtFlags;
	qtFlags.reserve(static_cast<int>(_flags.size()));
	for (const auto& f : _flags) {
		qtFlags.append({QString::fromStdString(f.first), f.second ? Qt::Checked : Qt::Unchecked});
	}
	ui->flagList->setFlags(qtFlags);

	connect(ui->flagList, &fso::fred::FlagListWidget::flagToggled, this, [this](const QString& name, int checked) {
		for (auto& flag : _flags) {
			if (flag.first == name.toUtf8().constData()) {
				flag.second = (checked == Qt::Checked);
				break;
			}
		}
	});

	resize(QDialog::sizeHint());
}

WingFlagsDialog::~WingFlagsDialog() = default;

std::vector<std::pair<SCP_string, bool>> WingFlagsDialog::getFlags() const
{
	return _flags;
}

} // namespace fso::fred::dialogs
