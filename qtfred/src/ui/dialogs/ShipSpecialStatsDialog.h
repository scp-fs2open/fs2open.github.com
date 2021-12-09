#pragma once

#include <mission/dialogs/ShipSpecialStatsDialogModel.h>

#include <QtWidgets/QDialog>

namespace fso {
	namespace fred {
		namespace dialogs {

			namespace Ui {
				class ShipSpecialStatsDialog;
			}

			class ShipSpecialStatsDialog : public QDialog {
				Q_OBJECT

			public:
				explicit ShipSpecialStatsDialog(QWidget* parent, EditorViewport* viewport);
				~ShipSpecialStatsDialog() override;

			protected:
				void closeEvent(QCloseEvent*) override;

			private:
				std::unique_ptr<Ui::ShipSpecialStatsDialog> ui;
				std::unique_ptr<ShipSpecialStatsDialogModel> _model;
				EditorViewport* _viewport;

				void updateUI();
			};
		}
	}
}