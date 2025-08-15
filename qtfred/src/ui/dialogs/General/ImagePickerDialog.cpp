#include "ImagePickerDialog.h"

#include "ui/util/ImageRenderer.h"

#include <QBoxLayout>
#include <QImage>
#include <QStyle>
#include <QPainter>

using fso::fred::util::loadImageToQImage;

ImagePickerDialog::ImagePickerDialog(QWidget* parent) : QDialog(parent)
{
	setWindowTitle("Choose Image");
	resize(720, 520);

	auto* vbox = new QVBoxLayout(this);

	_filterEdit = new QLineEdit(this);
	_filterEdit->setPlaceholderText("Filter by name...");
	vbox->addWidget(_filterEdit);

	_list = new QListWidget(this);
	_list->setViewMode(QListView::IconMode);
	_list->setIconSize(QSize(112, 112));
	_list->setResizeMode(QListView::Adjust);
	_list->setUniformItemSizes(true);
	_list->setMovement(QListView::Static);
	_list->setSelectionMode(QAbstractItemView::SingleSelection);
	_list->setSpacing(8);
	vbox->addWidget(_list, 1);

	auto* hbox = new QHBoxLayout();
	hbox->addStretch(1);
	_okBtn = new QPushButton("OK", this);
	_cancelBtn = new QPushButton("Cancel", this);
	hbox->addWidget(_okBtn);
	hbox->addWidget(_cancelBtn);
	vbox->addLayout(hbox);

	connect(_filterEdit, &QLineEdit::textChanged, this, &ImagePickerDialog::onFilterTextChanged);
	connect(_list, &QListWidget::itemActivated, this, &ImagePickerDialog::onItemActivated);
	connect(_okBtn, &QPushButton::clicked, this, &ImagePickerDialog::onOk);
	connect(_cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
}

void ImagePickerDialog::setImageFilenames(const QStringList& filenames)
{
	_allFiles = filenames;
	rebuildList();
}

void ImagePickerDialog::setInitialSelection(const QString& filename)
{
	_selected = filename;
	// Apply after list is built
	for (int i = 0; i < _list->count(); ++i) {
		auto* it = _list->item(i);
		if (it->data(Qt::UserRole).toString() == filename) {
			_list->setCurrentItem(it);
			_list->scrollToItem(it, QAbstractItemView::PositionAtCenter);
			break;
		}
	}
}

void ImagePickerDialog::onFilterTextChanged(const QString& text)
{
	_filterText = text;
	rebuildList();
}

void ImagePickerDialog::onItemActivated(QListWidgetItem* item)
{
	if (!item)
		return;
	_selected = item->data(Qt::UserRole).toString();
	accept();
}

void ImagePickerDialog::onOk()
{
	auto* item = _list->currentItem();
	_selected = item ? item->data(Qt::UserRole).toString() : QString();
	accept();
}

QIcon ImagePickerDialog::iconFor(const QString& name)
{
	if (_thumbCache.contains(name)) {
		return {_thumbCache.value(name)};
	}

	// Decode via bmpman using ImageRenderer
	QImage img;
	QString err;
	if (loadImageToQImage(name.toStdString(), img, &err) && !img.isNull()) {
		// Scale to icon size for snappy scrolling
		QPixmap pm = QPixmap::fromImage(img.scaled(_list->iconSize(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
		_thumbCache.insert(name, pm);
		return {pm};
	}

	// Fallback file icon
	return style()->standardIcon(QStyle::SP_FileIcon);
}

static QIcon makeEmptySlotIcon(const QSize& size)
{
	QPixmap pm(size);
	pm.fill(Qt::transparent);

	QPainter p(&pm);
	p.setRenderHint(QPainter::Antialiasing);

	// Border
	QPen pen(QColor(180, 180, 180));
	pen.setWidth(2);
	p.setPen(pen);
	p.setBrush(Qt::NoBrush);
	p.drawRect(pm.rect().adjusted(1, 1, -2, -2));

	// Subtle X
	QPen xPen(QColor(180, 180, 180, 180)); // slightly transparent gray
	xPen.setWidth(2);
	p.setPen(xPen);

	const int pad = 6; // padding inside the square so X isn't edge-to-edge
	QPoint topLeft(pad, pad);
	QPoint bottomRight(size.width() - pad, size.height() - pad);
	QPoint topRight(bottomRight.x(), topLeft.y());
	QPoint bottomLeft(topLeft.x(), bottomRight.y());

	p.drawLine(topLeft, bottomRight);
	p.drawLine(topRight, bottomLeft);

	p.end();

	return {pm};
}


void ImagePickerDialog::rebuildList()
{
	_list->clear();

	// Add unset option first, if enabled
	if (_allowUnset) {
		auto unsetIcon = makeEmptySlotIcon(_list->iconSize());
		auto* unsetItem = new QListWidgetItem(unsetIcon, "<None>");
		unsetItem->setData(Qt::UserRole, QString()); // empty filename
		unsetItem->setToolTip("Remove image / no image selected");
		_list->addItem(unsetItem);

		if (_selected.isEmpty()) {
			_list->setCurrentItem(unsetItem);
		}
	}

	const auto fl = _filterText.trimmed().toLower();
	for (const auto& name : _allFiles) {
		if (!fl.isEmpty() && !name.toLower().contains(fl))
			continue;

		auto icon = iconFor(name);
		auto* it = new QListWidgetItem(icon, name);
		it->setData(Qt::UserRole, name);
		it->setToolTip(name);
		_list->addItem(it);

		if (!_selected.isEmpty() && name == _selected) {
			_list->setCurrentItem(it);
		}
	}
}