#include "PlayerOrdersDialog.h"

#include "ui_PlayerOrdersDialog.h"
#include <ui/util/SignalBlockers.h>
#include "mission/util.h"
#include <QCloseEvent>
namespace fso {
	namespace fred {
		namespace dialogs {
	PlayerOrdersDialog::PlayerOrdersDialog(QDialog* parent, EditorViewport* viewport, bool editMultiple)
			: QDialog(parent), ui(new Ui::PlayerOrdersDialog()),
			  _model(new PlayerOrdersDialogModel(this, viewport, editMultiple)),
				_viewport(viewport)
			{

				ui->setupUi(this);
				connect(this, &QDialog::accepted, _model.get(), &PlayerOrdersDialogModel::apply);
				connect(ui->cancelButton, &QPushButton::clicked, this, &PlayerOrdersDialog::rejectHandler);
				for (size_t i = 0; i < _model->getAcceptedOrders().size(); i++) {
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
			void PlayerOrdersDialog::closeEvent(QCloseEvent* e)
			{
				if (!rejectOrCloseHandler(this, _model.get(), _viewport)) {
					e->ignore();
				};
			}
			void PlayerOrdersDialog::rejectHandler()
			{
				this->close();
			}
			void PlayerOrdersDialog::updateUI()
			{
				util::SignalBlockers blockers(this);
				for (size_t i = 0; i < check_boxes.size(); i++) {
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