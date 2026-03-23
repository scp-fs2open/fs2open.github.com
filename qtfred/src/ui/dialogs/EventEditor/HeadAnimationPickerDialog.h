#pragma once

#include <QDialog>
#include <QHash>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QTimer>
#include <QVector>

class HeadAnimationPickerDialog : public QDialog {
	Q_OBJECT

  public:
	explicit HeadAnimationPickerDialog(QWidget* parent = nullptr);

	void setHeadAnimationNames(const QStringList& names);
	void setInitialSelection(const QString& name);
	QString selectedFile() const
	{
		return _selected;
	}

  private slots:
	void onFilterTextChanged(const QString& text);
	void onItemActivated(QListWidgetItem* item);
	void onSelectionChanged();
	void onBrowse();
	void onOk();
	void onTick();

  private: // NOLINT(readability-redundant-access-specifiers)
	struct PreviewData {
		QString sourceName;
		int firstFrame{-1};
		int numFrames{1};
		int fps{15};
		QPixmap firstPixmap;
		QVector<QPixmap> frames; // all animation frames, loaded lazily on first preview
	};

	void rebuildList();
	PreviewData* ensurePreview(const QString& displayName);
	static void loadPreviewFrames(PreviewData* preview);
	void updatePreview();
	void setSelectedByName(const QString& name);
	static QString findPreviewSource(const QString& displayName);
	static QString normalizeHeadAniName(const QString& baseName);
	static QStringList detectVariants(const QString& baseName);
	void rebuildVariantButtons(const QString& baseName);

	QLineEdit* _filterEdit{nullptr};
	QListWidget* _list{nullptr};
	QLabel* _previewLabel{nullptr};
	QWidget* _variantButtonWidget{nullptr};
	QHBoxLayout* _variantButtonLayout{nullptr};
	QVector<QPushButton*> _variantButtons;
	QPushButton* _browseBtn{nullptr};
	QPushButton* _okBtn{nullptr};
	QPushButton* _cancelBtn{nullptr};
	QTimer _tickTimer;

	QStringList _allNames;
	QString _filterText;
	QString _selected;
	QString _previewingName;

	QHash<QString, PreviewData> _cache;
	float _previewElapsedSeconds{0.0f};
};
