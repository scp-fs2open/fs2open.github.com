#pragma once

#include <QString>
#include <QVector>
#include <QWidget>
#include <QHash>
#include <string>
#include <vector>

class QLineEdit;
class QToolButton;
class QListView;
class QStandardItemModel;
class QSortFilterProxyModel;
class QStandardItem;

namespace fso::fred {

class FlagListWidget final : public QWidget {
	Q_OBJECT
	Q_PROPERTY(bool filterVisible READ filterVisible WRITE setFilterVisible)
	Q_PROPERTY(bool toolbarVisible READ toolbarVisible WRITE setToolbarVisible)

  public:
	explicit FlagListWidget(QWidget* parent = nullptr);
	~FlagListWidget() override;

	void setFlags(const QVector<std::pair<QString, int>>& flags);

	// Optionally set descriptions
	void setFlagDescriptions(const QVector<std::pair<QString, QString>>& descriptions);

	// Read back the entire list and their checked states
	QVector<std::pair<QString, int>> getFlags() const;

	// Optional UI controls
	void setFilterVisible(bool visible);
	bool filterVisible() const;

	void setToolbarVisible(bool visible);
	bool toolbarVisible() const;

	// Clear all items
	void clear();

  signals:
	// Emitted whenever a checkbox is toggled
	void flagToggled(const QString& name, int checked);
	// Emitted after any change that alters the entire set
	void flagsChanged(const QVector<std::pair<QString, int>>& flags);

  private slots:
	void onItemChanged(QStandardItem* item);
	void onFilterTextChanged(const QString& text);
	void onSelectAll();
	void onClearAll();

  private: // NOLINT(readability-redundant-access-specifiers)
	enum Roles : int {
		KeyRole = Qt::UserRole + 1
	};

	void buildUi();
	void connectSignals();
	void rebuildModel(const QVector<std::pair<QString, int>>& flags);
	void applyTooltipsToItems();
	QVector<std::pair<QString, int>> snapshot() const;

	QLineEdit* _filter = nullptr;
	QToolButton* _btnAll = nullptr;
	QToolButton* _btnNone = nullptr;
	QListView* _list = nullptr;
	QStandardItemModel* _model = nullptr;
	QSortFilterProxyModel* _proxy = nullptr;

	QHash<QString, QString> _descByName;

	bool _updating = false; // guards against emitting signals during programmatic changes
	bool _filterVisible = true;
	bool _toolbarVisible = true;
};

} // namespace fso::fred