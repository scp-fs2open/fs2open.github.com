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

public slots:
	void onCameraChanged(vec3d pos, matrix orient);

private slots:
	void onSpinBoxChanged();
	void onApplyClicked();

private:
	void setupUi();

	BriefingEditorDialogModel* _model;
	fso::fred::BriefingMapWidget* _mapWidget;

	QDoubleSpinBox* _posX;
	QDoubleSpinBox* _posY;
	QDoubleSpinBox* _posZ;
	QDoubleSpinBox* _heading;
	QDoubleSpinBox* _pitch;
	QDoubleSpinBox* _bank;

	bool _updatingFromCamera = false;
};

} // namespace fso::fred::dialogs
