#pragma once

#include <QDialog>
#include <QTimer>

#include <mission/dialogs/MusicPlayerDialogModel.h>
#include <ui/FredView.h>

namespace fso::fred::dialogs {

namespace Ui {
class MusicPlayerDialog;
}

class MusicPlayerDialog final : public QDialog {
	Q_OBJECT
  public:
	MusicPlayerDialog(FredView* parent, EditorViewport* viewport);
	~MusicPlayerDialog() override;

	void accept() override; // not used
	void reject() override; // ensure we stop playback on close

  protected:
	void closeEvent(QCloseEvent* /*e*/) override;

  private slots:
	void on_musicList_itemSelectionChanged();
	void on_playButton_clicked();
	void on_stopButton_clicked();
	void on_nextButton_clicked();
	void on_prevButton_clicked();
	void on_autoplayCheck_toggled(bool on);
	void on_musicTblButton_clicked();

	// timer
	void onTick();

  private: // NOLINT(readability-redundant-access-specifiers)
	void populateList();
	void syncSelectionToModel();
	void syncButtonsEnabled();

	EditorViewport* _viewport = nullptr;
	std::unique_ptr<Ui::MusicPlayerDialog> ui;
	std::unique_ptr<MusicPlayerDialogModel> _model;
	QTimer _timer;
};

} // namespace fso::fred::dialogs
