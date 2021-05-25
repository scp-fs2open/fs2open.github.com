#ifndef CHECKEDDATALISTMODEL_H
#define CHECKEDDATALISTMODEL_H

#include <QAbstractListModel>
#include "globalincs/vmallocator.h"
#include <QBrush>

namespace fso {
namespace fred {

template<typename T>
class CheckedDataListModel : public QAbstractListModel
{
// no Q_OBJECT
public:
	struct RowData {
		explicit RowData() :
			_text(""),
			_internalData(),
			_checked(),
			_color()
		{}

		RowData(const QString &text, T &internalData, bool checked, Qt::GlobalColor bgColor = Qt::color0) :
			_text(text),
			_internalData(std::move(internalData)),
			_checked(checked),
			_color(bgColor)
		{}

		RowData(RowData&& move) = default;
		RowData& operator=(RowData&& move) = default;

		const QString _text;
		T _internalData;
		bool _checked;
		Qt::GlobalColor _color;
	};

	CheckedDataListModel(QObject *parent = nullptr) = delete;

	template<typename Container, typename InputIterator>
	CheckedDataListModel(const Container &c,
						 RowData (*translatorFn)(InputIterator &it),
						 QObject *parent = nullptr, size_t reserve = 0) :
		CheckedDataListModel(c.cbegin(), c.cend(), translatorFn, parent, reserve ? reserve : c.size())
	{}

	template<typename InputIterator>
	CheckedDataListModel(InputIterator first, InputIterator last,
						 RowData (*translatorFn)(InputIterator &it),
						 QObject *parent = nullptr, size_t reserve = 30) :
		QAbstractListModel(parent)
	{
		items.reserve(reserve);
		for ( auto it = first; it != last; ++it ) {
			items.emplace_back(translatorFn(it));
			if (items.back()._text.isEmpty())
				items.pop_back();
		}
		items.shrink_to_fit();
	}

	int rowCount(const QModelIndex &parent = QModelIndex()) const override {
		// For list models only the root node (an invalid parent) should return the list's size. For all
		// other (valid) parents, rowCount() should return 0 so that it does not become a tree model.
		if (parent.isValid())
			return 0;

		return items.size();
	}

	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override {
		if (!index.isValid())
			return QVariant();

		auto srow = static_cast<size_t>(index.row());
		if (srow >= items.size())
			return QVariant();

		if (role == Qt::DisplayRole)
			return items[srow]._text;
		else if (role == Qt::CheckStateRole)
			return items[srow]._checked ? Qt::Checked : Qt::Unchecked;
		else if (items[srow]._color != Qt::color0 && role == Qt::ForegroundRole)
			return QBrush(items[srow]._color);
		else
			return QVariant();
	}

	bool setData(const QModelIndex &index, const QVariant &value,
				 int role = Qt::EditRole) override {
		if(!index.isValid() || role != Qt::CheckStateRole)
			return false;
		auto srow = static_cast<size_t>(index.row());
		if (srow >= items.size())
			return false;

		if (data(index, role) != value) {
			items[srow]._checked = value == Qt::Checked;
			dataChanged(index, index, QVector<int>() << role);
			return true;
		} else
			return false;
	}

	Qt::ItemFlags flags(const QModelIndex& index) const override {
		Qt::ItemFlags defaultFlags = QAbstractListModel::flags(index);
		if (index.isValid())
			return defaultFlags | Qt::ItemIsUserCheckable;
		else
			return defaultFlags;
	}

	SCP_vector<const T*> getCheckedData() const {
		SCP_vector<const T*> ret;
		ret.reserve(items.size());
		for ( auto& itm: items )
			if (itm._checked)
				ret.push_back(&itm._internalData);
		return ret;
	}

	template<class... Args>
	inline void addRow(Args&&... args) {
		beginInsertRows(QModelIndex(), static_cast<int>(items.size()), static_cast<int>(items.size() + 1));
		items.emplace_back(std::forward<Args>(args)...);
		endInsertRows();
	}

	inline bool contains(const QString &text) const {
		return std::find_if(items.cbegin(), items.cend(),
							[&](const RowData &row){return row._text == text;})
				!= items.cend();
	}

private:
	SCP_vector<RowData> items;
};
}
}
#endif // CHECKEDDATALISTMODEL_H
