#pragma once
#include <weapon/weapon.h>

#include <QApplication>
#include <QDrag>
#include <QListView>
#include <QMimeData>
#include <QMouseEvent>
#include <QPainter>
namespace fso::fred {
struct WeaponItem {
	WeaponItem(const int id, QString name);
	const QString name;
	const int id;
};
class WeaponModel : public QAbstractListModel {
	Q_OBJECT
  public:
	WeaponModel(int type);
	~WeaponModel() override;
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
	void mousePressEvent(QMouseEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	QPoint dragStartPosition;

  private:
};
} // namespace fso::fred