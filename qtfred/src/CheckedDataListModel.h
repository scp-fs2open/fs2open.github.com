#ifndef CHECKEDDATALISTMODEL_H
#define CHECKEDDATALISTMODEL_H

#include <QAbstractListModel>
#include "globalincs/vmallocator.h"

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
	                _internalData()
	        {}
	        RowData(const QString &text, const T &internalData, bool checked) :
		        _text(text),
		        _internalData(internalData),
		        _checked(checked)
		{}
		const QString _text;
		const T _internalData;
		bool _checked;
	};

	CheckedDataListModel(QObject *parent = nullptr) = delete;

	template<typename InputIterator>
	CheckedDataListModel(InputIterator first, InputIterator last,
						 RowData (*translatorFn)(InputIterator &it),
						 QObject *parent = nullptr) :
		QAbstractListModel(parent)
	{
		for ( auto it = first; it != last; ++it ) {
			items.emplace_back(translatorFn(it));
			if (items.back()._text == "")
				items.pop_back();
		}
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

	SCP_vector<T> getCheckedData() const {
	        SCP_vector<T> ret;
		for ( auto& itm: items )
		        if (itm._checked)
			        ret.emplace_back(itm._internalData);
		return ret;
	}
private:
	SCP_vector<RowData> items;
};
}
}
#endif // CHECKEDDATALISTMODEL_H
