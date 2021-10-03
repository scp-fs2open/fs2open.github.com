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
	struct BaseRowData {
		const QString _text;
		bool _checked;
		Qt::GlobalColor _color;

		BaseRowData(QString text, bool checked, Qt::GlobalColor bgColor) :
			_text(std::move(text)),
			_checked(checked),
			_color(bgColor)
		{}
	};
	template<class U = T, class Enabler = void>
	class RowData : public BaseRowData{
		U _internalData;
	public:
		RowData(const QString &text, const U &internalData, bool checked, Qt::GlobalColor bgColor = Qt::color0) :
			BaseRowData(text, checked, bgColor),
			_internalData(internalData)
		{}

		explicit RowData(RowData&& move) noexcept = default;
		RowData& operator=(RowData&& move) = delete;

		inline U& internalData() {return _internalData;}
	};

	template<class U>
	class RowData<U, typename std::enable_if<std::is_class<U>::value>::type> : public BaseRowData{
		std::unique_ptr<U> _internalData;
	public:
		RowData(const QString &text, U* internalData, bool checked, Qt::GlobalColor bgColor = Qt::color0) :
			BaseRowData(text, checked, bgColor),
			_internalData(std::move(internalData))
		{}

		explicit RowData(RowData&& move) noexcept = default;
		RowData& operator=(RowData&& move) = delete;

		inline U& internalData() {return *_internalData;}
	};

public:
	CheckedDataListModel(QObject *parent) = delete;

	template<typename Container>
	CheckedDataListModel(const Container &c,
						 void (*emplacerFn)(const typename Container::const_iterator &it, CheckedDataListModel<T> &model),
						 QObject *parent = nullptr, size_t reserve = 0) :
		CheckedDataListModel(c.cbegin(), c.cend(), emplacerFn, parent, reserve ? reserve : c.size())
	{}

	template<typename InputIterator>
	CheckedDataListModel(InputIterator first, InputIterator last,
						 void (*emplacerFn)(const InputIterator &it, CheckedDataListModel<T> &model),
						 QObject *parent = nullptr, size_t reserve = 30) :
		QAbstractListModel(parent)
	{
		items.reserve(reserve);
		for ( auto it = first; it != last; ++it )
			emplacerFn(it, *this);
		items.shrink_to_fit();
	}

	int rowCount(const QModelIndex &parent = QModelIndex()) const override {
		// For list models only the root node (an invalid parent) should return the list's size. For all
		// other (valid) parents, rowCount() should return 0 so that it does not become a tree model.
		if (parent.isValid())
			return 0;

		return static_cast<int>(items.size());
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

	inline T& internalData(const QModelIndex &index) {
		return items[static_cast<size_t>(index.row())].internalData();
	}

	inline const T& internalDataConst(const QModelIndex &index) const {
		return items[static_cast<size_t>(index.row())].internalData();
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
			if (items[srow]._checked) {
				checkeds.insert(&items[srow].internalData());
				checkedsConst.insert(&items[srow].internalData());
			} else {
				checkeds.erase(&items[srow].internalData());
				checkedsConst.erase(&items[srow].internalData());
			}
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

	inline const SCP_unordered_set<T*>& getCheckedData() {
		return checkeds;
	}

	inline const SCP_unordered_set<const T*>& getCheckedDataConst() const {
		return checkedsConst;
	}

	template<class... Args>
	inline void initRow(Args&&... args) {
		items.emplace_back(std::forward<Args>(args)...);
		if (items.back()._checked) {
			checkeds.insert(&items.back().internalData());
			checkedsConst.insert(&items.back().internalData());
		}
	}

	template<class... Args>
	inline void addRow(Args&&... args) {
		beginInsertRows(QModelIndex(), static_cast<int>(items.size()), static_cast<int>(items.size() + 1));
		initRow(std::forward<Args>(args)...);
		endInsertRows();
	}

	inline bool contains(const QString &text) const {
		return std::find_if(items.cbegin(), items.cend(),
							[&](const RowData<> &row){return row._text == text;})
				!= items.cend();
	}

private:
	SCP_vector<RowData<T>> items;
	SCP_unordered_set<T*> checkeds;
	SCP_unordered_set<const T*> checkedsConst;
};
}
}
#endif // CHECKEDDATALISTMODEL_H
