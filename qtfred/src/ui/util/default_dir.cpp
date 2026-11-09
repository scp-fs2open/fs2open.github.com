#include "default_dir.h"

#include <cfile/cfilesystem.h>

#include <QDir>
#include <QFileInfo>
#include <QSettings>

namespace fso::fred::util {

QString fredDefaultDir(int cfType)
{
	SCP_string path;
	// CF_LOCATION_ROOT_GAME is required so we get the game install directory
	// rather than the user-profile directory (AppData/Roaming/...).
	// User-dir mod roots are registered before game-dir roots, so omitting
	// CF_LOCATION_ROOT_GAME would return the wrong location.
	cf_create_default_path_string(path, cfType, nullptr,
		CF_LOCATION_TYPE_PRIMARY_MOD | CF_LOCATION_ROOT_GAME);

	QDir dir(QString::fromStdString(path));
	while (!dir.exists()) {
		if (!dir.cdUp())
			break;
	}
	return dir.absolutePath();
}

QString getLastDir(const QString& settingsKey, int cfType)
{
	return getLastDir(settingsKey, fredDefaultDir(cfType));
}

QString getLastDir(const QString& settingsKey, const QString& defaultDir)
{
	QSettings settings("QtFRED", "QtFRED");
	return settings.value(settingsKey, defaultDir).toString();
}

void saveLastDir(const QString& settingsKey, const QString& selectedPath)
{
	QSettings settings("QtFRED", "QtFRED");
	const QFileInfo fi(selectedPath);
	// For a directory (e.g. from QFileDialog::getExistingDirectory) store it
	// directly; for a file path store its parent directory.
	settings.setValue(settingsKey, fi.isDir() ? fi.absoluteFilePath() : fi.absolutePath());
}

} // namespace fso::fred::util
