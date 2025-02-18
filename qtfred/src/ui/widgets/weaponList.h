#pragma once
#include <QListView>
#include <QMouseEvent>
#include <QApplication>
#include <QDrag>
#include <QMimeData>
#include <QPainter>
#include <weapon/weapon.h>
namespace fso {
namespace fred {
struct WeaponItem {
	WeaponItem(const int id, const QString& name);
	const QString name;
	const int id;
};
class WeaponModel : public QAbstractListModel {
	Q_OBJECT
  public:
	WeaponModel(int type);
	~WeaponModel();
	int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	QMimeData* mimeData(const QModelIndexList& indexes) const override;
	QVector<WeaponItem*> weapons;
};
class weaponList : public QListView {
	Q_OBJECT
  public:
	weaponList(QWidget* parent);

  protected:
	void mousePressEvent(QMouseEvent* event);
	void mouseMoveEvent(QMouseEvent* event);
	QPoint dragStartPosition;

  private:
};
}
} // namespace fso