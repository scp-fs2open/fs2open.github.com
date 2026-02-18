#include "ui/dialogs/PropEditorDialog.h"

#include "ui_PropEditorDialog.h"

#include <ui/util/SignalBlockers.h>
#include <ui/widgets/FlagList.h>

#include <QMetaObject>

namespace fso::fred::dialogs {

PropEditorDialog::PropEditorDialog(FredView* parent, EditorViewport* viewport)
	: QDialog(parent), ui(new ::Ui::PropEditorDialog()), _model(new PropEditorDialogModel(this, viewport)) {
	ui->setupUi(this);

	initializeUi();
	updateUi();

	connect(_model.get(), &PropEditorDialogModel::modelDataChanged, this, [this]() {
		initializeUi();
		updateUi();
	});

	connect(ui->propFlagsListWidget, &fso::fred::FlagListWidget::flagToggled, this, [this](const QString& name, int checked) {
		const auto& labels = _model->getFlagLabels();
		for (size_t i = 0; i < labels.size(); ++i) {
			if (name == QString::fromStdString(labels[i].first)) {
				_model->setFlagState(i, checked);
				break;
			}
		}

		// Applying immediately can re-enter FlagListWidget while it is still processing
		// itemChanged, which may invalidate the underlying item/model pointers.
		// Queue the apply until the current signal stack unwinds.
		QMetaObject::invokeMethod(this, [this]() { _model->apply(); }, Qt::QueuedConnection);
	});

	resize(QDialog::sizeHint());
}

PropEditorDialog::~PropEditorDialog() = default;

void PropEditorDialog::initializeUi() {
	util::SignalBlockers blockers(this);

	const auto& labels = _model->getFlagLabels();
	QVector<std::pair<QString, int>> toWidget;
	toWidget.reserve(static_cast<int>(labels.size()));

	for (size_t i = 0; i < labels.size(); ++i) {
		toWidget.append({QString::fromStdString(labels[i].first), _model->getFlagState()[i]});
	}
	ui->propFlagsListWidget->setFlags(toWidget);
	ui->propFlagsListWidget->setFilterVisible(true);
	ui->propFlagsListWidget->setToolbarVisible(true);

	const auto enable = _model->hasValidSelection();
	ui->propNameLineEdit->setEnabled(enable && !_model->hasMultipleSelection());
	ui->propFlagsListWidget->setEnabled(enable);
	ui->nextButton->setEnabled(enable);
	ui->prevButton->setEnabled(enable);
}

void PropEditorDialog::updateUi() {
	util::SignalBlockers blockers(this);

	ui->propNameLineEdit->setText(QString::fromStdString(_model->getPropName()));
}

void PropEditorDialog::on_propNameLineEdit_editingFinished() {
	_model->setPropName(ui->propNameLineEdit->text().toUtf8().constData());
	if (!_model->apply()) {
		updateUi();
	}
}

void PropEditorDialog::on_nextButton_clicked() {
	_model->selectNextProp();
}

void PropEditorDialog::on_prevButton_clicked() {
	_model->selectPreviousProp();
}

} // namespace fso::fred::dialogs
