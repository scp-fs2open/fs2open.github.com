#include "ShipTextureReplacementDialog.h"

#include "ui_ShipTextureReplacementDialog.h"
#include <ui/util/SignalBlockers.h>

#include <QCloseEvent>
namespace fso {
	namespace fred {
		namespace dialogs {

			//Model for list view, Should really have its own file but its not big enught for me to bother.
			MapModel::MapModel(ShipTextureReplacementDialogModel* data, QObject* parent)
				: QAbstractListModel(parent), _model(data)
			{
			}

			int MapModel::rowCount(const QModelIndex& /*parent*/) const
			{
				return _model->getSize();
			}

			QVariant MapModel::data(const QModelIndex& index, int role) const
			{
				if (role == Qt::DisplayRole) {
					QString out = _model->getDefaultName(index.row()).c_str();
					return out;
				}
				if (role == Qt::SizeHintRole) {
					QString out = _model->getDefaultName(index.row()).c_str();
					if (out.isEmpty()) {
						return (QSize(0, 0));
					}
				}
				return QVariant();
			}


			ShipTextureReplacementDialog::ShipTextureReplacementDialog(QDialog* parent, EditorViewport* viewport, bool multi)
				: QDialog(parent), ui(new Ui::ShipTextureReplacementDialog()), _model(new ShipTextureReplacementDialogModel(this, viewport, multi)),
				_viewport(viewport)
			{
				ui->setupUi(this);
				connect(this, &QDialog::accepted, _model.get(), &ShipTextureReplacementDialogModel::apply);
				connect(this, &QDialog::rejected, _model.get(), &ShipTextureReplacementDialogModel::reject);
				listmodel = new MapModel(_model.get(), this);
				ui->TexturesList->setModel(listmodel);
				QItemSelectionModel* selectionModel = ui->TexturesList->selectionModel();
				connect(selectionModel, &QItemSelectionModel::selectionChanged,
					this, &ShipTextureReplacementDialog::updateUIFull);
				QModelIndex index = listmodel->index(0);
				ui->TexturesList->setCurrentIndex(index);

				connect(ui->newTextureLineEdit, &QLineEdit::editingFinished, this, &ShipTextureReplacementDialog::setMain);
				connect(ui->MiscTextureLineEdit, &QLineEdit::editingFinished, this, &ShipTextureReplacementDialog::setMisc);
				connect(ui->GlowTextureLineEdit, &QLineEdit::editingFinished, this, &ShipTextureReplacementDialog::setGlow);
				connect(ui->ShineTextureLineEdit, &QLineEdit::editingFinished, this, &ShipTextureReplacementDialog::setShine);
				connect(ui->NormalTextureLineEdit, &QLineEdit::editingFinished, this, &ShipTextureReplacementDialog::setNormal);
				connect(ui->HeightTextureLineEdit, &QLineEdit::editingFinished, this, &ShipTextureReplacementDialog::setHeight);
				connect(ui->AmbiantTextureLineEdit, &QLineEdit::editingFinished, this, &ShipTextureReplacementDialog::setAo);
				connect(ui->ReflectTextureLineEdit, &QLineEdit::editingFinished, this, &ShipTextureReplacementDialog::setReflect);


				connect(ui->useMiscTexturecheckbox, &QCheckBox::toggled, this, &ShipTextureReplacementDialog::setMiscReplace);
				connect(ui->useGlowTexturecheckbox, &QCheckBox::toggled, this, &ShipTextureReplacementDialog::setGlowReplace);
				connect(ui->useShineTexturecheckbox, &QCheckBox::toggled, this, &ShipTextureReplacementDialog::setShineReplace);
				connect(ui->useNormalTexturecheckbox, &QCheckBox::toggled, this, &ShipTextureReplacementDialog::setNormalReplace);
				connect(ui->useHeightTexturecheckbox, &QCheckBox::toggled, this, &ShipTextureReplacementDialog::setHeightReplace);
				connect(ui->useAmbiantTexturecheckbox, &QCheckBox::toggled, this, &ShipTextureReplacementDialog::setAoReplace);
				connect(ui->useReflectTexturecheckbox, &QCheckBox::toggled, this, &ShipTextureReplacementDialog::setReflectReplace);

				connect(ui->inheritMiscTextureCheckbox, &QCheckBox::toggled, this, &ShipTextureReplacementDialog::setMiscInherit);
				connect(ui->inheritGlowTextureCheckbox, &QCheckBox::toggled, this, &ShipTextureReplacementDialog::setGlowInherit);
				connect(ui->inheritShineTextureCheckbox, &QCheckBox::toggled, this, &ShipTextureReplacementDialog::setShineInherit);
				connect(ui->inheritNormalTextureCheckbox, &QCheckBox::toggled, this, &ShipTextureReplacementDialog::setNormalInherit);
				connect(ui->inheritHeightTextureCheckbox, &QCheckBox::toggled, this, &ShipTextureReplacementDialog::setHeightInherit);
				connect(ui->inheritAmbiantTextureCheckbox, &QCheckBox::toggled, this, &ShipTextureReplacementDialog::setAoInherit);
				connect(ui->inheritReflectTextureCheckbox, &QCheckBox::toggled, this, &ShipTextureReplacementDialog::setReflectInherit);

				connect(_model.get(), &AbstractDialogModel::modelChanged, this, &ShipTextureReplacementDialog::updateUI);

				// Resize the dialog to the minimum size
				resize(QDialog::sizeHint());
			}
			ShipTextureReplacementDialog::~ShipTextureReplacementDialog() = default;

			void ShipTextureReplacementDialog::closeEvent(QCloseEvent* event)
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

			void ShipTextureReplacementDialog::updateUI()
			{
				util::SignalBlockers blockers(this);
				ui->newTextureLineEdit->setText(_model->getMap(row, "main").c_str());
				bool replace;
				bool inherit;
				ui->useMiscTexturecheckbox->setChecked(replace = _model->getReplace(row)["misc"]);
				if (replace) {
					ui->inheritMiscTextureCheckbox->setChecked(inherit = _model->getInherit(row)["misc"]);
					ui->inheritMiscTextureCheckbox->setEnabled(replace);
					ui->MiscTextureLineEdit->setEnabled(!inherit);
					if (!inherit) {
						ui->MiscTextureLineEdit->setText(_model->getMap(row, "misc").c_str());
					}
				}
				else {
					ui->inheritMiscTextureCheckbox->setDisabled(true);
					ui->MiscTextureLineEdit->setDisabled(true);
				}

				ui->useShineTexturecheckbox->setChecked(replace = _model->getReplace(row)["shine"]);
				if (replace) {
					ui->inheritShineTextureCheckbox->setChecked(inherit = _model->getInherit(row)["shine"]);
					ui->inheritShineTextureCheckbox->setEnabled(replace);

					ui->ShineTextureLineEdit->setEnabled(!inherit);
					if (!inherit) {
						ui->ShineTextureLineEdit->setText(_model->getMap(row, "shine").c_str());
					}
				}
				else {
					ui->inheritShineTextureCheckbox->setDisabled(true);
					ui->ShineTextureLineEdit->setDisabled(true);
				}

				ui->useGlowTexturecheckbox->setChecked(replace = _model->getReplace(row)["glow"]);
				if (replace) {
					ui->inheritGlowTextureCheckbox->setEnabled(replace);
					ui->inheritGlowTextureCheckbox->setChecked(inherit = _model->getInherit(row)["glow"]);
					ui->GlowTextureLineEdit->setEnabled(!inherit);
					if (!inherit) {
						ui->GlowTextureLineEdit->setText(_model->getMap(row, "glow").c_str());
					}
				}
				else {
					ui->inheritGlowTextureCheckbox->setDisabled(true);
					ui->GlowTextureLineEdit->setDisabled(true);
				}

				ui->useNormalTexturecheckbox->setChecked(replace = _model->getReplace(row)["normal"]);
				if (replace) {
					ui->inheritNormalTextureCheckbox->setChecked(inherit = _model->getInherit(row)["normal"]);
					ui->inheritNormalTextureCheckbox->setEnabled(replace);
					ui->NormalTextureLineEdit->setEnabled(!inherit);
					if (!inherit) {
						ui->NormalTextureLineEdit->setText(_model->getMap(row, "normal").c_str());
					}
				}
				else {
					ui->inheritNormalTextureCheckbox->setDisabled(true);
					ui->NormalTextureLineEdit->setDisabled(true);
				}

				ui->useHeightTexturecheckbox->setChecked(replace = _model->getReplace(row)["height"]);
				if (replace) {
					ui->inheritHeightTextureCheckbox->setChecked(inherit = _model->getInherit(row)["height"]);
					ui->inheritHeightTextureCheckbox->setEnabled(replace);
					ui->HeightTextureLineEdit->setEnabled(!inherit);
					if (!inherit) {
						ui->HeightTextureLineEdit->setText(_model->getMap(row, "height").c_str());
					}
				}
				else {
					ui->inheritHeightTextureCheckbox->setDisabled(true);
					ui->HeightTextureLineEdit->setDisabled(true);
				}

				ui->useAmbiantTexturecheckbox->setChecked(replace = _model->getReplace(row)["ao"]);
				if (replace) {
					ui->inheritAmbiantTextureCheckbox->setChecked(inherit = _model->getInherit(row)["ao"]);
					ui->AmbiantTextureLineEdit->setEnabled(!inherit);
					ui->inheritAmbiantTextureCheckbox->setEnabled(replace);
					if (!inherit) {
						ui->AmbiantTextureLineEdit->setText(_model->getMap(row, "ao").c_str());
					}
				}
				else {
					ui->inheritAmbiantTextureCheckbox->setDisabled(true);
					ui->AmbiantTextureLineEdit->setDisabled(true);
				}

				ui->useReflectTexturecheckbox->setChecked(replace = _model->getReplace(row)["reflect"]);
				if (replace) {
					ui->inheritReflectTextureCheckbox->setChecked(inherit = _model->getInherit(row)["reflect"]);
					ui->ReflectTextureLineEdit->setEnabled(!inherit);
					ui->inheritReflectTextureCheckbox->setEnabled(replace);
					if (!inherit) {
						ui->ReflectTextureLineEdit->setText(_model->getMap(row, "reflect").c_str());
					}
				}
				else {
					ui->inheritReflectTextureCheckbox->setDisabled(true);
					ui->ReflectTextureLineEdit->setDisabled(true);
				}

			}
			void ShipTextureReplacementDialog::updateUIFull()
			{
				util::SignalBlockers blockers(this);
				const QModelIndex index = ui->TexturesList->selectionModel()->currentIndex();
				row = index.row();
				SCP_map<SCP_string, bool> subtypes = _model->getSubtypesForMap(row);
				bool hide = !(subtypes)["misc"];
				ui->inheritMiscTextureCheckbox->setHidden(hide);
				ui->MiscTextureLabel->setHidden(hide);
				ui->MiscTextureLineEdit->setHidden(hide);
				ui->useMiscTexturecheckbox->setHidden(hide);
				hide = !(subtypes)["shine"];
				ui->inheritShineTextureCheckbox->setHidden(hide);
				ui->ShineTextureLabel->setHidden(hide);
				ui->ShineTextureLineEdit->setHidden(hide);
				ui->useShineTexturecheckbox->setHidden(hide);
				hide = !(subtypes)["glow"];
				ui->inheritGlowTextureCheckbox->setHidden(hide);
				ui->GlowTextureLabel->setHidden(hide);
				ui->GlowTextureLineEdit->setHidden(hide);
				ui->useGlowTexturecheckbox->setHidden(hide);
				hide = !(subtypes)["normal"];
				ui->inheritNormalTextureCheckbox->setHidden(hide);
				ui->NormalTextureLabel->setHidden(hide);
				ui->NormalTextureLineEdit->setHidden(hide);
				ui->useNormalTexturecheckbox->setHidden(hide);
				hide = !(subtypes)["height"];
				ui->inheritHeightTextureCheckbox->setHidden(hide);
				ui->HeightTextureLabel->setHidden(hide);
				ui->HeightTextureLineEdit->setHidden(hide);
				ui->useHeightTexturecheckbox->setHidden(hide);
				hide = !(subtypes)["ao"];
				ui->inheritAmbiantTextureCheckbox->setHidden(hide);
				ui->AmbiantTextureLabel->setHidden(hide);
				ui->AmbiantTextureLineEdit->setHidden(hide);
				ui->useAmbiantTexturecheckbox->setHidden(hide);
				hide = !(subtypes)["reflect"];
				ui->inheritReflectTextureCheckbox->setHidden(hide);
				ui->ReflectTextureLabel->setHidden(hide);
				ui->ReflectTextureLineEdit->setHidden(hide);
				ui->useReflectTexturecheckbox->setHidden(hide);
				resize(QDialog::sizeHint());
				updateUI();
			}
			void ShipTextureReplacementDialog::setMain()
			{
				SCP_string newText;
				if (!ui->newTextureLineEdit->text().isEmpty()) {
					newText = ui->newTextureLineEdit->text().toStdString();
				}
				_model->setMap(row, "main", newText);
			}
			void ShipTextureReplacementDialog::setMisc()
			{
				SCP_string newText;
				if (!ui->MiscTextureLineEdit->text().isEmpty()) {
					 newText = ui->MiscTextureLineEdit->text().toStdString();
					_model->setMap(row, "misc", newText);
				}
			}
			void ShipTextureReplacementDialog::setGlow()
			{
				SCP_string newText;
				if (!ui->GlowTextureLineEdit->text().isEmpty()) {
					 newText = ui->GlowTextureLineEdit->text().toStdString();
					_model->setMap(row, "glow", newText);
				}
			}
			void ShipTextureReplacementDialog::setShine()
			{
				SCP_string newText;
				if (!ui->ShineTextureLineEdit->text().isEmpty()) {
					newText = ui->ShineTextureLineEdit->text().toStdString();
					_model->setMap(row, "shine", newText);
				}
			}
			void ShipTextureReplacementDialog::setNormal()
			{
				SCP_string newText;
				if (!ui->NormalTextureLineEdit->text().isEmpty()) {
					 newText = ui->NormalTextureLineEdit->text().toStdString();
					_model->setMap(row, "normal", newText);
				}
			}
			void ShipTextureReplacementDialog::setHeight()
			{
				SCP_string newText;
				if (!ui->HeightTextureLineEdit->text().isEmpty()) {
					 newText = ui->HeightTextureLineEdit->text().toStdString();
					_model->setMap(row, "height", newText);
				}
			}
			void ShipTextureReplacementDialog::setAo()
			{
				SCP_string newText;
				if (!ui->AmbiantTextureLineEdit->text().isEmpty()) {
					newText = ui->AmbiantTextureLineEdit->text().toStdString();
					_model->setMap(row, "ao", newText);
				}
			}
			void ShipTextureReplacementDialog::setReflect()
			{
				SCP_string newText;
				if (!ui->ReflectTextureLineEdit->text().isEmpty()) {
					newText = ui->ReflectTextureLineEdit->text().toStdString();
					_model->setMap(row, "reflect", newText);
				}
			}

			void ShipTextureReplacementDialog::setMiscInherit(const bool state)
			{
				_model->setInherit(row, "misc", state);
			}
			void ShipTextureReplacementDialog::setGlowInherit(const bool state)
			{
				_model->setInherit(row, "glow", state);
			}
			void ShipTextureReplacementDialog::setShineInherit(const bool state)
			{
				_model->setInherit(row, "shine", state);
			}
			void ShipTextureReplacementDialog::setNormalInherit(const bool state)
			{
				_model->setInherit(row, "normal", state);
			}
			void ShipTextureReplacementDialog::setHeightInherit(const bool state)
			{
				_model->setInherit(row, "height", state);
			}
			void ShipTextureReplacementDialog::setAoInherit(const bool state)
			{
				_model->setInherit(row, "ao", state);
			}
			void ShipTextureReplacementDialog::setReflectInherit(const bool state)
			{
				_model->setInherit(row, "reflect", state);
			}


			void ShipTextureReplacementDialog::setMiscReplace(const bool state)
			{
				_model->setReplace(row, "misc", state);
			}
			void ShipTextureReplacementDialog::setGlowReplace(const bool state)
			{
				_model->setReplace(row, "glow", state);
			}
			void ShipTextureReplacementDialog::setShineReplace(const bool state)
			{
				_model->setReplace(row, "shine", state);
			}
			void ShipTextureReplacementDialog::setNormalReplace(const bool state)
			{
				_model->setReplace(row, "normal", state);
			}
			void ShipTextureReplacementDialog::setHeightReplace(const bool state)
			{
				_model->setReplace(row, "height", state);
			}
			void ShipTextureReplacementDialog::setAoReplace(const bool state)
			{
				_model->setReplace(row, "ao", state);
			}
			void ShipTextureReplacementDialog::setReflectReplace(const bool state)
			{
				_model->setReplace(row, "reflect", state);
			}
		}
	}
}
