#pragma once

#include <QDialog>

#include <mission/dialogs/AsteroidEditorDialogModel.h>
#include <ui/FredView.h>

namespace fso {
namespace fred {
namespace dialogs {

namespace Ui {
class AsteroidEditorDialog;
}

class AsteroidEditorDialog : public QDialog
{
	Q_OBJECT
public:
	AsteroidEditorDialog(FredView* parent, EditorViewport* viewport);
	~AsteroidEditorDialog() override;

private:
	void done(int r) override;

	void toggleEnabled(bool enabled);
	void toggleInnerBoxEnabled(bool enabled);
	void toggleAsteroid(AsteroidEditorDialogModel::_roid_types colour, bool enabled);

	void asteroidNumberChanged(int num_asteroids);

	void setFieldActive();
	void setFieldPassive();
	void setGenreAsteroid();
	void setGenreDebris();

	void changedBoxTextIMinX(const QString &text);
	void changedBoxTextIMinY(const QString &text);
	void changedBoxTextIMinZ(const QString &text);
	void changedBoxTextIMaxX(const QString &text);
	void changedBoxTextIMaxY(const QString &text);
	void changedBoxTextIMaxZ(const QString &text);
	void changedBoxTextOMinX(const QString &text);
	void changedBoxTextOMinY(const QString &text);
	void changedBoxTextOMinZ(const QString &text);
	void changedBoxTextOMaxX(const QString &text);
	void changedBoxTextOMaxY(const QString &text);
	void changedBoxTextOMaxZ(const QString &text);
	QString & getBoxText(AsteroidEditorDialogModel::_box_line_edits type);

	void updateComboBox(int idx, int debris_type);
	void updateUI();

	EditorViewport* _viewport = nullptr;
	Editor* _editor = nullptr;

	std::unique_ptr<Ui::AsteroidEditorDialog> ui;
	std::unique_ptr<AsteroidEditorDialogModel> _model;

	QDoubleValidator _box_validator;
	QIntValidator _speed_validator;

	QList<QComboBox *> debrisComboBoxes;
};


} // namespace dialogs
} // namespace fred
} // namespace fso
