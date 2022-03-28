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
		inline const U& internalData() const {return _internalData;}
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
		inline const U& internalData() const {return *_internalData;}
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
			return {};

		auto srow = static_cast<size_t>(index.row());
		if (srow >= items.size())
			return {};

		if (role == Qt::DisplayRole)
			return items[srow]._text;
		else if (role == Qt::CheckStateRole)
			return items[srow]._checked ? Qt::Checked : Qt::Unchecked;
		else if (items[srow]._color != Qt::color0 && role == Qt::ForegroundRole)
			return QBrush(items[srow]._color);
		else
			return {};
	}

	inline const T* internalData(const QModelIndex &index) const {
		if (! index.isValid()) return nullptr;
		size_t row{static_cast<size_t>(index.row())};
		if (row >= items.size()) return nullptr;
		return &items[row].internalData();
	}

	inline T* internalData(const QModelIndex &index) {
		return const_cast<T*>(
					const_cast<const CheckedDataListModel<T>*>(this)
					->internalData(index));
	}

	bool setData(const QModelIndex &index, const QVariant &value,
				 int role = Qt::EditRole) override {
		if(!index.isValid() || role != Qt::CheckStateRole)
			return false;
		auto srow = static_cast<size_t>(index.row());
		if (srow >= items.size())
			return false;

		if (data(index, role) != value) {
			items[srow]._checked = (value == Qt::Checked);
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

private:
	template<typename V>
	inline void getCheckedData2(SCP_unordered_set<V*> &set) {
		for (auto& it : items)
			if (it._checked)
				set.insert(&it.internalData());
	}

public:
	inline SCP_unordered_set<T*> getCheckedData() {
		SCP_unordered_set<T*> ret{items.size() / 2};
		getCheckedData2(ret);
		return ret;
	}

	inline SCP_unordered_set<const T*> getCheckedData() const {
		SCP_unordered_set<const T*> ret{items.size() / 2};
		const_cast<CheckedDataListModel<T>*>(this)->getCheckedData2(ret);
		return ret;
	}

	struct Iterator
	{
		using iterator_delegate = typename SCP_vector<RowData<T>>::iterator;

		using iterator_category = std::forward_iterator_tag;
		using difference_type   = typename SCP_vector<RowData<T>>::iterator::difference_type;
		using value_type        = std::pair<T, bool>;
		using pointer           = std::pair<const T*, bool>;
		using reference         = std::pair<T&, bool>;

		Iterator(iterator_delegate it) : _it(it) {}

		reference operator*() const { return reference(_it->internalData(), _it->_checked); }
		pointer operator->() { return pointer(&_it->internalData(), _it->_checked); }
		Iterator& operator++() { _it++; return *this; }
		Iterator operator++(int) { Iterator tmp = *this; ++(*this); return tmp; }
		friend bool operator== (const Iterator &a, const Iterator &b) { return a._it == b._it;}
		friend bool operator!= (const Iterator &a, const Iterator &b) { return a._it != b._it;}

	private:
		iterator_delegate _it;
	};

	inline Iterator begin() { return Iterator(items.begin()); }
	inline Iterator end() { return Iterator(items.end()); }

	struct ConstIterator
	{
		using iterator_delegate = typename SCP_vector<RowData<T>>::const_iterator;

		using iterator_category = std::forward_iterator_tag;
		using difference_type   = typename SCP_vector<RowData<T>>::iterator::difference_type;
		using value_type        = std::pair<T, bool>;
		using pointer           = std::pair<const T*, bool>;
		using reference         = std::pair<const T&, bool>;

		ConstIterator(iterator_delegate it) : _it(it) {}

		reference operator*() const { return reference(_it->internalData(), _it->_checked); }
		pointer operator->() { return pointer(&_it->internalData(), _it->_checked); }
		ConstIterator& operator++() { _it++; return *this; }
		ConstIterator operator++(int) { ConstIterator tmp = *this; ++(*this); return tmp; }
		friend bool operator== (const ConstIterator &a, const ConstIterator &b) { return a._it == b._it;}
		friend bool operator!= (const ConstIterator &a, const ConstIterator &b) { return a._it != b._it;}

	private:
		iterator_delegate _it;
	};

	inline ConstIterator begin() const { return ConstIterator(items.cbegin()); }
	inline ConstIterator end() const { return ConstIterator(items.cend()); }
	inline ConstIterator cbegin() const { return begin(); }
	inline ConstIterator cend() const { return end(); }

	template<class... Args>
	inline void initRow(Args&&... args) {
		items.emplace_back(std::forward<Args>(args)...);
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
};
} // namespace fred
} // namespace fso
#endif // CHECKEDDATALISTMODEL_H
