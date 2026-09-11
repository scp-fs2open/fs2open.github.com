#pragma once

#include <QLayout>

namespace fso::fred {

/**
 * @brief A layout that arranges widgets left-to-right, wrapping to new rows as needed.
 *
 * Implements hasHeightForWidth() so the containing QVBoxLayout correctly resizes
 * the parent widget vertically when items wrap onto additional rows.
 */
class FlowLayout : public QLayout {
public:
	explicit FlowLayout(QWidget* parent = nullptr, int hSpacing = 4, int vSpacing = 2);
	~FlowLayout() override;

	void addItem(QLayoutItem* item) override;
	int horizontalSpacing() const { return m_hSpace; }
	int verticalSpacing() const { return m_vSpace; }
	Qt::Orientations expandingDirections() const override { return {}; }
	bool hasHeightForWidth() const override { return true; }
	int heightForWidth(int width) const override;
	int count() const override { return m_items.size(); }
	QLayoutItem* itemAt(int index) const override;
	QSize minimumSize() const override;
	void setGeometry(const QRect& rect) override;
	QSize sizeHint() const override;
	QLayoutItem* takeAt(int index) override;

private:
	int doLayout(const QRect& rect, bool testOnly) const;

	QList<QLayoutItem*> m_items;
	int m_hSpace;
	int m_vSpace;
};

} // namespace fso::fred
