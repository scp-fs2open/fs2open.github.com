#include <QtWidgets/QMessageBox>
#include "MissionCutscenesDialog.h"

#include "ui/util/SignalBlockers.h"
#include "mission/util.h"
#include "ui_MissionCutscenesDialog.h"

namespace fso::fred::dialogs {

MissionCutscenesDialog::MissionCutscenesDialog(QWidget* parent, EditorViewport* viewport)
	: QDialog(parent), SexpTreeEditorInterface({TreeFlags::LabeledRoot, TreeFlags::RootDeletable}),
	  ui(new Ui::MissionCutscenesDialog()), _model(new MissionCutscenesDialogModel(this, viewport)), _viewport(viewport)
{
	ui->setupUi(this);

	populateCutsceneCombos();

	ui->cutsceneEventTree->initializeEditor(viewport->editor, this);
	_model->setTreeControl(ui->cutsceneEventTree);

	ui->cutsceneFilename->setMaxLength(NAME_LENGTH - 1);

	ui->helpTextBox->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));

	connect(_model.get(), &MissionCutscenesDialogModel::modelChanged, this, &MissionCutscenesDialog::updateUi);

	_model->initializeData();

	load_tree();

	recreate_tree();
}

MissionCutscenesDialog::~MissionCutscenesDialog() = default;

void MissionCutscenesDialog::accept()
{
	// If apply() returns true, close the dialog
	if (_model->apply()) {
		QDialog::accept();
	}
	// else: validation failed, don’t close
}

void MissionCutscenesDialog::reject()
{
	// Asks the user if they want to save changes, if any
	// If they do, it runs _model->apply() and returns the success value
	// If they don't, it runs _model->reject() and returns true
	if (rejectOrCloseHandler(this, _model.get(), _viewport)) {
		QDialog::reject(); // actually close
	}
	// else: do nothing, don't close
}

void MissionCutscenesDialog::closeEvent(QCloseEvent* e)
{
	reject();
	e->ignore(); // Don't let the base class close the window
}

void MissionCutscenesDialog::updateUi()
{
	// Avoid infinite recursion by blocking signal calls caused by our changes here
	util::SignalBlockers blocker(this);

	if (!_model->isCurrentCutsceneValid()) {
		ui->cutsceneFilename->setText(QString());
		ui->cutsceneTypeCombo->setCurrentIndex(-1);

		ui->cutsceneTypeCombo->setEnabled(false);
		ui->cutsceneFilename->setEnabled(false);

		return;
	}

	auto& cutscene = _model->getCurrentCutscene();

	ui->cutsceneFilename->setText(QString::fromUtf8(cutscene.filename));
	ui->cutsceneTypeCombo->setCurrentIndex(cutscene.type);

	ui->cutsceneTypeCombo->setEnabled(true);
	ui->cutsceneFilename->setEnabled(true);

	setCutsceneTypeDescription();
}
void MissionCutscenesDialog::load_tree()
{
	ui->cutsceneEventTree->clear_tree();
	auto& cutscenes = _model->getCutscenes();
	for (auto& scene : cutscenes) {
		scene.formula = ui->cutsceneEventTree->load_sub_tree(scene.formula, true, "true");
	}
	ui->cutsceneEventTree->post_load();
}
void MissionCutscenesDialog::recreate_tree()
{
	ui->cutsceneEventTree->clear();
	const auto& cutscenes = _model->getCutscenes();
	for (const auto& scene : cutscenes) {
		if (!_model->isCutsceneVisible(scene)) {
			continue;
		}

		auto h = ui->cutsceneEventTree->insert(scene.filename);
		h->setData(0, sexp_tree::FormulaDataRole, scene.formula);
		ui->cutsceneEventTree->add_sub_tree(scene.formula, h);
	}

	_model->setCurrentCutscene(-1);
}
void MissionCutscenesDialog::createNewCutscene()
{
	auto& scene = _model->createNewCutscene();

	auto h = ui->cutsceneEventTree->insert(scene.filename);

	ui->cutsceneEventTree->setCurrentItemIndex(-1);
	ui->cutsceneEventTree->add_operator("true", h);
	auto index = scene.formula = ui->cutsceneEventTree->getCurrentItemIndex();
	h->setData(0, sexp_tree::FormulaDataRole, index);

	ui->cutsceneEventTree->setCurrentItem(h);
}
void MissionCutscenesDialog::changeCutsceneCategory(int type)
{
	if (_model->isCurrentCutsceneValid()) {
		_model->setCurrentCutsceneType(type);
		recreate_tree();
	}
}

void MissionCutscenesDialog::populateCutsceneCombos()
{
	ui->displayTypeCombo->clear();
	ui->cutsceneTypeCombo->clear();

	for (auto& item : CutsceneMenuData) {
		ui->displayTypeCombo->addItem(QString::fromStdString(item.label), item.value);
		ui->cutsceneTypeCombo->addItem(QString::fromStdString(item.label), item.value);
	}

	ui->displayTypeCombo->setCurrentIndex(_model->getSelectedCutsceneType());
	setCutsceneTypeDescription();
}

void MissionCutscenesDialog::setCutsceneTypeDescription()
{
	auto index = _model->getCutsceneType();
	if (index < 0 || index >= Num_movie_types) {
		ui->cutsceneTypeDescription->setText(QString());
		return;
	}

	ui->cutsceneTypeDescription->setText(QString::fromStdString(CutsceneMenuData[index].desc));
}

void MissionCutscenesDialog::on_okAndCancelButtons_accepted()
{
	accept();
}

void MissionCutscenesDialog::on_okAndCancelButtons_rejected()
{
	reject();
}

void MissionCutscenesDialog::on_displayTypeCombo_currentIndexChanged(int index)
{
	_model->setCutsceneType(index);
	setCutsceneTypeDescription();
	recreate_tree();
}

void MissionCutscenesDialog::on_cutsceneTypeCombo_currentIndexChanged(int index)
{
	changeCutsceneCategory(index);
}

void MissionCutscenesDialog::on_cutsceneFilename_textChanged(const QString& text)
{
	if (_model->isCurrentCutsceneValid()) {
		_model->setCurrentCutsceneFilename(text.toUtf8().constData());

		auto item = ui->cutsceneEventTree->currentItem();
		while (item->parent() != nullptr) {
			item = item->parent();
		}

		item->setText(0, text);
	}
}

void MissionCutscenesDialog::on_newCutsceneBtn_clicked()
{
	createNewCutscene();
}

void MissionCutscenesDialog::on_cutsceneEventTree_selectedRootChanged(int formula)
{
	auto& cutscenes = _model->getCutscenes();
	for (size_t i = 0; i < cutscenes.size(); ++i) {
		if (cutscenes[i].formula == formula) {
			_model->setCurrentCutscene(static_cast<int>(i));
			break;
		}
	}
}

void MissionCutscenesDialog::on_cutsceneEventTree_rootNodeDeleted(int node)
{
	_model->deleteCutscene(node);
}

void MissionCutscenesDialog::on_cutsceneEventTree_rootNodeFormulaChanged(int old, int node)
{
	_model->changeFormula(old, node);
}

void MissionCutscenesDialog::on_cutsceneEventTree_helpChanged(const QString& help)
{
	ui->helpTextBox->setPlainText(help);
}

} // namespace fso::fred::dialogs
