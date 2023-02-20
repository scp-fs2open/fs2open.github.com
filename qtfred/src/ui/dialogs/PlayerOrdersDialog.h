#pragma once

#include <QtWidgets/QDialog>

#include <mission/dialogs/PlayerOrdersDialogModel.h>
#include <ui/widgets/ShipFlagCheckbox.h>
#include <ui/dialogs/ShipEditorDialog.h>

namespace fso {
	namespace fred {
		namespace dialogs {

			namespace Ui {
				class PlayerOrdersDialog;
			}
				class ShipEditorDialog;
			class PlayerOrdersDialog : public QDialog
			{
				Q_OBJECT

			public:
				explicit PlayerOrdersDialog(QDialog* parent, EditorViewport* viewport);
				~PlayerOrdersDialog() override;

			protected:
				void closeEvent(QCloseEvent*) override;
				void showEvent(QShowEvent*) override;

			private:
				std::unique_ptr<Ui::PlayerOrdersDialog> ui;
				std::unique_ptr<PlayerOrdersDialogModel> _model;
				EditorViewport* _viewport;
				ShipEditorDialog* parentDialog;

				void updateUI();

				SCP_vector<ShipFlagCheckbox*> check_boxes;
			};
		}
	}
}