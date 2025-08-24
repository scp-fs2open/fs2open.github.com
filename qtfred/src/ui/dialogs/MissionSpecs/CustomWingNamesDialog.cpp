#include "CustomWingNamesDialog.h"

#include "ui_CustomWingNamesDialog.h"
#include <mission/util.h>

#include <ui/util/SignalBlockers.h>

namespace fso::fred::dialogs {

CustomWingNamesDialog::CustomWingNamesDialog(QWidget* parent, EditorViewport* viewport) :
	QDialog(parent), ui(new Ui::CustomWingNamesDialog()), _model(new CustomWingNamesDialogModel(this, viewport)),
	_viewport(viewport) {
    ui->setupUi(this);

	connect(_model.get(), &AbstractDialogModel::modelChanged, this, &CustomWingNamesDialog::updateUi);

	updateUi();
	
	// Resize the dialog to the minimum size
	resize(QDialog::sizeHint());
}

CustomWingNamesDialog::~CustomWingNamesDialog() = default;

void CustomWingNamesDialog::accept()
{
	// If apply() returns true, close the dialog
	if (_model->apply()) {
		QDialog::accept();
	}
	// else: validation failed, don’t close
}

void CustomWingNamesDialog::reject()
{
	// Custom reject or close logic because we need to handle talkback to Mission Specs
	if (_model->query_modified()) {
		auto button = _viewport->dialogProvider->showButtonDialog(fso::fred::DialogType::Question,
			"Changes detected",
			"Do you want to keep your changes?",
			{fso::fred::DialogButton::Yes, fso::fred::DialogButton::No, fso::fred::DialogButton::Cancel});

		if (button == fso::fred::DialogButton::Yes) {
			accept();
}
		if (button == fso::fred::DialogButton::No) {
			_model->reject();
			QDialog::reject(); // actually close
}
	} else {
		_model->reject();
		QDialog::reject();
	}
}

void CustomWingNamesDialog::closeEvent(QCloseEvent * e) {
	reject();
	e->ignore(); // Don't let the base class close the window
}

void CustomWingNamesDialog::setInitialStartingWings(const std::array<SCP_string, MAX_STARTING_WINGS>& startingWings) {
	_model->setInitialStartingWings(startingWings);
	updateUi();
}

void CustomWingNamesDialog::setInitialSquadronWings(const std::array<SCP_string, MAX_SQUADRON_WINGS>& squadronWings) {
	_model->setInitialSquadronWings(squadronWings);
	updateUi();
}

void CustomWingNamesDialog::setInitialTvTWings(const std::array<SCP_string, MAX_TVT_WINGS>& tvtWings) {
	_model->setInitialTvTWings(tvtWings);
	updateUi();
}

const std::array<SCP_string, MAX_STARTING_WINGS>& CustomWingNamesDialog::getStartingWings() const {
	return _model->getStartingWings();
}

const std::array<SCP_string, MAX_SQUADRON_WINGS>& CustomWingNamesDialog::getSquadronWings() const {
	return _model->getSquadronWings();
}

const std::array<SCP_string, MAX_TVT_WINGS>& CustomWingNamesDialog::getTvTWings() const {
	return _model->getTvTWings();
}

void CustomWingNamesDialog::updateUi() {
	util::SignalBlockers blockers(this);

	// Update starting wings
	ui->startingWing_1->setText(_model->getStartingWing(0).c_str());
	ui->startingWing_2->setText(_model->getStartingWing(1).c_str());
	ui->startingWing_3->setText(_model->getStartingWing(2).c_str());

	// Update squadron wings
	ui->squadronWing_1->setText(_model->getSquadronWing(0).c_str());
	ui->squadronWing_2->setText(_model->getSquadronWing(1).c_str());
	ui->squadronWing_3->setText(_model->getSquadronWing(2).c_str());
	ui->squadronWing_4->setText(_model->getSquadronWing(3).c_str());
	ui->squadronWing_5->setText(_model->getSquadronWing(4).c_str());

	// Update dogfight wings
	ui->dogfightWing_1->setText(_model->getTvTWing(0).c_str());
	ui->dogfightWing_2->setText(_model->getTvTWing(1).c_str());
}

void CustomWingNamesDialog::startingWingChanged(const QString & str, int index) {
	_model->setStartingWing(str.toUtf8().constData(), index);
}

void CustomWingNamesDialog::squadronWingChanged(const QString & str, int index) {
	_model->setSquadronWing(str.toUtf8().constData(), index);
}

void CustomWingNamesDialog::dogfightWingChanged(const QString & str, int index) {
	_model->setTvTWing(str.toUtf8().constData(), index);
}

void CustomWingNamesDialog::on_okAndCancelButtons_accepted()
{
	accept();
}

void CustomWingNamesDialog::on_okAndCancelButtons_rejected()
{
	reject();
}

void CustomWingNamesDialog::on_startingWing_1_textChanged(const QString & text)
{
	startingWingChanged(text, 0);
}

void CustomWingNamesDialog::on_startingWing_2_textChanged(const QString & text)
{
	startingWingChanged(text, 1);
}

void CustomWingNamesDialog::on_startingWing_3_textChanged(const QString & text)
{
	startingWingChanged(text, 2);
}

void CustomWingNamesDialog::on_squadronWing_1_textChanged(const QString & text)
{
	squadronWingChanged(text, 0);
}

void CustomWingNamesDialog::on_squadronWing_2_textChanged(const QString & text)
{
	squadronWingChanged(text, 1);
}

void CustomWingNamesDialog::on_squadronWing_3_textChanged(const QString & text)
{
	squadronWingChanged(text, 2);
}

void CustomWingNamesDialog::on_squadronWing_4_textChanged(const QString & text)
{
	squadronWingChanged(text, 3);
}

void CustomWingNamesDialog::on_squadronWing_5_textChanged(const QString & text)
{
	squadronWingChanged(text, 4);
}

void CustomWingNamesDialog::on_dogfightWing_1_textChanged(const QString & text)
{
	dogfightWingChanged(text, 0);
}

void CustomWingNamesDialog::on_dogfightWing_2_textChanged(const QString & text)
{
	dogfightWingChanged(text, 1);
}

} // namespace fso::fred::dialogs
