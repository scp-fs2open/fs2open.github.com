#ifndef CHECKEDDATALISTMODEL_H
#define CHECKEDDATALISTMODEL_H

#include <QAbstractListModel>
#include "globalincs/vmallocator.h"
#include <QBrush>
#include "osapi/dialogs.h"

namespace fso {
namespace fred {

/**
 * @brief A model for use with a QListView with checkboxes
 * @details It can associate (and, for complex types, manage) user data,
 *   iterate over the data and its check state, collect checked data
 *   and dynamically append rows
 * @param T type of the associated/managed data
 */
template<typename T>
class CheckedDataListModel : public QAbstractListModel
{
// no Q_OBJECT
	/**
	 * @brief What gets displayed in a row, sans associated/managed data
	 */
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

	/**
	 * @brief Stores data of a simple type, to associate with each row
	 */
	template<class U = T, class Enabler = void>
	class RowData : public BaseRowData{
		U _managedData;
	public:
		RowData(const QString &text, const U &managedData, bool checked, Qt::GlobalColor bgColor = Qt::color0) :
			BaseRowData(text, checked, bgColor),
			_managedData(managedData)
		{}

		explicit RowData(RowData&& move) noexcept = default;
		RowData& operator=(RowData&& move) = delete;

		inline U& managedData() {return _managedData;}
		inline const U& managedData() const {return _managedData;}
	};

	/**
	 * @brief Takes ownership of a pointer to complex data, to associate with each row
	 */
	template<class U>
	class RowData<U, typename std::enable_if<std::is_class<U>::value>::type> : public BaseRowData{
		std::unique_ptr<U> _managedData;
	public:
		RowData(const QString &text, U* managedData, bool checked, Qt::GlobalColor bgColor = Qt::color0) :
			BaseRowData(text, checked, bgColor),
			_managedData(std::move(managedData))
		{}

		explicit RowData(RowData&& move) noexcept = default;
		RowData& operator=(RowData&& move) = delete;

		inline U& managedData() {return *_managedData;}
		inline const U& managedData() const {return *_managedData;}
	};

	SCP_vector<RowData<T>> items;

public:
	CheckedDataListModel(QObject *parent) = delete;

	/**
	 * @brief Create a model from a range
	 * @param first Begin of user data
	 * @param last End of user data
	 * @param emplacerFn Callback to perform any necessary conversions or discarding. Should use initRow()
	 * @param parent Parent for Qt-style memory management
	 * @param reserve Expected count of items, for optimisation
	 */
	template<typename InputIterator>
	CheckedDataListModel(InputIterator first, InputIterator last,
						 void (*emplacerFn)(const InputIterator &it, CheckedDataListModel<T> &model),
						 QObject *parent = nullptr, size_t reserve = 30 /*optimizing assumption*/) :
		QAbstractListModel(parent)
	{
		items.reserve(reserve);
		for ( auto it = first; it != last; ++it )
			emplacerFn(it, *this);
		items.shrink_to_fit();
	}

	/**
	 * @brief Create a model from an iterable Container. See above.
	 */
	template<typename Container>
	CheckedDataListModel(const Container &c,
						 void (*emplacerFn)(const typename Container::const_iterator &it, CheckedDataListModel<T> &model),
						 QObject *parent = nullptr, size_t reserve = 0) :
		CheckedDataListModel(c.cbegin(), c.cend(), emplacerFn, parent, reserve ? reserve : c.size())
	{}

	int rowCount(const QModelIndex &parent = QModelIndex()) const override {
		// For list models only the root node (an invalid parent) should return the list's size. For all
		// other (valid) parents, rowCount() should return 0 so that it does not become a tree model.
		if (parent.isValid())
			return 0;

		return static_cast<int>(items.size());
	}

	/**
	 * @brief Read-only of base data.
	 * @note Mostly for QListView use. See managedData() for user data.
	 * @param index The row
	 * @param role The property
	 * @return The requested property wrapped in a QVariant, or QVariant()
	 */
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

	/**
	 * @brief managedData
	 * @param The row
	 * @return Pointer to requested user data, or nullptr if it doesn't exist
	 */
	inline const T* managedData(const QModelIndex &index) const {
		if (! index.isValid()) {
			Warning(LOCATION,"Tried to retrieve invalid/empty index from CheckedDataListModel %s \nValidate item selection before calling managedData() if possible.", qPrintable(this->objectName()));
			return nullptr;
		}
		auto row = static_cast<size_t>(index.row());
		if (row >= items.size())
			return nullptr;
		return &items[row].managedData();
	}

	/**
	 * @brief See const version above
	 */
	inline T* managedData(const QModelIndex &index) {
		//re-use the const implementation for non-const this
		return const_cast<T*>(
					const_cast<const CheckedDataListModel<T>*>(this)
					->managedData(index));
	}

	// make checkboxes editable in QListView
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

	// enable showing checkboxes in QListView
	Qt::ItemFlags flags(const QModelIndex& index) const override {
		Qt::ItemFlags defaultFlags = QAbstractListModel::flags(index);
		if (index.isValid())
			return defaultFlags | Qt::ItemIsUserCheckable;
		else
			return defaultFlags;
	}

private: // private shared implementation for the following two methods
	template<typename V>
	inline void collectCheckedDataImpl(SCP_unordered_set<V*> &set) {
		for (auto& item : items)
			if (item._checked)
				set.insert(&item.managedData());
	}

public:
	/**
	 * @brief Collect all user data associated with a checked row.
	 * @param expected_checked Expected count of items, for optimisation
	 * @return A set of pointers to all checked user data
	 * @warning Pointers will be invalidated when new rows are added, or the model is destroyed.
	 */
	inline SCP_unordered_set<T*> collectCheckedData(size_t expected_checked = 0) {
		if (expected_checked == 0)
			// optimize for half of items checked
			expected_checked = items.size() / 2;
		SCP_unordered_set<T*> ret{expected_checked};
		collectCheckedDataImpl(ret);
		return ret;
	}

	/**
	 * @brief See non-const version above
	 */
	inline SCP_unordered_set<const T*> collectCheckedData(size_t expected_checked = 0) const {
		if (expected_checked == 0)
			// optimize for half of items checked
			expected_checked = items.size() / 2;
		SCP_unordered_set<const T*> ret{expected_checked};
		const_cast<CheckedDataListModel<T>*>(this)->collectCheckedDataImpl(ret);
		return ret;
	}

	/**
	 * @brief Adds a row during initialisation
	 * @note Call this from emplacerFn with arguments from the RowData constructor
	 * @param text Text to be displayed
	 * @param managedData Simple user data, or pointer / smart pointer to user data
	 * @param checked Initial checked status
	 * @param bgColor Color of the text
	 */
	template<class... Args>
	inline void initRow(Args&&... args) {
		items.emplace_back(std::forward<Args>(args)...);
	}

	/**
	 * @brief Adds a row during when the model is connected to a view
	 * @note Call this while the model is displayed with arguments from the RowData constructor
	 * @param text Text to be displayed
	 * @param managedData Simple user data, or pointer / smart pointer to user data
	 * @param checked Initial checked status
	 * @param bgColor Color of the text
	 */
	template<class... Args>
	inline void addRow(Args&&... args) {
		beginInsertRows(QModelIndex(), static_cast<int>(items.size()), static_cast<int>(items.size() + 1));
		initRow(std::forward<Args>(args)...);
		endInsertRows();
	}

	/**
	 * @brief contains
	 * @param text of the row to check
	 * @return If a row with the given text exists
	 * @note Use this before adding a row if you need a distinct model
	 */
	inline bool contains(const QString &text) const {
		return std::find_if(items.cbegin(), items.cend(),
							[&](const RowData<> &row){return row._text == text;})
				!= items.cend();
	}


	// Rest is iterators and functions suitable for range-based for loop,
	// as of https://stackoverflow.com/questions/8054273
	struct Iterator
	{
		using iterator_delegate = typename SCP_vector<RowData<T>>::iterator;

		using iterator_category = std::forward_iterator_tag;
		using difference_type   = typename SCP_vector<RowData<T>>::iterator::difference_type;
		using value_type        = std::pair<T, bool>;
		using pointer           = std::pair<const T*, bool>;
		using reference         = std::pair<T&, bool>;

		Iterator(iterator_delegate it) : _it(it) {}

		reference operator*() const { return reference(_it->managedData(), _it->_checked); }
		pointer operator->() { return pointer(&_it->managedData(), _it->_checked); }
		Iterator& operator++() { _it++; return *this; }
		Iterator operator++(int) { Iterator tmp = *this; ++(*this); return tmp; }
		bool operator== (const Iterator &other) { return _it == other._it;}
		bool operator!= (const Iterator &other) { return _it != other._it;}

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

		reference operator*() const { return reference(_it->managedData(), _it->_checked); }
		pointer operator->() { return pointer(&_it->managedData(), _it->_checked); }
		ConstIterator& operator++() { _it++; return *this; }
		ConstIterator operator++(int) { ConstIterator tmp = *this; ++(*this); return tmp; }
		bool operator== (const ConstIterator &other) { return _it == other._it;}
		bool operator!= (const ConstIterator &other) { return _it != other._it;}

	private:
		iterator_delegate _it;
	};

	inline ConstIterator begin() const { return ConstIterator(items.cbegin()); }
	inline ConstIterator end() const { return ConstIterator(items.cend()); }
	inline ConstIterator cbegin() const { return begin(); }
	inline ConstIterator cend() const { return end(); }
};
} // namespace fred
} // namespace fso
#endif // CHECKEDDATALISTMODEL_H
