#include "PlayerOrdersDialog.h"

#include "ui_PlayerOrdersDialog.h"
#include <ui/util/SignalBlockers.h>

#include <QCloseEvent>
namespace fso {
	namespace fred {
		namespace dialogs {
			PlayerOrdersDialog::PlayerOrdersDialog(QDialog* parent, EditorViewport* viewport)
				: QDialog(parent), ui(new Ui::PlayerOrdersDialog()),
				_viewport(viewport)
			{
				parentDialog = dynamic_cast<ShipEditorDialog*>(parent);
				Assert(parentDialog);
				_model = std::unique_ptr<PlayerOrdersDialogModel>(new PlayerOrdersDialogModel(this,
					viewport,
					parentDialog->getIfMultipleShips()));

				ui->setupUi(this);
				connect(this, &QDialog::accepted, _model.get(), &PlayerOrdersDialogModel::apply);
				connect(this, &QDialog::rejected, _model.get(), &PlayerOrdersDialogModel::reject);
				for (int i = 0; i < (int)_model->getAcceptedOrders().size(); i++) {
					//i == 0 check added to avoid culling first entry where getAcceptedOrders returns 0
					if (_model->getAcceptedOrders()[i] || i == 0) {
						check_boxes.push_back(new ShipFlagCheckbox(nullptr));
						check_boxes.back()->setText(_model->getOrderNames()[i].c_str());
						connect(check_boxes.back(),
							QOverload<int>::of(&ShipFlagCheckbox::stateChanged),
							[=](int value) {
							int state = value;
							if (state == Qt::Checked) {
								state = 1;
							}
							_model->setCurrentOrder(state, i);
							});
						ui->verticalLayout_2->addWidget(check_boxes.back());
					}
				}

				updateUI();
				// Resize the dialog to the minimum size
				resize(QDialog::sizeHint());
			}
			PlayerOrdersDialog::~PlayerOrdersDialog() = default;
			void PlayerOrdersDialog::closeEvent(QCloseEvent* event)
			{
				if (_model->query_modified()) {
					auto button = _viewport->dialogProvider->showButtonDialog(DialogType::Question, "Changes detected", "Do you want to keep your changes?",
						{ DialogButton::Yes, DialogButton::No, DialogButton::Cancel });

					if (button == DialogButton::Cancel) {
						event->ignore();
						return;
					}

					if (button == DialogButton::Yes) {
						accept();
						return;
					}
				}

				QDialog::closeEvent(event);
			}
			void PlayerOrdersDialog::showEvent(QShowEvent* e)
			{
				_model->initialiseData(parentDialog->getIfMultipleShips());

				QDialog::showEvent(e);
			}
			void PlayerOrdersDialog::updateUI()
			{
				util::SignalBlockers blockers(this);
				for (int i = 0; i < (int)check_boxes.size(); i++) {
					int state = _model->getCurrentOrders()[i];
					switch (state) {
					case 0:
						state = Qt::Unchecked;
						break;
					case 1:
						state = Qt::Checked;
						break;
					case 2:
						state = Qt::PartiallyChecked;
					}
					if (state != check_boxes[i]->checkState()) {
						check_boxes[i]->setCheckState((Qt::CheckState)state);
					}
				}
			}
		}
	}
}