#pragma once

#include <QDialog>
#include <QDoubleSpinBox>

#include "globalincs/pstypes.h"

namespace fso::fred::dialogs {

class BriefingEditorDialogModel;

class IconCoordinatesDialog : public QDialog {
	Q_OBJECT

public:
	IconCoordinatesDialog(QWidget* parent, BriefingEditorDialogModel* model);

private slots:
	void onSpinBoxEditingFinished();
	void onCancelClicked();

private: // NOLINT(readability-redundant-access-specifiers)
	void setupUi();
	void populateFromCurrentIcon();
	void applyCurrentInputsToIcon();

	BriefingEditorDialogModel* _model;

	QDoubleSpinBox* _posX;
	QDoubleSpinBox* _posY;
	QDoubleSpinBox* _posZ;

	vec3d _initialIconPos {};
	bool _populatingUi = false;
};

} // namespace fso::fred::dialogs
