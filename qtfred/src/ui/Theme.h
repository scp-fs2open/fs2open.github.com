#pragma once

#include <QApplication>
#include <QColor>
#include <QPalette>

namespace fso::fred {

// Fusion draws button gradients that can't be removed via QPalette alone.
// These QSS snippets restore a flat, square button style for each mode.
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
)";

inline void applyEditorTheme(bool darkMode) {
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

} // namespace fso::fred
