#pragma once
#include <QDialog>
#include <QStringList>

class QLineEdit;
class QListView;
class QPushButton;
class QStringListModel;
class QSortFilterProxyModel;

class SimpleListSelectDialog final : public QDialog {
	Q_OBJECT
  public:
	explicit SimpleListSelectDialog(const QStringList& items, QWidget* parent = nullptr);

	// Optional cosmetics
	void setTitle(const QString& title);
	void setPlaceholder(const QString& text);

	// Results (empty string / -1 if nothing selected or dialog canceled)
	QString selectedText() const;
	int selectedRow() const;

  private Q_SLOTS:
	void onFilterTextChanged(const QString& text);
	void onSelectionChanged();
	void onItemActivated(const QModelIndex& proxyIndex); // double-click/Enter
	void onFilterReturnPressed();

  private: // NOLINT(readability-redundant-access-specifiers)
	int proxyToSourceRow(const QModelIndex& proxyIndex) const;

	QLineEdit* m_filter = nullptr;
	QListView* m_list = nullptr;
	QPushButton* m_okBtn = nullptr;
	QPushButton* m_cancelBtn = nullptr;
	QStringListModel* m_sourceModel = nullptr;
	QSortFilterProxyModel* m_proxy = nullptr;
};
