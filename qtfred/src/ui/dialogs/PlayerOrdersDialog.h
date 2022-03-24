#pragma once

#include <QtWidgets/QDialog>

#include <mission/dialogs/PlayerOrdersDialogModel.h>
#include <ui/widgets/ShipFlagCheckbox.h>

namespace fso {
	namespace fred {
		namespace dialogs {

			namespace Ui {
				class PlayerOrdersDialog;
			}

			class PlayerOrdersDialog : public QDialog
			{
				Q_OBJECT

			public:
				explicit PlayerOrdersDialog(QDialog* parent, EditorViewport* viewport, bool);
				~PlayerOrdersDialog() override;

			protected:
				void closeEvent(QCloseEvent*) override;

			private:
				std::unique_ptr<Ui::PlayerOrdersDialog> ui;
				std::unique_ptr<PlayerOrdersDialogModel> _model;
				EditorViewport* _viewport;

				void updateUI();

				SCP_vector<ShipFlagCheckbox*> check_boxes;
			};
		}
	}
}