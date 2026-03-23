#include "HeadAnimationPickerDialog.h"

#include "ui/util/ImageRenderer.h"

#include <bmpman/bmpman.h>

#include <QBoxLayout>
#include <QFileDialog>
#include <QLineEdit>
#include <QStyle>
#include <algorithm>

using fso::fred::util::loadHandleToQImage;

HeadAnimationPickerDialog::HeadAnimationPickerDialog(QWidget* parent) : QDialog(parent)
{
	setWindowTitle("Choose Head Animation");
	resize(860, 560);

	auto* root = new QVBoxLayout(this);

	_filterEdit = new QLineEdit(this);
	_filterEdit->setPlaceholderText("Filter by name...");
	root->addWidget(_filterEdit);

	auto* content = new QHBoxLayout();
	_list = new QListWidget(this);
	_list->setViewMode(QListView::IconMode);
	_list->setIconSize(QSize(112, 112));
	_list->setResizeMode(QListView::Adjust);
	_list->setUniformItemSizes(true);
	_list->setMovement(QListView::Static);
	_list->setSelectionMode(QAbstractItemView::SingleSelection);
	_list->setSpacing(8);
	content->addWidget(_list, 1);

	auto* previewCol = new QVBoxLayout();
	_previewLabel = new QLabel(this);
	_previewLabel->setMinimumSize(QSize(220, 220));
	_previewLabel->setAlignment(Qt::AlignCenter);
	_previewLabel->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
	previewCol->addWidget(_previewLabel, 1);
	content->addLayout(previewCol);

	root->addLayout(content, 1);

	auto* buttons = new QHBoxLayout();
	_browseBtn = new QPushButton("Browse", this);
	_okBtn = new QPushButton("OK", this);
	_cancelBtn = new QPushButton("Cancel", this);
	buttons->addWidget(_browseBtn);
	buttons->addStretch(1);
	buttons->addWidget(_okBtn);
	buttons->addWidget(_cancelBtn);
	root->addLayout(buttons);

	connect(_filterEdit, &QLineEdit::textChanged, this, &HeadAnimationPickerDialog::onFilterTextChanged);
	connect(_list, &QListWidget::itemActivated, this, &HeadAnimationPickerDialog::onItemActivated);
	connect(_list, &QListWidget::itemSelectionChanged, this, &HeadAnimationPickerDialog::onSelectionChanged);
	connect(_browseBtn, &QPushButton::clicked, this, &HeadAnimationPickerDialog::onBrowse);
	connect(_okBtn, &QPushButton::clicked, this, &HeadAnimationPickerDialog::onOk);
	connect(_cancelBtn, &QPushButton::clicked, this, &QDialog::reject);

	_tickTimer.setInterval(33);
	connect(&_tickTimer, &QTimer::timeout, this, &HeadAnimationPickerDialog::onTick);
	_tickTimer.start();
}

void HeadAnimationPickerDialog::setHeadAnimationNames(const QStringList& names)
{
	_allNames = names;
	rebuildList();
}

void HeadAnimationPickerDialog::setInitialSelection(const QString& name)
{
	_selected = name;
	setSelectedByName(name);
}

void HeadAnimationPickerDialog::onFilterTextChanged(const QString& text)
{
	_filterText = text;
	rebuildList();
}

void HeadAnimationPickerDialog::onItemActivated(QListWidgetItem* item)
{
	if (!item)
		return;
	_selected = item->data(Qt::UserRole).toString();
	accept();
}

void HeadAnimationPickerDialog::onSelectionChanged()
{
	auto* item = _list->currentItem();
	if (!item)
		return;

	_selected = item->data(Qt::UserRole).toString();
	_previewingName = _selected;
	_previewElapsedSeconds = 0.0f;
	updatePreview();
}

void HeadAnimationPickerDialog::onBrowse()
{
	const QString filters = "FSO Animations/Images (*.ani *.eff *.png);;All files (*.*)";
	const QString file = QFileDialog::getOpenFileName(this, tr("Select Head Animation"), QString(), filters);
	if (file.isEmpty()) {
		return;
	}

	_selected = file;
	_previewingName = file;
	_previewElapsedSeconds = 0.0f;
	updatePreview();
}

void HeadAnimationPickerDialog::onOk()
{
	auto* item = _list->currentItem();
	if (item) {
		_selected = item->data(Qt::UserRole).toString();
	}
	accept();
}

void HeadAnimationPickerDialog::onTick()
{
	// Disabled for now: timer-driven frame readback caused instability on some systems.
	// Keep static preview rendering only.
}

void HeadAnimationPickerDialog::rebuildList()
{
	_list->clear();

	const auto fl = _filterText.trimmed().toLower();
	for (const auto& name : _allNames) {
		if (!fl.isEmpty() && !name.toLower().contains(fl)) {
			continue;
		}

		auto* preview = ensurePreview(name);
		QIcon icon;
		if (preview && !preview->firstPixmap.isNull()) {
			icon = QIcon(preview->firstPixmap.scaled(_list->iconSize(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
		} else {
			icon = style()->standardIcon(QStyle::SP_FileIcon);
		}

		auto* it = new QListWidgetItem(icon, name);
		it->setData(Qt::UserRole, name);
		it->setToolTip(name);
		_list->addItem(it);
	}

	setSelectedByName(_selected);
}

HeadAnimationPickerDialog::PreviewData* HeadAnimationPickerDialog::ensurePreview(const QString& displayName)
{
	auto it = _cache.find(displayName);
	if (it != _cache.end()) {
		return &it.value();
	}

	PreviewData previewInfo;
	if (!displayName.compare("<None>", Qt::CaseInsensitive) || displayName.trimmed().isEmpty()) {
		_cache.insert(displayName, previewInfo);
		return &_cache[displayName];
	}

	previewInfo.sourceName = findPreviewSource(displayName);
	if (previewInfo.sourceName.isEmpty()) {
		_cache.insert(displayName, previewInfo);
		return &_cache[displayName];
	}

	int nframes = 1;
	int fps = 15;
	const auto sourceName = previewInfo.sourceName.toUtf8();
	int firstFrame = bm_load_animation(sourceName.constData(), &nframes, &fps);
	if (firstFrame < 0) {
		firstFrame = bm_load(sourceName.constData());
		nframes = 1;
		fps = 15;
	}

	if (firstFrame >= 0) {
		previewInfo.firstFrame = firstFrame;
		previewInfo.numFrames = std::max(1, nframes);
		previewInfo.fps = std::max(1, fps);

		QImage img;
		if (loadHandleToQImage(firstFrame, img, nullptr) && !img.isNull()) {
			previewInfo.firstPixmap = QPixmap::fromImage(img);
		}
	}

	_cache.insert(displayName, previewInfo);
	return &_cache[displayName];
}

void HeadAnimationPickerDialog::updatePreview()
{
	QString displayName = _previewingName;
	if (displayName.isEmpty()) {
		auto* item = _list->currentItem();
		if (item) {
			displayName = item->data(Qt::UserRole).toString();
		}
	}
	if (displayName.isEmpty()) {
		_previewLabel->setPixmap(QPixmap());
		_previewLabel->setText("No selection");
		return;
	}

	auto* preview = ensurePreview(displayName);
	if (!preview || preview->firstFrame < 0) {
		_previewLabel->setPixmap(QPixmap());
		_previewLabel->setText("No preview available");
		return;
	}

	if (preview->firstPixmap.isNull()) {
		_previewLabel->setPixmap(QPixmap());
		_previewLabel->setText("No preview available");
		return;
	}

	const auto scaled = preview->firstPixmap.scaled(_previewLabel->size() - QSize(8, 8), Qt::KeepAspectRatio, Qt::SmoothTransformation);
	_previewLabel->setPixmap(scaled);
}

void HeadAnimationPickerDialog::setSelectedByName(const QString& name)
{
	for (int i = 0; i < _list->count(); ++i) {
		auto* it = _list->item(i);
		if (it->data(Qt::UserRole).toString() == name) {
			_list->setCurrentItem(it);
			_list->scrollToItem(it, QAbstractItemView::PositionAtCenter);
			_previewingName = name;
			_previewElapsedSeconds = 0.0f;
			updatePreview();
			return;
		}
	}

	if (!name.isEmpty()) {
		_previewingName = name;
		updatePreview();
	}
}

QString HeadAnimationPickerDialog::findPreviewSource(const QString& displayName) const
{
	const auto trimmed = displayName.trimmed();
	if (trimmed.isEmpty()) {
		return {};
	}

	QStringList candidates;
	const auto lower = trimmed.toLower();
	const bool looksLikeHead = lower.startsWith("head-");
	const bool explicitSuffix = lower.endsWith("-reg") || lower.endsWith("-death");

	if (looksLikeHead && !explicitSuffix) {
		candidates << (trimmed + "a") << (trimmed + "-reg") << trimmed;
	} else {
		candidates << trimmed;
	}

	for (const auto& candidate : candidates) {
		if (candidate.isEmpty()) {
			continue;
		}

		const auto candidateName = candidate.toUtf8();
		if (bm_load_animation(candidateName.constData()) >= 0 || bm_load(candidateName.constData()) >= 0) {
			return candidate;
		}
	}

	return {};
}
