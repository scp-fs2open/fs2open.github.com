#include "HeadAnimationPickerDialog.h"

#include "ui/util/ImageRenderer.h"

#include <bmpman/bmpman.h>
#include <cfile/cfile.h>

#include <QBoxLayout>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
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

	_variantButtonWidget = new QWidget(this);
	_variantButtonWidget->setVisible(false);
	_variantButtonLayout = new QHBoxLayout(_variantButtonWidget);
	_variantButtonLayout->setContentsMargins(0, 2, 0, 0);
	_variantButtonLayout->setSpacing(4);
	previewCol->addWidget(_variantButtonWidget);

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
	rebuildVariantButtons(_selected);
}

void HeadAnimationPickerDialog::onBrowse()
{
	// Start the file dialog in the FSO interface directory
	int z = cfile_push_chdir(CF_TYPE_INTERFACE);
	const QString interfacePath = QDir::currentPath();
	if (!z) {
		cfile_pop_dir();
	}

	const QString filters = "FSO Animations (*.ani *.eff *.png);;All Files (*.*)";
	const QString filename = QFileDialog::getOpenFileName(this, tr("Select Head Animation"), interfacePath, filters);
	if (filename.isEmpty()) {
		return;
	}

	// Store just the base name
	const QString rawName = QFileInfo(filename).completeBaseName();

	// If the selected file has a trailing set-designator (single letter, -reg,
	// or -death) and the stripped base also resolves, use the shorter base name.
	// This mirrors how built-ins are stored: "Head-TP2" not "Head-TP2a".
	const QString baseName = normalizeHeadAniName(rawName);

	// Add to the gallery list if not already present
	if (!_allNames.contains(baseName, Qt::CaseInsensitive)) {
		_allNames.append(baseName);
	}

	_selected = baseName;
	_previewingName = baseName;
	_previewElapsedSeconds = 0.0f;
	rebuildList();
}

void HeadAnimationPickerDialog::onOk()
{
	accept();
}

void HeadAnimationPickerDialog::onTick()
{
	if (_previewingName.isEmpty()) {
		return;
	}

	auto* preview = ensurePreview(_previewingName);
	if (!preview || preview->numFrames <= 1) {
		return;
	}

	_previewElapsedSeconds += 0.033f;
	updatePreview();
}

void HeadAnimationPickerDialog::rebuildList()
{
	{
		const QSignalBlocker blocker(_list);

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
	} // signals re-enabled here

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

void HeadAnimationPickerDialog::loadPreviewFrames(PreviewData* preview)
{
	if (!preview || preview->firstFrame < 0 || !preview->frames.isEmpty()) {
		return;
	}

	for (int i = 0; i < preview->numFrames; ++i) {
		QImage img;
		if (loadHandleToQImage(preview->firstFrame + i, img, nullptr) && !img.isNull()) {
			preview->frames.push_back(QPixmap::fromImage(img));
		} else {
			preview->frames.push_back(QPixmap()); // placeholder so indices stay consistent
		}
	}
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

	loadPreviewFrames(preview);

	// Pick the right frame based on elapsed time.
	QPixmap pixmap;
	if (!preview->frames.isEmpty()) {
		int idx = 0;
		if (preview->frames.size() > 1) {
			idx = bm_get_anim_frame(preview->firstFrame, _previewElapsedSeconds, 0.0f, true);
			idx = qBound(0, idx, preview->frames.size() - 1);
		}
		pixmap = preview->frames[idx];
	}

	if (pixmap.isNull()) {
		pixmap = preview->firstPixmap; // fallback to the static thumbnail
	}

	if (pixmap.isNull()) {
		_previewLabel->setPixmap(QPixmap());
		_previewLabel->setText("No preview available");
		return;
	}

	const auto scaled = pixmap.scaled(_previewLabel->size() - QSize(8, 8), Qt::KeepAspectRatio, Qt::SmoothTransformation);
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
			rebuildVariantButtons(name);
			return;
		}
	}

	if (!name.isEmpty()) {
		_previewingName = name;
		_previewElapsedSeconds = 0.0f;
		updatePreview();
		rebuildVariantButtons(name);
	}
}

QString HeadAnimationPickerDialog::normalizeHeadAniName(const QString& baseName)
{
	// Strip trailing set-designators (-reg, -death, or a single letter a-z)
	// and check whether the resulting base resolves to a valid animation

	const QString lower = baseName.toLower();
	QString stripped;

	if (lower.endsWith("-reg")) {
		stripped = baseName.left(baseName.length() - 4);
	} else if (lower.endsWith("-death")) {
		stripped = baseName.left(baseName.length() - 6);
	} else if (baseName.length() > 1) {
		const QChar last = baseName.at(baseName.length() - 1).toLower();
		if (last >= QLatin1Char('a') && last <= QLatin1Char('z')) {
			stripped = baseName.left(baseName.length() - 1);
		}
	}

	if (!stripped.isEmpty() && !findPreviewSource(stripped).isEmpty()) {
		return stripped;
	}

	return baseName;
}

QString HeadAnimationPickerDialog::findPreviewSource(const QString& displayName)
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

QStringList HeadAnimationPickerDialog::detectVariants(const QString& baseName)
{
	if (baseName.isEmpty() || !baseName.compare("<None>", Qt::CaseInsensitive)) {
		return {};
	}

	QStringList variants;

	// Check single-letter suffixes a-z sequentially; stop at first gap.
	for (char c = 'a'; c <= 'z'; ++c) {
		const QString letterName = baseName + QLatin1Char(c);
		const auto letterNameBytes = letterName.toUtf8();
		if (bm_load_animation(letterNameBytes.constData()) >= 0 || bm_load(letterNameBytes.constData()) >= 0) {
			variants << letterName;
		} else {
			break;
		}
	}

	// Check -reg and -death suffixes.
	const QString regName = baseName + "-reg";
	const auto regNameBytes = regName.toUtf8();
	if (bm_load_animation(regNameBytes.constData()) >= 0 || bm_load(regNameBytes.constData()) >= 0) {
		variants << regName;
	}

	const QString deathName = baseName + "-death";
	const auto deathNameBytes = deathName.toUtf8();
	if (bm_load_animation(deathNameBytes.constData()) >= 0 || bm_load(deathNameBytes.constData()) >= 0) {
		variants << deathName;
	}

	return variants;
}

void HeadAnimationPickerDialog::rebuildVariantButtons(const QString& baseName)
{
	// Remove all items from the layout
	while (_variantButtonLayout->count() > 0) {
		QLayoutItem* item = _variantButtonLayout->takeAt(0);
		if (QWidget* w = item->widget()) {
			w->deleteLater();
		}
		delete item;
	}
	_variantButtons.clear();

	const QStringList variants = detectVariants(baseName);

	if (variants.size() < 2) {
		_variantButtonWidget->setVisible(false);
		return;
	}

	// Determine which variant is currently active
	const QString activeVariant = findPreviewSource(baseName);

	_variantButtonLayout->addStretch(1);

	for (const QString& variant : variants) {
		// Derive a short label by stripping the base name prefix.
		QString label = variant.mid(baseName.length());
		if (label.startsWith(QLatin1Char('-'))) {
			label = label.mid(1); // strip leading dash from "-reg", "-death"
		}
		if (label.isEmpty()) {
			label = variant;
		}

		auto* btn = new QPushButton(label, _variantButtonWidget);
		btn->setCheckable(true);
		btn->setChecked(variant.compare(activeVariant, Qt::CaseInsensitive) == 0);
		btn->setProperty("variantName", variant);

		connect(btn, &QPushButton::clicked, this, [this, variant]() {
			_previewingName = variant;
			_previewElapsedSeconds = 0.0f;
			updatePreview();
			for (auto* b : _variantButtons) {
				b->setChecked(b->property("variantName").toString().compare(variant, Qt::CaseInsensitive) == 0);
			}
		});

		_variantButtonLayout->addWidget(btn);
		_variantButtons.push_back(btn);
	}

	_variantButtonLayout->addStretch(1);
	_variantButtonWidget->setVisible(true);
}
