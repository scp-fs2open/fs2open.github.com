#pragma once

#include <QAbstractButton>
#include <QAction>
#include <QColor>
#include <QIcon>
#include <QString>
#include <QStyle>

namespace fso::fred {

// Apply the Fusion-compatible light or dark palette and button stylesheet.
void applyEditorTheme(bool darkMode);

// Draw a palette-aware icon for a standard Qt pixmap using QPainter.
// Falls back to the style's own icon for unhandled StandardPixmap values.
QIcon makeThemedIcon(QStyle::StandardPixmap sp, const QColor& color, int size = 16);

// Bind a palette-aware icon to a button and refresh it when the theme changes.
void bindStandardIcon(QAbstractButton* btn, QStyle::StandardPixmap sp);

// Bind a theme-adaptive PNG icon to a toolbar action.
// Loads :/images/toolbar/<baseName>-dark.png or <baseName>-light.png based on
// the current palette, and refreshes automatically when the theme changes.
void bindThemeIcon(QAction* action, const QString& baseName);

// Overload for QAbstractButton (e.g. QToolButton in the transform bar).
void bindThemeIcon(QAbstractButton* btn, const QString& baseName);

} // namespace fso::fred
