#pragma once

#include <cfile/cfile.h>
#include <QString>

namespace fso::fred::util {

/**
 * @brief Returns the best default directory for the given CF_TYPE_* constant.
 *
 * Prefers the active mod directory over the game root. If the ideal directory
 * does not yet exist (e.g. a newly created mod without that subdirectory),
 * walks up the path hierarchy until an existing parent is found.
 *
 * @param cfType  One of the CF_TYPE_* constants defined in cfile/cfile.h.
 * @return Absolute path to the best available directory.
 */
QString fredDefaultDir(int cfType);

/**
 * @brief Returns the last-used directory for a file dialog, persisted in QSettings.
 *
 * If no directory has been saved for @p settingsKey yet, falls back to
 * fredDefaultDir(cfType).
 *
 * @param settingsKey  QSettings key, e.g. "briefing/voiceFile".
 * @param cfType       CF_TYPE_* fallback if no saved value exists.
 */
QString getLastDir(const QString& settingsKey, int cfType);

/**
 * @brief Returns the last-used directory for a file dialog, persisted in QSettings.
 *
 * If no directory has been saved for @p settingsKey yet, falls back to @p defaultDir.
 * Use this overload when the appropriate fallback is not a CF_TYPE_* game directory
 * (e.g. QDir::homePath() for user-facing export dialogs).
 *
 * @param settingsKey  QSettings key, e.g. "voiceActingManager/exportScript".
 * @param defaultDir   Fallback directory if no saved value exists.
 */
QString getLastDir(const QString& settingsKey, const QString& defaultDir);

/**
 * @brief Saves the directory of @p selectedPath to QSettings under @p settingsKey.
 *
 * Call this after the user successfully picks a file so the next dialog opens
 * in the same directory.
 *
 * @param settingsKey   QSettings key, e.g. "briefing/voiceFile".
 * @param selectedPath  Full path to the selected file (or its parent directory).
 */
void saveLastDir(const QString& settingsKey, const QString& selectedPath);

} // namespace fso::fred::util
