#pragma once
#include <QDialog>
#include <QHash>
#include <QLineEdit>
#include <QListWidget>
#include <QPixmap>
#include <QPushButton>

class ImagePickerDialog : public QDialog {
	Q_OBJECT
  public:
	explicit ImagePickerDialog(QWidget* parent = nullptr);

	void setImageFilenames(const QStringList& filenames);
	void setInitialSelection(const QString& filename);
	QString selectedFile() const
	{
		return _selected;
	}

	void allowUnset(bool enable)
	{
		_allowUnset = enable;
	}

  private slots:
	void onFilterTextChanged(const QString& text);
	void onItemActivated(QListWidgetItem* item);
	void onOk();

  private: // NOLINT(readability-redundant-access-specifiers)
	void rebuildList();
	QIcon iconFor(const QString& name);

	QLineEdit* _filterEdit{nullptr};
	QListWidget* _list{nullptr};
	QPushButton* _okBtn{nullptr};
	QPushButton* _cancelBtn{nullptr};

	QStringList _allFiles;
	QString _filterText;
	QString _selected;

	QHash<QString, QPixmap> _thumbCache;

	bool _allowUnset{false};
};