#pragma once

#include <QApplication>
#include <QColor>
#include <QPalette>

namespace fso::fred {

inline void applyEditorTheme(bool darkMode) {
	if (darkMode) {
		QPalette p;
		p.setColor(QPalette::Window,          QColor(53, 53, 53));
		p.setColor(QPalette::WindowText,      Qt::white);
		p.setColor(QPalette::Base,            QColor(25, 25, 25));
		p.setColor(QPalette::AlternateBase,   QColor(53, 53, 53));
		p.setColor(QPalette::ToolTipBase,     Qt::white);
		p.setColor(QPalette::ToolTipText,     Qt::white);
		p.setColor(QPalette::Text,            Qt::white);
		p.setColor(QPalette::Button,          QColor(53, 53, 53));
		p.setColor(QPalette::ButtonText,      Qt::white);
		p.setColor(QPalette::BrightText,      Qt::red);
		p.setColor(QPalette::Link,            QColor(42, 130, 218));
		p.setColor(QPalette::Highlight,       QColor(42, 130, 218));
		p.setColor(QPalette::HighlightedText, Qt::black);
		p.setColor(QPalette::Disabled, QPalette::Text,       QColor(127, 127, 127));
		p.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(127, 127, 127));
		qApp->setPalette(p);
	} else {
		qApp->setPalette(QPalette()); // reset to Fusion defaults
	}
}

} // namespace fso::fred
