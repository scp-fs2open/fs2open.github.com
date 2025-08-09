#include "weaponList.h"

namespace fso::fred {
weaponList::weaponList(QWidget* parent) : QListView(parent) {}

void weaponList::mousePressEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton) {
		dragStartPosition = event->pos();
	}
	QListView::mousePressEvent(event);
}
void weaponList::mouseMoveEvent(QMouseEvent* event)
{
	if (!(event->buttons() & Qt::LeftButton))
		return;
	if ((event->pos() - dragStartPosition).manhattanLength() < QApplication::startDragDistance())
		return;
	QModelIndex idx = currentIndex();
	if (!idx.isValid()) {
		return;
	}
	auto drag = new QDrag(this);
	QModelIndexList idxs;
	idxs.append(idx);
	QMimeData* mimeData = model()->mimeData(idxs);
	auto iconPixmap = new QPixmap();
	QPainter painter(iconPixmap);
	painter.setFont(QFont("Arial"));
	painter.drawText(QPoint(100, 100), model()->data(idx, Qt::DisplayRole).toString());
	drag->setPixmap(*iconPixmap);
	drag->setMimeData(mimeData);
	drag->exec();
}

WeaponModel::WeaponModel(int type)
{
	auto noWeapon = new WeaponItem(-1, "None");
	weapons.push_back(noWeapon);
	if (type == 0) {
		for (int i = 0; i < static_cast<int>(Weapon_info.size()); i++) {
			const auto& w = Weapon_info[i];
			if (w.subtype == WP_LASER || w.subtype == WP_BEAM) {
				if (!w.wi_flags[Weapon::Info_Flags::No_fred]) {
					auto newWeapon = new WeaponItem(i, w.name);
					weapons.push_back(newWeapon);
				}
			}
		}
	} else if (type == 1) {
		for (int i = 0; i < static_cast<int>(Weapon_info.size()); i++) {
			const auto& w = Weapon_info[i];
			if (w.subtype == WP_MISSILE) {
				if (!w.wi_flags[Weapon::Info_Flags::No_fred]) {
					auto newWeapon = new WeaponItem(i, w.name);
					weapons.push_back(newWeapon);
				}
			}
		}
	}
}
WeaponModel::~WeaponModel()
{
	for (auto pointer : weapons) {
		delete pointer;
	}
}
int WeaponModel::rowCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent);
	return static_cast<int>(weapons.size());
}
QVariant WeaponModel::data(const QModelIndex& index, int role) const
{
	if (role == Qt::DisplayRole) {
		const QString out = weapons[index.row()]->name;
		return out;
	}
	if (role == Qt::UserRole) {
		const int id = weapons[index.row()]->id;
		return id;
	}
	return {};
}
QMimeData* WeaponModel::mimeData(const QModelIndexList& indexes) const
{
	auto mimeData = new QMimeData();
	QByteArray encodedData;
	QDataStream stream(&encodedData, QIODevice::WriteOnly);
	for (auto& index : indexes) {
		if (index.isValid()) {
			int id = data(index, Qt::UserRole).toInt();
			stream << id;
		}
	}

	mimeData->setData("application/weaponid", encodedData);

	return mimeData;
}
WeaponItem::WeaponItem(const int inID, QString inName) : name(std::move(inName)), id(inID) {}
} // namespace fso::fred