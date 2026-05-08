#pragma once

#include <QDialog>
#include <QDoubleSpinBox>

#include "globalincs/pstypes.h"

namespace fso::fred {
class BriefingMapWidget;
}

namespace fso::fred::dialogs {

class BriefingEditorDialogModel;

class CameraCoordinatesDialog : public QDialog {
	Q_OBJECT

public:
	CameraCoordinatesDialog(QWidget* parent,
		BriefingEditorDialogModel* model,
		fso::fred::BriefingMapWidget* mapWidget);

private slots:
	void onSpinBoxEditingFinished();
	void onCancelClicked();

private: // NOLINT(readability-redundant-access-specifiers)
	void setupUi();
	void populateFromCurrentCamera();
	void applyCurrentInputsToCamera();

	BriefingEditorDialogModel* _model;
	fso::fred::BriefingMapWidget* _mapWidget;

	QDoubleSpinBox* _posX;
	QDoubleSpinBox* _posY;
	QDoubleSpinBox* _posZ;
	QDoubleSpinBox* _heading;
	QDoubleSpinBox* _pitch;
	QDoubleSpinBox* _bank;

	vec3d _initialCameraPos {};
	matrix _initialCameraOrient {};
	bool _populatingUi = false;

};

} // namespace fso::fred::dialogs
