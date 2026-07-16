#pragma once

#include "ui/ThemeMode.h"

#include <QAbstractButton>
#include <QAction>
#include <QColor>
#include <QIcon>
#include <QPixmap>
#include <QString>
#include <QStyle>

namespace fso::fred {

// Apply the Fusion-compatible light or dark palette and button stylesheet.
// System resolves against the OS color scheme, and the theme re-applies on its own
// whenever the OS scheme changes while this mode is active.
void applyEditorTheme(ThemeMode mode);

// Read/write the theme mode preference. Both live here so the settings group and key
// stay in one place: the startup path applies the theme before EditorViewport exists.
ThemeMode readThemeModeSetting();
void writeThemeModeSetting(ThemeMode mode);

// Draw a palette-aware icon for a standard Qt pixmap using QPainter.
// Falls back to the style's own icon for unhandled StandardPixmap values.
QIcon makeThemedIcon(QStyle::StandardPixmap sp, const QColor& color, int size = 16);

// Bind a palette-aware icon to a button and refresh it when the theme changes.
void bindStandardIcon(QAbstractButton* btn, QStyle::StandardPixmap sp);

// Palette-aware icons drawn by QPainter that have no QStyle::StandardPixmap equivalent.
// The MoveTo* values are "jump to end" arrows: an arrow with a bar across the end it
// points toward (e.g. MoveToTop is an up arrow with a bar along the top).
enum class CustomIcon {
	MoveToTop,
	MoveToBottom,
	MoveToLeft,
	MoveToRight,
};

// Draw a palette-aware icon for a CustomIcon using QPainter.
QIcon makeThemedIcon(CustomIcon icon, const QColor& color, int size = 16);

// Bind a palette-aware CustomIcon to a button and refresh it when the theme changes.
void bindCustomIcon(QAbstractButton* btn, CustomIcon icon);

// Bind a theme-adaptive PNG icon to a toolbar action.
// Loads :/images/toolbar/<baseName>-dark.png or <baseName>-light.png based on
// the current palette, and refreshes automatically when the theme changes.
void bindThemeIcon(QAction* action, const QString& baseName);

// Overload for QAbstractButton (e.g. QToolButton in the transform bar).
void bindThemeIcon(QAbstractButton* btn, const QString& baseName);

// --- Icon colorization + shadow helpers (used by the sexp tree icons) ---

// Colorize a neutral master by multiplying `color` into its RGB while preserving
// the master's alpha. White areas become the full color, darker areas become a
// darker shade of it. Use for solid silhouettes that must take an arbitrary color
// (including black/white), e.g. the dot and chain icons.
QPixmap tintMultiply(const QPixmap& src, const QColor& color);

// Colorize a neutral master by screening `color` over its RGB while preserving the
// master's alpha. Light areas stay light (white stays white); dark areas take the
// color. Use for two-tone icons whose light areas must be preserved, e.g. the data
// document (black leaves it unchanged, red produces the "variable" look).
QPixmap tintScreen(const QPixmap& src, const QColor& color);

// Composite a subtle, uniform neutral drop shadow beneath an icon, returning a new
// pixmap of the same size. Applied to every sexp tree icon so shadow styling stays
// consistent and decoupled from the (tinted) artwork.
QPixmap applyIconShadow(const QPixmap& src);

} // namespace fso::fred
