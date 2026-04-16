#include "ui/Theme.h"

#include <QApplication>
#include <QPainter>
#include <QPainterPath>
#include <QPalette>
#include <QPixmap>

namespace {

const char* const LIGHT_BUTTON_QSS = R"(
QPushButton {
    background-color: #e1e1e1;
    border: 1px solid #adadad;
    border-radius: 2px;
    padding: 3px 10px;
    min-height: 16px;
}
QPushButton:hover {
    background-color: #e5f1fb;
    border-color: #0078d7;
}
QPushButton:pressed {
    background-color: #cce4f7;
    border-color: #005499;
}
QPushButton:default {
    border-color: #0078d7;
}
QPushButton:disabled {
    background-color: #f0f0f0;
    color: #a0a0a0;
    border-color: #c0c0c0;
}
QToolButton {
    background-color: transparent;
    border: 1px solid transparent;
    border-radius: 2px;
    padding: 2px;
}
QToolButton:hover {
    background-color: #e5f1fb;
    border-color: #0078d7;
}
QToolButton:pressed,
QToolButton:checked {
    background-color: #cce4f7;
    border-color: #005499;
}
QToolButton:checked:hover {
    background-color: #d8ecf9;
    border-color: #0078d7;
}
QToolButton::menu-indicator {
    image: none;
}
)";

const char* const DARK_BUTTON_QSS = R"(
QPushButton {
    background-color: #353535;
    border: 1px solid #555555;
    border-radius: 2px;
    padding: 3px 10px;
    min-height: 16px;
}
QPushButton:hover {
    background-color: #454545;
    border-color: #888888;
}
QPushButton:pressed {
    background-color: #606060;
    border-color: #909090;
}
QPushButton:default {
    border-color: #2a82da;
}
QPushButton:disabled {
    background-color: #2a2a2a;
    color: #7f7f7f;
    border-color: #444444;
}
QToolButton {
    background-color: transparent;
    border: 1px solid transparent;
    border-radius: 2px;
    padding: 2px;
}
QToolButton:hover {
    background-color: #4a4a4a;
    border-color: #666666;
}
QToolButton:pressed,
QToolButton:checked {
    background-color: #606060;
    border-color: #909090;
}
QToolButton:checked:hover {
    background-color: #6a6a6a;
    border-color: #aaaaaa;
}
QToolButton::menu-indicator {
    image: none;
}
QMenu::separator {
    height: 1px;
    background: #8f8f8f;
    margin: 4px 8px;
}
)";

} // anonymous namespace

namespace fso::fred {

void applyEditorTheme(bool darkMode)
{
	if (darkMode) {
		QPalette p;
		p.setColor(QPalette::Window,          QColor(53, 53, 53));
		p.setColor(QPalette::WindowText,      Qt::white);
		p.setColor(QPalette::Base,            QColor(25, 25, 25));
		p.setColor(QPalette::AlternateBase,   QColor(53, 53, 53));
		p.setColor(QPalette::ToolTipBase,     QColor(53, 53, 53));
		p.setColor(QPalette::ToolTipText,     Qt::white);
		p.setColor(QPalette::Text,            Qt::white);
		p.setColor(QPalette::Button,          QColor(53, 53, 53));
		p.setColor(QPalette::ButtonText,      Qt::white);
		p.setColor(QPalette::BrightText,      Qt::red);
		p.setColor(QPalette::Link,            QColor(42, 130, 218));
		p.setColor(QPalette::Highlight,       QColor(42, 130, 218));
		p.setColor(QPalette::HighlightedText, Qt::black);
		// Mid-tone roles — Fusion uses these for scroll bar grooves, frame bevels, spin boxes.
		// Without them the roles inherit from the system (light-mode) palette, causing
		// light-gray artifacts on dark backgrounds.
		p.setColor(QPalette::Mid,       QColor( 45,  45,  45));
		p.setColor(QPalette::Midlight,  QColor( 65,  65,  65));
		p.setColor(QPalette::Dark,      QColor( 18,  18,  18));
		p.setColor(QPalette::Shadow,    QColor(  5,   5,   5));
		// Disabled roles — must set WindowText and Light or Fusion renders
		// disabled text with a bright shadow, making it look blurry
		p.setColor(QPalette::Disabled, QPalette::WindowText, QColor(127, 127, 127));
		p.setColor(QPalette::Disabled, QPalette::Text,       QColor(127, 127, 127));
		p.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(127, 127, 127));
		p.setColor(QPalette::Disabled, QPalette::Light,      QColor(53, 53, 53));
		qApp->setPalette(p);
		qApp->setStyleSheet(DARK_BUTTON_QSS);
	} else {
		// Explicit light palette matching Windows 10 native colors
		QPalette p;
		p.setColor(QPalette::Window,          QColor(240, 240, 240));
		p.setColor(QPalette::WindowText,      Qt::black);
		p.setColor(QPalette::Base,            Qt::white);
		p.setColor(QPalette::AlternateBase,   QColor(233, 231, 227));
		p.setColor(QPalette::ToolTipBase,     QColor(255, 255, 220));
		p.setColor(QPalette::ToolTipText,     Qt::black);
		p.setColor(QPalette::Text,            Qt::black);
		p.setColor(QPalette::Button,          QColor(225, 225, 225));
		p.setColor(QPalette::ButtonText,      Qt::black);
		p.setColor(QPalette::BrightText,      Qt::red);
		p.setColor(QPalette::Link,            QColor(0, 0, 255));
		p.setColor(QPalette::Highlight,       QColor(0, 120, 215));
		p.setColor(QPalette::HighlightedText, Qt::white);
		p.setColor(QPalette::Disabled, QPalette::Text,       QColor(160, 160, 160));
		p.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(160, 160, 160));
		qApp->setPalette(p);
		qApp->setStyleSheet(LIGHT_BUTTON_QSS);
	}
}

QIcon makeThemedIcon(QStyle::StandardPixmap sp, const QColor& color, int size)
{
	QPixmap pm(size, size);
	pm.fill(Qt::transparent);
	QPainter p(&pm);
	p.setRenderHint(QPainter::Antialiasing);
	p.setPen(Qt::NoPen);
	p.setBrush(color);

	const qreal m  = size * 0.15;
	const QRectF r(m, m, size - 2 * m, size - 2 * m);
	const QPointF c = r.center();

	// Arrow geometry helpers — shaft+head arrow as a single filled polygon (no seam)
	const qreal headLen  = r.width()  * 0.50; // how far the arrowhead extends along the axis
	const qreal shaftW   = r.width()  * 0.38; // width of the rectangular shaft (cross-axis)
	const qreal shaftOff = (r.width() - shaftW) / 2.0; // offset from edge to shaft side

	switch (sp) {
	case QStyle::SP_ArrowUp: {
		const qreal headY = r.top() + headLen;
		QPainterPath path;
		path.moveTo(c.x(),               r.top());   // tip
		path.lineTo(r.right(),           headY);     // head right corner
		path.lineTo(r.left() + shaftOff + shaftW, headY);     // shoulder right
		path.lineTo(r.left() + shaftOff + shaftW, r.bottom()); // shaft bottom right
		path.lineTo(r.left() + shaftOff,           r.bottom()); // shaft bottom left
		path.lineTo(r.left() + shaftOff,           headY);     // shoulder left
		path.lineTo(r.left(),            headY);     // head left corner
		path.closeSubpath();
		p.drawPath(path);
		break;
	}
	case QStyle::SP_ArrowDown: {
		const qreal headY = r.bottom() - headLen;
		QPainterPath path;
		path.moveTo(r.left() + shaftOff,           r.top());   // shaft top left
		path.lineTo(r.left() + shaftOff + shaftW, r.top());   // shaft top right
		path.lineTo(r.left() + shaftOff + shaftW, headY);     // shoulder right
		path.lineTo(r.right(),           headY);     // head right corner
		path.lineTo(c.x(),               r.bottom()); // tip
		path.lineTo(r.left(),            headY);     // head left corner
		path.lineTo(r.left() + shaftOff, headY);     // shoulder left
		path.closeSubpath();
		p.drawPath(path);
		break;
	}
	case QStyle::SP_ArrowLeft: {
		const qreal headX = r.left() + headLen;
		QPainterPath path;
		path.moveTo(r.left(),  c.y());               // tip
		path.lineTo(headX,     r.top());             // head top corner
		path.lineTo(headX,     r.top() + shaftOff); // shoulder top
		path.lineTo(r.right(), r.top() + shaftOff); // shaft right top
		path.lineTo(r.right(), r.top() + shaftOff + shaftW); // shaft right bottom
		path.lineTo(headX,     r.top() + shaftOff + shaftW); // shoulder bottom
		path.lineTo(headX,     r.bottom());          // head bottom corner
		path.closeSubpath();
		p.drawPath(path);
		break;
	}
	case QStyle::SP_ArrowRight: {
		const qreal headX = r.right() - headLen;
		QPainterPath path;
		path.moveTo(r.left(), r.top() + shaftOff);           // shaft left top
		path.lineTo(headX,    r.top() + shaftOff);           // shoulder top
		path.lineTo(headX,    r.top());                      // head top corner
		path.lineTo(r.right(), c.y());                       // tip
		path.lineTo(headX,    r.bottom());                   // head bottom corner
		path.lineTo(headX,    r.top() + shaftOff + shaftW); // shoulder bottom
		path.lineTo(r.left(), r.top() + shaftOff + shaftW); // shaft left bottom
		path.closeSubpath();
		p.drawPath(path);
		break;
	}
	case QStyle::SP_MediaPlay: {
		QPainterPath path;
		path.moveTo(r.left(),  r.top());
		path.lineTo(r.right(), c.y());
		path.lineTo(r.left(),  r.bottom());
		path.closeSubpath();
		p.drawPath(path);
		break;
	}
	case QStyle::SP_MediaStop:
		p.drawRect(r);
		break;
	case QStyle::SP_MediaSkipForward: {
		// triangle pointing right + vertical bar on the right
		const qreal barW = r.width() * 0.18;
		const qreal gap  = r.width() * 0.05;
		const qreal triR = r.right() - barW - gap;
		QPainterPath path;
		path.moveTo(r.left(), r.top());
		path.lineTo(triR,     c.y());
		path.lineTo(r.left(), r.bottom());
		path.closeSubpath();
		p.drawPath(path);
		p.drawRect(QRectF(r.right() - barW, r.top(), barW, r.height()));
		break;
	}
	case QStyle::SP_MediaSkipBackward: {
		// vertical bar on the left + triangle pointing left
		const qreal barW = r.width() * 0.18;
		const qreal gap  = r.width() * 0.05;
		const qreal triL = r.left() + barW + gap;
		p.drawRect(QRectF(r.left(), r.top(), barW, r.height()));
		QPainterPath path;
		path.moveTo(r.right(), r.top());
		path.lineTo(triL,      c.y());
		path.lineTo(r.right(), r.bottom());
		path.closeSubpath();
		p.drawPath(path);
		break;
	}
	default:
		// Fall back to the style's own icon for anything we haven't drawn
		return qApp->style()->standardIcon(sp);
	}

	return {pm};
}

void bindStandardIcon(QAbstractButton* btn, QStyle::StandardPixmap sp)
{
	auto refresh = [btn, sp]() {
		const QColor color = qApp->palette().color(QPalette::ButtonText);
		btn->setIcon(makeThemedIcon(sp, color));
	};
	refresh();
	QObject::connect(qApp, &QApplication::paletteChanged, btn, [refresh](const QPalette&) {
		refresh();
	});
}

void bindThemeIcon(QAction* action, const QString& baseName)
{
	auto refresh = [action, baseName]() {
		const bool dark = qApp->palette().color(QPalette::Window).lightness() < 128;
		const QString path = QStringLiteral(":/images/toolbar/") + baseName
		                   + (dark ? QStringLiteral("-dark") : QStringLiteral("-light"))
		                   + QStringLiteral(".png");
		action->setIcon(QIcon(path));
	};
	refresh();
	QObject::connect(qApp, &QApplication::paletteChanged, action, [refresh](const QPalette&) {
		refresh();
	});
}

} // namespace fso::fred
