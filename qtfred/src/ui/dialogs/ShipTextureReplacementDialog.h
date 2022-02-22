#pragma once

#include <QtWidgets/QDialog>

#include <mission/dialogs/ShipTextureReplacementDialogModel.h>

namespace fso {
	namespace fred {
		namespace dialogs {

			namespace Ui {
				class ShipTextureReplacementDialog;
			}

			class ShipTextureReplacementDialog : public QDialog {
				
				Q_OBJECT

			public:
				explicit ShipTextureReplacementDialog(QDialog* parent, EditorViewport* viewport, bool);
				~ShipTextureReplacementDialog() override;
			};
		}
	}
}