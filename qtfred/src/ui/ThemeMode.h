#pragma once

namespace fso::fred {

// How the editor picks between the light and dark palette. System follows the OS
// light/dark setting and re-applies whenever it changes.
//
// Kept apart from Theme.h so the mission layer can store the preference without
// pulling QtWidgets into EditorViewport.h.
enum class ThemeMode { System, Light, Dark };

} // namespace fso::fred
