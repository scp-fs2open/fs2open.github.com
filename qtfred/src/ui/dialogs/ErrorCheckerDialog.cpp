#include "ErrorCheckerDialog.h"
#include "ui_ErrorCheckerDialog.h"

#include <algorithm>

#include <QEvent>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPalette>
#include <QVBoxLayout>
#include <QWidget>

namespace fso::fred::dialogs {

ErrorCheckerDialog::ErrorCheckerDialog(QWidget* parent, EditorViewport* viewport, Mode mode)
	: QDialog(parent)
	, ui(new Ui::ErrorCheckerDialog())
	, _model(new ErrorCheckerDialogModel(this, viewport))
	, _mode(mode)
{
	ui->setupUi(this);
	connect(_model.get(), &ErrorCheckerDialogModel::modelChanged, this, &ErrorCheckerDialog::updateUi);

	initializeUi();
	updateUi();
}

ErrorCheckerDialog::~ErrorCheckerDialog() = default;

bool ErrorCheckerDialog::runCheck() {
	return _model->runCheck();
}

void ErrorCheckerDialog::clearErrors() {
	_model->clearErrors();
}

int ErrorCheckerDialog::getErrorCount() const {
	return static_cast<int>(_model->getErrors().size());
}

const SCP_vector<ErrorEntry>& ErrorCheckerDialog::getErrors() const {
	return _model->getErrors();
}

void ErrorCheckerDialog::setForcePotentialsDisplay(bool force) {
	if (_forcePotentialsDisplay == force)
		return;
	_forcePotentialsDisplay = force;
	updateUi();
}

void ErrorCheckerDialog::on_runButton_clicked() {
	_model->runCheck();
}

void ErrorCheckerDialog::on_closeButton_clicked() {
	close();
}

void ErrorCheckerDialog::on_checkPotentialIssues_toggled(bool checked) {
	_model->setCheckPotentialIssues(checked);
}

void ErrorCheckerDialog::on_checkApplyAutoCorrections_toggled(bool checked) {
	_model->setApplyAutoCorrections(checked);
}

void ErrorCheckerDialog::changeEvent(QEvent* event) {
	QDialog::changeEvent(event);
	if (event->type() == QEvent::PaletteChange || event->type() == QEvent::StyleChange) {
		updateUi();
	}
}

void ErrorCheckerDialog::initializeUi() {
	// --- Color legend bar (common to both modes) ---
	// Sits between the toolbar row and the scroll area so designers always know what
	// the stripe colors mean without having to hover over anything.
	{
		auto* legend       = new QWidget(this);
		auto* legendLayout = new QHBoxLayout(legend);
		legendLayout->setContentsMargins(4, 2, 4, 2);
		legendLayout->setSpacing(10);

		for (const auto& info : fso::fred::severity_info) {
			auto* swatch = new QWidget(legend);
			swatch->setFixedSize(12, 12);
			swatch->setAutoFillBackground(true);
			QPalette p = swatch->palette();
			p.setColor(QPalette::Window, QColor(info.r, info.g, info.b));
			swatch->setPalette(p);
			swatch->setToolTip(tr(info.tooltip));
			legendLayout->addWidget(swatch);

			auto* lbl = new QLabel(tr(info.label), legend);
			lbl->setToolTip(tr(info.tooltip));
			legendLayout->addWidget(lbl);
		}
		legendLayout->addStretch();

		// Insert at index 1: after the toolbar layout, before the scroll area.
		ui->mainLayout->insertWidget(1, legend);
	}

	// --- Auto-correction nudge (Normal mode only) ---
	// Sits between the legend and the scroll area. Shown by updateUi() when the
	// run produced warnings and "Apply auto-corrections" is currently off, so the
	// designer knows the warnings can be resolved without manual editing.
	if (_mode == Mode::Normal) {
		_autoFixNudge = new QLabel(this);
		_autoFixNudge->setWordWrap(true);
		_autoFixNudge->setContentsMargins(6, 4, 6, 4);
		_autoFixNudge->hide();
		ui->mainLayout->insertWidget(2, _autoFixNudge);
	}

	// --- Scroll area content (common to both modes) ---
	auto* scrollContent = new QWidget(ui->errorScrollArea);
	_errorLayout = new QVBoxLayout(scrollContent);
	_errorLayout->setAlignment(Qt::AlignTop);
	_errorLayout->setSpacing(4);
	_errorLayout->setContentsMargins(4, 4, 4, 4);
	ui->errorScrollArea->setWidget(scrollContent);
	ui->errorScrollArea->setWidgetResizable(true);

	if (_mode == Mode::Normal) {
		ui->checkPotentialIssues->setChecked(_model->getCheckPotentialIssues());
		ui->checkApplyAutoCorrections->setChecked(_model->getApplyAutoCorrections());
		return;
	}

	// --- PreSave mode ---
	setWindowTitle(tr("Pre-save Error Check"));
	setWindowModality(Qt::WindowModal);

	// Authoring controls are not relevant for a one-shot pre-save scan
	ui->runButton->hide();
	ui->checkPotentialIssues->hide();
	ui->checkApplyAutoCorrections->hide();

	// Note explaining Fix and Save's scope, inserted above the button row
	auto* scopeNote = new QLabel(
		tr("<i>\"Fix and Save\" only applies automatic corrections for simple, well-defined issues "
		   "(such as missing loadout pool entries). Complex errors must be addressed manually.</i>"),
		this);
	scopeNote->setWordWrap(true);
	// Insert just before the last item (bottomLayout) in the main layout
	ui->mainLayout->insertWidget(ui->mainLayout->count() - 1, scopeNote);

	// Replace the Close button with the three pre-save decision buttons
	ui->closeButton->hide();

	_fixSaveButton         = new QPushButton(tr("Fix and Save"), this);
	auto* saveAnywayButton = new QPushButton(tr("Save Anyway"),  this);
	auto* cancelButton     = new QPushButton(tr("Cancel"),       this);

	_fixSaveButton->setToolTip(
		tr("Apply automatic corrections to simple, well-defined errors, then save.\n"
		   "Issues that cannot be auto-corrected will remain and must be fixed manually."));

	// Insert before the hidden Close button so visual order matches expected flow
	const int closeIdx = ui->bottomLayout->indexOf(ui->closeButton);
	ui->bottomLayout->insertWidget(closeIdx,     _fixSaveButton);
	ui->bottomLayout->insertWidget(closeIdx + 1, saveAnywayButton);
	ui->bottomLayout->insertWidget(closeIdx + 2, cancelButton);

	connect(_fixSaveButton,   &QPushButton::clicked, this, [this]() { _preSaveAction = PreSaveAction::FixAndSave; accept(); });
	connect(saveAnywayButton, &QPushButton::clicked, this, [this]() { _preSaveAction = PreSaveAction::SaveAsIs;   accept(); });
	connect(cancelButton,     &QPushButton::clicked, this, [this]() { _preSaveAction = PreSaveAction::Cancel;      reject(); });
}

void ErrorCheckerDialog::updateUi() {
	while (QLayoutItem* item = _errorLayout->takeAt(0)) {
		if (QWidget* w = item->widget())
			w->deleteLater();
		delete item;
	}

	if (_autoFixNudge)
		_autoFixNudge->hide();

	if (!_model->hasBeenRun()) {
		ui->statusLabel->setText(tr("No check has been run yet."));
		if (_fixSaveButton)
			_fixSaveButton->setEnabled(false);
		return;
	}

	// Build the display list: always-run checks collect everything; the preference
	// only controls whether potential issues are visible in the UI. A transient
	// override (setForcePotentialsDisplay) can force potentials on for a single
	// session without changing the saved preference.
	const bool showPotential = _forcePotentialsDisplay || _model->getCheckPotentialIssues();
	auto errors = _model->getErrors();
	if (!showPotential) {
		errors.erase(std::remove_if(errors.begin(), errors.end(),
			[](const ErrorEntry& e) { return e.severity == ErrorSeverity::Potential; }),
			errors.end());
	}

	if (errors.empty()) {
		ui->statusLabel->setText(tr("No errors found!"));
		if (_fixSaveButton)
			_fixSaveButton->setEnabled(false);
		return;
	}

	// Sort so the UI always shows Critical → Error → Warning → Potential,
	// regardless of the order checks were run. The enum is ordered by severity so a
	// plain less-than comparison gives the right result.
	std::sort(errors.begin(), errors.end(), [](const ErrorEntry& a, const ErrorEntry& b) {
		return a.severity < b.severity;
	});

	const QColor windowColor = ui->errorScrollArea->palette().color(QPalette::Window);
	const QColor cardBg = windowColor.lightness() < 128
		? windowColor.lighter(115)
		: windowColor.darker(108);

	int errorCount     = 0;
	int warningCount   = 0;
	int potentialCount = 0;
	bool hasAutoFixable = false;

	for (const auto& entry : errors) {
		const fso::fred::SeverityInfo& info = fso::fred::infoFor(entry.severity);

		switch (entry.severity) {
		case ErrorSeverity::Error:
		case ErrorSeverity::InternalError:
			++errorCount;
			break;
		case ErrorSeverity::Warning:
			++warningCount;
			hasAutoFixable = true;
			break;
		case ErrorSeverity::Potential:
			++potentialCount;
			break;
		}

		auto* card = new QFrame();
		card->setFrameShape(QFrame::StyledPanel);
		card->setFrameShadow(QFrame::Plain);
		card->setAutoFillBackground(true);
		QPalette cardPalette = card->palette();
		cardPalette.setColor(QPalette::Window, cardBg);
		card->setPalette(cardPalette);

		auto* cardLayout = new QHBoxLayout(card);
		cardLayout->setContentsMargins(0, 0, 0, 0);
		cardLayout->setSpacing(0);

		auto* stripe = new QWidget(card);
		stripe->setFixedWidth(5);
		stripe->setAutoFillBackground(true);
		QPalette stripePalette = stripe->palette();
		stripePalette.setColor(QPalette::Window, QColor(info.r, info.g, info.b));
		stripe->setPalette(stripePalette);
		// Tooltip on the stripe gives context without cluttering the message text
		stripe->setToolTip(tr("%1 — %2").arg(tr(info.label), tr(info.tooltip)));
		cardLayout->addWidget(stripe);

		auto* label = new QLabel(QString::fromStdString(entry.message), card);
		label->setWordWrap(true);
		label->setContentsMargins(8, 6, 8, 6);
		cardLayout->addWidget(label);

		_errorLayout->addWidget(card);
	}

	QStringList parts;
	if (errorCount > 0)
		parts << tr("%1 error(s)").arg(errorCount);
	if (warningCount > 0)
		parts << tr("%1 warning(s)").arg(warningCount);
	if (potentialCount > 0)
		parts << tr("%1 potential issue(s)").arg(potentialCount);
	ui->statusLabel->setText(parts.join(tr(", ")) + tr(" found."));

	// "Fix and Save" is only enabled when there are entries the auto-corrector can address.
	// Under the current taxonomy, only Warnings have auto-fixes; Errors must be addressed manually.
	if (_fixSaveButton)
		_fixSaveButton->setEnabled(hasAutoFixable);

	// Surface the auto-correction nudge when the run produced warnings and the
	// designer doesn't currently have auto-corrections enabled. PreSave mode has
	// the dedicated "Fix and Save" button instead, so the nudge is suppressed there.
	if (_autoFixNudge && warningCount > 0 && !_model->getApplyAutoCorrections()) {
		_autoFixNudge->setText(tr("<b>%1 warning(s)</b> can be fixed automatically. Enable <i>Apply auto-corrections</i> and re-run.").arg(warningCount));
		_autoFixNudge->show();
	}
}

} // namespace fso::fred::dialogs
