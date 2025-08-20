#include "MissionEventsDialog.h"
#include "ui_MissionEventsDialog.h"
#include "ui/util/SignalBlockers.h"

#include "mission/util.h"

#include <sound/audiostr.h>
#include <localization/localize.h>

#include <QInputDialog>
#include <QMessageBox>
#include <QFileDialog>
#include <QDebug>
#include <QKeyEvent>
#include <mission/missionmessage.h>

namespace fso::fred::dialogs {

MissionEventsDialog::MissionEventsDialog(QWidget* parent, EditorViewport* viewport) :
	QDialog(parent),
	SexpTreeEditorInterface({ TreeFlags::LabeledRoot, TreeFlags::RootDeletable, TreeFlags::RootEditable }),
	  ui(new Ui::MissionEventsDialog()), _viewport(viewport)
{
	ui->setupUi(this);

	// Build the Qt adapter now that eventTree exists
	struct QtTreeOps final : IEventTreeOps {
		explicit QtTreeOps(sexp_tree& t) : tree(t) {}
		sexp_tree& tree;

		int load_sub_tree(int formula, bool allow_empty = false, const char* default_body = "do-nothing") override
		{
			return tree.load_sub_tree(formula, allow_empty, default_body);
		}

		void post_load() override
		{
			tree.post_load();
		}

		void add_sub_tree(const SCP_string& name, NodeImage image, int formula) override
		{
			auto h = tree.insert(name.c_str(), image);
			h->setData(0, sexp_tree::FormulaDataRole, formula);
			tree.add_sub_tree(formula, h);
		}

		QTreeWidgetItem* findRootByFormula(int formula)
		{
			const int n = tree.topLevelItemCount();
			for (int i = 0; i < n; ++i) {
				auto* it = tree.topLevelItem(i);
				if (it && it->data(0, sexp_tree::FormulaDataRole).toInt() == formula)
					return it;
			}
			return nullptr;
		}

		int build_default_root(const SCP_string& name, int after_root) override
		{
			QTreeWidgetItem* afterItem = (after_root >= 0) ? findRootByFormula(after_root) : nullptr;

			auto* root = tree.insert(name.c_str(), NodeImage::ROOT, /*parent*/ nullptr, afterItem);

			// Build default body: when -> true -> do-nothing
			tree.setCurrentItemIndex(-1);
			int whenIdx = tree.add_operator("when", root);
			root->setData(0, sexp_tree::FormulaDataRole, whenIdx);
			tree.add_operator("true");
			tree.setCurrentItemIndex(whenIdx);
			tree.add_operator("do-nothing");

			tree.clearSelection();
			root->setSelected(true);

			return root->data(0, sexp_tree::FormulaDataRole).toInt();
		}

		int save_tree(int root_formula) override
		{
			return tree.save_tree(root_formula);
		}

		void ensure_top_level_index(int root_formula, int desired_index) override
		{
			if (auto* item = findRootByFormula(root_formula)) {
				int cur = tree.indexOfTopLevelItem(item);
				if (cur != desired_index) {
					tree.takeTopLevelItem(cur);
					tree.insertTopLevelItem(desired_index, item);
				}
			}
		}

		void select_root(int root_formula) override
		{
			if (auto* item = findRootByFormula(root_formula))
				tree.setCurrentItem(item);
		}

		void clear() override
		{
			tree.clear();
		}

		void delete_event() override
		{
			// This is such an ugly hack but I don't want to rewrite sexp_tree just for this..
			auto item = tree.currentItem();
			while (item->parent() != nullptr) {
				item = item->parent();
			}
			tree.setCurrentItem(item);

			tree.deleteCurrentItem();
		}
	};

	_treeOps = std::make_unique<QtTreeOps>(QtTreeOps{*ui->eventTree});

	ui->eventTree->initializeEditor(viewport->editor, this);
	ui->eventTree->clear_tree();
	ui->eventTree->post_load();

	// Now construct the model with reference to tree ops
	_model = std::make_unique<MissionEventsDialogModel>(this, _viewport, *_treeOps);

	initMessageWidgets();

	initEventWidgets();
}

MissionEventsDialog::~MissionEventsDialog() = default;

void MissionEventsDialog::initEventWidgets() {
	initEventTeams();

	ui->miniHelpBox->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
	ui->helpBox->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));

	// connect the sexp tree stuff
	connect(ui->eventTree, &sexp_tree::modified, this, [this]() { _model->setModified(); });
	connect(ui->eventTree, &sexp_tree::rootNodeDeleted, this, &MissionEventsDialog::rootNodeDeleted);
	connect(ui->eventTree, &sexp_tree::rootNodeRenamed, this, &MissionEventsDialog::rootNodeRenamed);
	connect(ui->eventTree, &sexp_tree::rootNodeFormulaChanged, this, &MissionEventsDialog::rootNodeFormulaChanged);
	connect(ui->eventTree, &sexp_tree::miniHelpChanged, this, [this](const QString& help) { ui->miniHelpBox->setText(help); });
	connect(ui->eventTree, &sexp_tree::helpChanged, this, [this](const QString& help) { ui->helpBox->setPlainText(help); });
	connect(ui->eventTree, &sexp_tree::selectedRootChanged, this, [this](int formula) { MissionEventsDialog::rootNodeSelectedByFormula(formula); });

	_model->setCurrentlySelectedEvent(-1);

	updateEventUi();
}

void MissionEventsDialog::accept()
{
	// If apply() returns true, close the dialog
	if (_model->apply()) {
		QDialog::accept();
	}
	// else: validation failed, don’t close
}

void MissionEventsDialog::reject()
{
	// Asks the user if they want to save changes, if any
	// If they do, it runs _model->apply() and returns the success value
	// If they don't, it runs _model->reject() and returns true
	if (rejectOrCloseHandler(this, _model.get(), _viewport)) {
		QDialog::reject(); // actually close
	}
	// else: do nothing, don't close
}

void MissionEventsDialog::closeEvent(QCloseEvent* e)
{
	reject();
	e->ignore(); // Don't let the base class close the window
}

void MissionEventsDialog::initMessageWidgets() {
	initHeadCombo();
	initWaveFilenames();
	initPersonas();
	initMessageTeams();

	initMessageList();

	ui->messageName->setMaxLength(NAME_LENGTH - 1);

	if (auto* le = ui->aniCombo->lineEdit()) {
		connect(le, &QLineEdit::editingFinished, this, &MissionEventsDialog::on_aniCombo_editingFinished);
	}

	if (auto* le = ui->waveCombo->lineEdit()) {
		connect(le, &QLineEdit::editingFinished, this, &MissionEventsDialog::on_waveCombo_editingFinished);
	}

	updateMessageUi();
}

void MissionEventsDialog::rootNodeDeleted(int node) {
	_model->deleteRootNode(node);
}

void MissionEventsDialog::rootNodeRenamed(int node) {
	QTreeWidgetItem* item = nullptr;
	for (int i = 0; i < ui->eventTree->topLevelItemCount(); ++i) {
		auto* it = ui->eventTree->topLevelItem(i);
		if (it && it->data(0, sexp_tree::FormulaDataRole).toInt() == node) {
			item = it;
			break;
		}
	}
	if (!item)
		return;

	SCP_string newText = item->text(0).toUtf8().constData();

	_model->renameRootNode(node, newText);
}

void MissionEventsDialog::rootNodeFormulaChanged(int old, int node) {
	_model->changeRootNodeFormula(old, node);
}

void MissionEventsDialog::rootNodeSelectedByFormula(int formula) {
	_model->setCurrentlySelectedEventByFormula(formula);
	updateEventUi();
}

void MissionEventsDialog::initMessageList() {
	rebuildMessageList();

	if (_model->getMessageList().size() > 0) {
		_model->setCurrentlySelectedMessage(0);
	} else {
		_model->setCurrentlySelectedMessage(-1);
	}
}

void MissionEventsDialog::rebuildMessageList() {
	// Block signals so that the current item index isn't overwritten by this
	QSignalBlocker blocker(ui->messageList);

	ui->messageList->clear();
	for (auto& msg : _model->getMessageList()) {
		auto item = new QListWidgetItem(msg.name, ui->messageList);
		ui->messageList->addItem(item);
	}
}

void MissionEventsDialog::updateEventUi() {
	util::SignalBlockers blockers(this);

	if (!_model->eventIsValid()) {
		ui->repeatCountBox->setValue(1);
		ui->triggerCountBox->setValue(1);
		ui->intervalTimeBox->setValue(1);
		ui->chainDelayBox->setValue(0);
		ui->teamCombo->setCurrentIndex(0); // was MAX_TVT_TEAMS for none?
		ui->editDirectiveText->setText("");
		ui->editDirectiveKeypressText->setText("");

		ui->repeatCountBox->setEnabled(false);
		ui->triggerCountBox->setEnabled(false);
		ui->intervalTimeBox->setEnabled(false);
		ui->chainDelayBox->setEnabled(false);
		ui->teamCombo->setEnabled(false);
		ui->editDirectiveText->setEnabled(false);
		ui->editDirectiveKeypressText->setEnabled(false);
		return;
	}

	ui->teamCombo->setCurrentIndex(ui->teamCombo->findData(_model->getEventTeam()));

	ui->repeatCountBox->setValue(_model->getRepeatCount());
	ui->triggerCountBox->setValue(_model->getTriggerCount());
	ui->intervalTimeBox->setValue(_model->getIntervalTime());
	ui->scoreBox->setValue(_model->getEventScore());
	if (_model->getChained()) {
		ui->chainedCheckBox->setChecked(true);
		ui->chainDelayBox->setValue(_model->getChainDelay());
		ui->chainDelayBox->setEnabled(true);
	} else {
		ui->chainedCheckBox->setChecked(false);
		ui->chainDelayBox->setValue(0);
		ui->chainDelayBox->setEnabled(false);
	}

	ui->editDirectiveText->setText(QString::fromStdString(_model->getEventDirectiveText()));
	ui->editDirectiveKeypressText->setText(QString::fromStdString(_model->getEventDirectiveKeyText()));

	ui->repeatCountBox->setEnabled(true);
	ui->triggerCountBox->setEnabled(true);

	if ((_model->getRepeatCount() > 1) || (_model->getRepeatCount() < 0) ||
		(_model->getTriggerCount() > 1) || (_model->getTriggerCount() < 0)) {
		ui->intervalTimeBox->setEnabled(true);
	} else {
		ui->intervalTimeBox->setValue(_model->getIntervalTime());
		ui->intervalTimeBox->setEnabled(false);
	}

	ui->scoreBox->setEnabled(true);
	ui->chainedCheckBox->setEnabled(true);
	ui->editDirectiveText->setEnabled(true);
	ui->editDirectiveKeypressText->setEnabled(true);
	ui->teamCombo->setEnabled(_model->getMissionIsMultiTeam());

	// handle event log flags
	ui->checkLogTrue->setChecked(_model->getLogTrue());
	ui->checkLogFalse->setChecked(_model->getLogFalse());
	ui->checkLogPrevious->setChecked(_model->getLogLogPrevious());
	ui->checkLogAlwaysFalse->setChecked(_model->getLogAlwaysFalse());
	ui->checkLogFirstRepeat->setChecked(_model->getLogFirstRepeat());
	ui->checkLogLastRepeat->setChecked(_model->getLogLastRepeat());
	ui->checkLogFirstTrigger->setChecked(_model->getLogFirstTrigger());
	ui->checkLogLastTrigger->setChecked(_model->getLogLastTrigger());
}
void MissionEventsDialog::initHeadCombo() {
	auto list = _model->getHeadAniList();

	ui->aniCombo->clear();

	for (auto& head : list) {
		ui->aniCombo->addItem(QString().fromStdString(head));
	}
}
void MissionEventsDialog::initWaveFilenames() {
	auto list = _model->getWaveList();

	ui->waveCombo->clear();

	for (auto& wave : list) {
		ui->waveCombo->addItem(QString().fromStdString(wave));
	}
}
void MissionEventsDialog::initPersonas() {
	auto list = _model->getPersonaList();

	ui->personaCombo->clear();

	for (auto&& [name, id] : _model->getPersonaList()) {
		ui->personaCombo->addItem(QString::fromStdString(name), id);
	}
}

void MissionEventsDialog::initMessageTeams() {
	auto list = _model->getTeamList();

	ui->messageTeamCombo->clear();

	for (const auto& team : list) {
		ui->messageTeamCombo->addItem(QString::fromStdString(team.first), team.second);
	}

}

void MissionEventsDialog::initEventTeams()
{
	auto list = _model->getTeamList();

	ui->teamCombo->clear();

	for (const auto& team : list) {
		ui->teamCombo->addItem(QString::fromStdString(team.first), team.second);
	}
}

void MissionEventsDialog::updateMessageUi()
{
	bool enable = true;

	if (!_model->messageIsValid()) {
		enable = false;

		ui->messageName->setText("");
		ui->messageContent->setPlainText("");
		ui->aniCombo->setEditText("");
		ui->personaCombo->setCurrentIndex(-1);
		ui->waveCombo->setEditText("");
		ui->messageTeamCombo->setCurrentIndex(-1);
		ui->btnMsgNote->setText("Add Node");
	} else {
		ui->messageName->setText(QString().fromStdString(_model->getMessageName()));
		ui->messageContent->setPlainText(QString().fromStdString(_model->getMessageText()));
		ui->aniCombo->setEditText(QString().fromStdString(_model->getMessageAni()));
		ui->personaCombo->setCurrentIndex(ui->personaCombo->findData(_model->getMessagePersona()));
		ui->waveCombo->setEditText(QString().fromStdString(_model->getMessageWave()));
		ui->messageTeamCombo->setCurrentIndex(ui->messageTeamCombo->findData(_model->getMessageTeam()));
		if (_model->getMessageNote().empty()) {
			ui->btnMsgNote->setText("Add Note");
		} else {
			ui->btnMsgNote->setText("Edit Note");
		}
	}

	ui->messageName->setEnabled(enable);
	ui->messageContent->setEnabled(enable);
	ui->aniCombo->setEnabled(enable);
	ui->btnAniBrowse->setEnabled(enable);
	ui->btnBrowseWave->setEnabled(enable);
	ui->btnWavePlay->setEnabled(enable);
	ui->waveCombo->setEnabled(enable);
	ui->btnDeleteMsg->setEnabled(enable);
	ui->personaCombo->setEnabled(enable);
	ui->messageTeamCombo->setEnabled(enable && _model->getMissionIsMultiTeam());
	ui->btnMsgNote->setEnabled(enable);
}
bool MissionEventsDialog::hasDefaultMessageParamter() {
	//return !m_messages.empty();
	return false;
}
int MissionEventsDialog::getRootReturnType() const {
	return OPR_NULL;
}

void MissionEventsDialog::browseAni() {
	//TODO
	/*if (m_cur_msg < 0 || m_cur_msg >= (int)m_messages.size()) {
		return;
	}

	auto z = cfile_push_chdir(CF_TYPE_INTERFACE);
	auto interface_path = QDir::currentPath();
	if (!z) {
		cfile_pop_dir();
	}

	auto name = QFileDialog::getOpenFileName(this,
											 tr("Select message animation"),
											 interface_path,
											 "APNG Files (*.png);;Ani Files (*.ani);;Eff Files (*.eff);;"
											 "All Anims (*.ani, *.eff, *.png)");

	if (name.isEmpty()) {
		// Nothing was selected
		return;
	}

	QFileInfo info(name);

	if (m_messages[m_cur_msg].avi_info.name) {
		free(m_messages[m_cur_msg].avi_info.name);
		m_messages[m_cur_msg].avi_info.name = nullptr;
	}
	m_messages[m_cur_msg].avi_info.name = strdup(info.fileName().toUtf8().constData());
	set_current_message(m_cur_msg);

	modified = true;*/
}

// TODO??
void MissionEventsDialog::updateEventBitmap() {
	auto chained = _model->getChained();
	auto hasObjectiveText = !_model->getEventDirectiveText().empty();

	NodeImage bitmap;
	if (chained) {
		if (!hasObjectiveText) {
			bitmap = NodeImage::CHAIN;
		} else {
			bitmap = NodeImage::CHAIN_DIRECTIVE;
		}
	} else {
		if (!hasObjectiveText) {
			bitmap = NodeImage::ROOT;
		} else {
			bitmap = NodeImage::ROOT_DIRECTIVE;
		}
	}
	for (int i = 0; i < ui->eventTree->topLevelItemCount(); ++i) {
		auto item = ui->eventTree->topLevelItem(i);

		if (item->data(0, sexp_tree::FormulaDataRole).toInt() == _model->getFormula()) {
			item->setIcon(0, sexp_tree::convertNodeImageToIcon(bitmap));
			return;
		}
	}
}

// TODO??
QTreeWidgetItem* MissionEventsDialog::get_event_handle(int num)
{
	/*for (auto i = 0; i < ui->eventTree->topLevelItemCount(); ++i) {
		auto item = ui->eventTree->topLevelItem(i);

		if (item->data(0, sexp_tree::FormulaDataRole).toInt() == m_events[num].formula) {
			return item;
		}
	}
	return nullptr;*/
}

// TODO??
void MissionEventsDialog::keyPressEvent(QKeyEvent* event) {
	if (event->key() == Qt::Key_Escape) {
		// Instead of calling reject when we close a dialog it should try to close the window which will will allow the
		// user to save unsaved changes
		event->ignore();
		this->close();
		return;
	}
	QDialog::keyPressEvent(event);
}

void MissionEventsDialog::on_okAndCancelButtons_accepted()
{
	accept();
}

void MissionEventsDialog::on_okAndCancelButtons_rejected()
{
	reject();
}

void MissionEventsDialog::on_btnNewEvent_clicked()
{
	_model->createEvent();

	updateEventUi();
}

void MissionEventsDialog::on_btnInsertEvent_clicked()
{
	_model->insertEvent();

	updateEventUi();
}

void MissionEventsDialog::on_btnDeleteEvent_clicked()
{
	_model->deleteEvent();

	updateEventUi();
}

void MissionEventsDialog::on_repeatCountBox_valueChanged(int value)
{
	_model->setRepeatCount(value);
	updateEventUi();
}

void MissionEventsDialog::on_triggerCountBox_valueChanged(int value)
{
	_model->setTriggerCount(value);
	updateEventUi();
}

void MissionEventsDialog::on_intervalTimeBox_valueChanged(int value)
{
	_model->setIntervalTime(value);
}

void MissionEventsDialog::on_chainedCheckBox_stateChanged(int state)
{
	_model->setChained(state == Qt::Checked);
	updateEventBitmap();
	updateEventUi();
}

void MissionEventsDialog::on_chainedDelayBox_valueChanged(int value)
{
	_model->setChainDelay(value);
}

void MissionEventsDialog::on_scoreBox_valueChanged(int value)
{
	_model->setEventScore(value);
}

void MissionEventsDialog::on_teamCombo_currentIndexChanged(int index)
{
	_model->setEventTeam(ui->teamCombo->itemData(index).toInt());
}

void MissionEventsDialog::on_editDirectiveText_textChanged(const QString& text)
{
	SCP_string dir = text.toUtf8().constData();
	_model->setEventDirectiveText(dir);
	updateEventBitmap();
}

void MissionEventsDialog::on_editDirectiveKeypressText_textChanged(const QString& text)
{
	SCP_string dir = text.toUtf8().constData();
	_model->setEventDirectiveKeyText(dir);
}

void MissionEventsDialog::on_checkLogTrue_stateChanged(int state)
{
	_model->setLogTrue(state == Qt::Checked);
}

void MissionEventsDialog::on_checkLogFalse_stateChanged(int state)
{
	_model->setLogFalse(state == Qt::Checked);
}

void MissionEventsDialog::on_checkLogPrevious_stateChanged(int state)
{
	_model->setLogLogPrevious(state == Qt::Checked);
}

void MissionEventsDialog::on_checkLogAlwaysFalse_stateChanged(int state)
{
	_model->setLogAlwaysFalse(state == Qt::Checked);
}

void MissionEventsDialog::on_checkLogFirstRepeat_stateChanged(int state)
{
	_model->setLogFirstRepeat(state == Qt::Checked);
}

void MissionEventsDialog::on_checkLogLastRepeat_stateChanged(int state)
{
	_model->setLogLastRepeat(state == Qt::Checked);
}

void MissionEventsDialog::on_checkLogFirstTrigger_stateChanged(int state)
{
	_model->setLogFirstTrigger(state == Qt::Checked);
}

void MissionEventsDialog::on_checkLogLastTrigger_stateChanged(int state)
{
	_model->setLogLastTrigger(state == Qt::Checked);
}

void MissionEventsDialog::on_messageList_currentRowChanged(int row)
{
	_model->setCurrentlySelectedMessage(row);
	updateMessageUi();
}

void MissionEventsDialog::on_messageList_itemDoubleClicked(QListWidgetItem* item)
{
	// TODO
	/*auto message_name = item->text();

	int message_nodes[MAX_SEARCH_MESSAGE_DEPTH];
	auto num_messages =
		ui->eventTree->find_text(message_name.toUtf8().constData(), message_nodes, MAX_SEARCH_MESSAGE_DEPTH);

	if (num_messages == 0) {
		QString message = tr("No events using message '%1'").arg(message_name);
		QMessageBox::information(this, "Error", message);
	} else {
		// find last message_node
		if (m_last_message_node == -1) {
			m_last_message_node = message_nodes[0];
		} else {

			if (num_messages == 1) {
				// only 1 message
				m_last_message_node = message_nodes[0];
			} else {
				// find which message and go to next message
				int found_pos = -1;
				for (int i = 0; i < num_messages; i++) {
					if (message_nodes[i] == m_last_message_node) {
						found_pos = i;
						break;
					}
				}

				if (found_pos == -1) {
					// no previous message
					m_last_message_node = message_nodes[0];
				} else if (found_pos == num_messages - 1) {
					// cycle back to start
					m_last_message_node = message_nodes[0];
				} else {
					// go to next
					m_last_message_node = message_nodes[found_pos + 1];
				}
			}
		}

		// highlight next
		ui->eventTree->hilite_item(m_last_message_node);
	}*/
}

void MissionEventsDialog::on_btnNewMsg_clicked()
{
	_model->createMessage();

	rebuildMessageList();
	updateMessageUi();
}

void MissionEventsDialog::on_btnDeleteMsg_clicked()
{
	_model->deleteMessage();

	rebuildMessageList();
	updateMessageUi();
}

void MissionEventsDialog::on_messageName_textChanged(const QString& text)
{
	SCP_string name = text.toUtf8().constData();
	_model->setMessageName(name);

	rebuildMessageList();
}

void MissionEventsDialog::on_messageContent_textChanged()
{
	SCP_string content = ui->messageContent->toPlainText().toUtf8().constData();
	_model->setMessageText(content);
}

void MissionEventsDialog::on_btnMsgNote_clicked()
{
	if (!_model->messageIsValid())
		return;

	QDialog dlg(this);
	dlg.setWindowTitle(tr("Message Note"));
	auto* layout = new QVBoxLayout(&dlg);
	auto* label = new QLabel(tr("Enter a note for this message:"), &dlg);
	auto* edit = new QTextEdit(&dlg);
	edit->setPlainText(QString::fromUtf8(_model->getMessageNote().c_str()));
	edit->setMinimumSize(700, 500); // big!
	auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);

	layout->addWidget(label);
	layout->addWidget(edit, 1);
	layout->addWidget(buttons);

	QObject::connect(buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
	QObject::connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

	if (dlg.exec() != QDialog::Accepted)
		return;

	SCP_string note = edit->toPlainText().toUtf8().constData();
	_model->setMessageNote(note);

	// Update the button text
	if (note.empty()) {
		ui->btnMsgNote->setText("Add Note");
	} else {
		ui->btnMsgNote->setText("Edit Note");
	}
}

void MissionEventsDialog::on_aniCombo_editingFinished()
{
	SCP_string name = ui->aniCombo->currentText().toUtf8().constData();
	_model->setMessageAni(name);

	initHeadCombo();
	ui->aniCombo->setCurrentText(QString::fromStdString(name));
}

void MissionEventsDialog::on_aniCombo_selectedIndexChanged(int index)
{
	SCP_string name = ui->aniCombo->itemText(index).toUtf8().constData();
	_model->setMessageAni(name);
}

void MissionEventsDialog::on_btnAniBrowse_clicked()
{
	// TODO make ANI browser with previews and file input
}

void MissionEventsDialog::on_waveCombo_editingFinished()
{
	SCP_string name = ui->waveCombo->currentText().toUtf8().constData();
	_model->setMessageWave(name);

	initWaveFilenames();
	ui->waveCombo->setCurrentText(QString::fromStdString(name));
}

void MissionEventsDialog::on_waveCombo_selectedIndexChanged(int index)
{
	SCP_string name = ui->waveCombo->itemText(index).toUtf8().constData();
	_model->setMessageWave(name);
}

void MissionEventsDialog::on_btnBrowseWave_clicked()
{
	if (!_model->messageIsValid()) {
		return;
	}

	int z;
	if (The_mission.game_type & MISSION_TYPE_TRAINING) {
		z = cfile_push_chdir(CF_TYPE_VOICE_TRAINING);
	} else {
		z = cfile_push_chdir(CF_TYPE_VOICE_SPECIAL);
	}
	auto interface_path = QDir::currentPath();
	if (!z) {
		cfile_pop_dir();
	}

	auto name = QFileDialog::getOpenFileName(this,
		tr("Select message animation"),
		interface_path,
		"Voice Files (*.ogg *.wav);;Ogg Vorbis Files (*.ogg);;Wave Files (*.wav);;All Files (*)");

	if (name.isEmpty()) {
		// Nothing was selected
		return;
	}

	QFileInfo info(name);

	SCP_string file_name = info.fileName().toUtf8().constData();

	_model->setMessageWave(file_name);

	initWaveFilenames();
	ui->waveCombo->setCurrentText(QString::fromStdString(file_name));
}

void MissionEventsDialog::on_btnWavePlay_clicked()
{
	_model->playMessageWave();
}

void MissionEventsDialog::on_personaCombo_currentIndexChanged(int index)
{
	_model->setMessagePersona(ui->personaCombo->itemData(index).toInt());
}

void MissionEventsDialog::on_btnUpdateStuff_clicked()
{
	auto result = _viewport->dialogProvider->showButtonDialog(
		DialogType::Question,
		"Update Message Stuff",
		"This will update the message animation and persona to match the current mission settings. "
		   "Are you sure you want to do this?",
		{DialogButton::Yes, DialogButton::No});

	if (result != DialogButton::Yes) {
		_model->autoSelectPersona();
		updateMessageUi();
	}
}

void MissionEventsDialog::on_messageTeamCombo_currentIndexChanged(int index)
{
	_model->setMessageTeam(ui->messageTeamCombo->itemData(index).toInt());
}

} // namespace fso::fred::dialogs

