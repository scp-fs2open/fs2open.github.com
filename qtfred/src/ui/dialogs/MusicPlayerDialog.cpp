#include <QMessageBox>
#include <QCloseEvent>
#include "MusicTBLViewer.h"
#include "MusicPlayerDialog.h"
#include "ui/util/SignalBlockers.h"
#include "ui_MusicPlayerDialog.h"

namespace fso::fred::dialogs {

MusicPlayerDialog::MusicPlayerDialog(FredView* parent, EditorViewport* viewport)
	: QDialog(parent), _viewport(viewport), ui(new Ui::MusicPlayerDialog()),
	  _model(new MusicPlayerDialogModel(this, viewport))
{
	setFocus();
	ui->setupUi(this);

	// build list
	_model->loadTracks();
	populateList();
	syncButtonsEnabled();

	// 200ms poll to mirror DoFrame() autoplay behavior
	_timer.setInterval(200);
	connect(&_timer, &QTimer::timeout, this, &MusicPlayerDialog::onTick);
	_timer.start();
}

MusicPlayerDialog::~MusicPlayerDialog() = default;

void MusicPlayerDialog::accept()
{
	// not used
}

void MusicPlayerDialog::reject()
{
	_model->stop();
	QDialog::reject();
}

void MusicPlayerDialog::closeEvent(QCloseEvent* /*e*/)
{
	reject();
}

// --- UI sync helpers ---

void MusicPlayerDialog::populateList()
{
	util::SignalBlockers block(this);
	ui->musicList->clear();
	QStringList items;
	for (const auto& track : _model->tracks()) {
		// Add items without extension
		items.append(QString::fromStdString(track));
	}
	ui->musicList->addItems(items);
}

void MusicPlayerDialog::syncSelectionToModel()
{
	// Map QListWidget selection to model row
	int row = -1;
	const auto selected = ui->musicList->selectedItems();
	if (!selected.isEmpty()) {
		row = ui->musicList->row(selected.front());
	}
	_model->setCurrentRow(row);
	syncButtonsEnabled();
}

void MusicPlayerDialog::syncButtonsEnabled()
{
	const bool hasSel = _model->currentRow() >= 0;
	ui->playButton->setEnabled(hasSel);
	ui->stopButton->setEnabled(_model->isPlaying());
	ui->nextButton->setEnabled(_model->currentRow() >= 0 && _model->currentRow() < static_cast<int>(_model->tracks().size()) - 1);
	ui->prevButton->setEnabled(_model->currentRow() > 0);
}

// --- Slots ---

void MusicPlayerDialog::on_musicList_itemSelectionChanged()
{
	syncSelectionToModel();
}

void MusicPlayerDialog::on_playButton_clicked()
{
	_model->play();
	syncButtonsEnabled();
}

void MusicPlayerDialog::on_stopButton_clicked()
{
	_model->stop();
	syncButtonsEnabled();
}

void MusicPlayerDialog::on_nextButton_clicked()
{
	// Mirror original: only restart playback if already playing
	const bool wasPlaying = _model->isPlaying();
	if (_model->selectNext() && wasPlaying) {
		_model->play();
	}
	// reflect selection in the list
	util::SignalBlockers block(this);
	ui->musicList->setCurrentRow(_model->currentRow());
	syncButtonsEnabled();
}

void MusicPlayerDialog::on_prevButton_clicked()
{
	const bool wasPlaying = _model->isPlaying();
	if (_model->selectPrev() && wasPlaying) {
		_model->play();
	}
	util::SignalBlockers block(this);
	ui->musicList->setCurrentRow(_model->currentRow());
	syncButtonsEnabled();
}

void MusicPlayerDialog::on_autoplayCheck_toggled(bool on)
{
	_model->setAutoplay(on);
}

void MusicPlayerDialog::on_musicTblButton_clicked()
{
	auto dialog = new MusicTBLViewer(this, _viewport);
	dialog->show();
}

void MusicPlayerDialog::onTick()
{
	const bool wasPlaying = _model->isPlaying();
	_model->tick();
	if (wasPlaying != _model->isPlaying()) {
		syncButtonsEnabled();
	}
	// If autoplay advanced selection, reflect it in the list
	if (ui->musicList->currentRow() != _model->currentRow()) {
		util::SignalBlockers block(this);
		ui->musicList->setCurrentRow(_model->currentRow());
		syncButtonsEnabled();
	}
}

} // namespace fso::fred::dialogs