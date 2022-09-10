//
//

#include "EventEditorDialog.h"
#include "ui_EventEditorDialog.h"
#include "ui/util/SignalBlockers.h"

#include <sound/audiostr.h>
#include <localization/localize.h>

#include <QMessageBox>
#include <QFileDialog>
#include <QDebug>
#include <QKeyEvent>
#include <mission/missionmessage.h>

namespace fso {
namespace fred {
namespace dialogs {

namespace {
void maybe_add_head(QComboBox* box, const char* name) {
	auto id = box->findText(QString::fromUtf8(name));
	if (id < 0) {
		box->addItem(name);
	}
}
int safe_stricmp(const char* one, const char* two) {
	if (!one && !two) {
		return 0;
	}

	if (!one) {
		return -1;
	}

	if (!two) {
		return 1;
	}

	return stricmp(one, two);
}

}

EventEditorDialog::EventEditorDialog(QWidget* parent, EditorViewport* viewport) :
	QDialog(parent),
	SexpTreeEditorInterface({ TreeFlags::LabeledRoot, TreeFlags::RootDeletable, TreeFlags::RootEditable }),
	ui(new Ui::EventEditorDialog()),
	_editor(viewport->editor) {
	ui->setupUi(this);

	ui->eventTree->initializeEditor(viewport->editor, this);

	connect(this, &QDialog::accepted, this, &EventEditorDialog::applyChanges);
	connect(this, &QDialog::rejected, this, &EventEditorDialog::rejectChanges);

	initMessageWidgets();

	initEventWidgets();
}
void EventEditorDialog::initEventWidgets() {
	initEventTree();

	ui->miniHelpBox->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
	ui->helpBox->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));

	connect(ui->eventTree, &sexp_tree::modified, this, [this]() { modified = true; });
	connect(ui->eventTree, &sexp_tree::rootNodeDeleted, this, &EventEditorDialog::rootNodeDeleted);
	connect(ui->eventTree, &sexp_tree::rootNodeRenamed, this, &EventEditorDialog::rootNodeRenamed);
	connect(ui->eventTree, &sexp_tree::rootNodeFormulaChanged, this, &EventEditorDialog::rootNodeFormulaChanged);
	connect(ui->eventTree,
			&sexp_tree::miniHelpChanged,
			this,
			[this](const QString& help) { ui->miniHelpBox->setText(help); });
	connect(ui->eventTree,
			&sexp_tree::helpChanged,
			this,
			[this](const QString& help) { ui->helpBox->setPlainText(help); });
	connect(ui->eventTree, &sexp_tree::selectedRootChanged, this, [this](int formula) {
		for (auto i = 0; i < m_num_events; i++) {
			if (m_events[i].formula == formula) {
				set_current_event(i);
				return;
			}
		}
		set_current_event(-1);
	});
	connect(ui->repeatCountBox, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int value) {
		if (cur_event < 0) {
			return;
		}
		m_events[cur_event].repeat_count = value;
	});
	connect(ui->triggerCountBox, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int value) {
		if (cur_event < 0) {
			return;
		}
		m_events[cur_event].trigger_count = value;
	});
	connect(ui->intervalTimeBox, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int value) {
		if (cur_event < 0) {
			return;
		}
		m_events[cur_event].interval = value;
	});
	connect(ui->scoreBox, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int value) {
		if (cur_event < 0) {
			return;
		}
		m_events[cur_event].score = value;
	});
	connect(ui->chainedCheckBox, &QCheckBox::stateChanged, this, [this](int value) {
		if (cur_event < 0) {
			return;
		}

		if (value != Qt::Checked) {
			m_events[cur_event].chain_delay = -1;
		} else {
			m_events[cur_event].chain_delay = ui->chainDelayBox->value();
		}

		updateEventBitmap();
	});
	connect(ui->editDirectiveText, &QLineEdit::textChanged, this, [this](const QString& value) {
		if (cur_event < 0) {
			return;
		}

		if (m_events[cur_event].objective_text) {
			free(m_events[cur_event].objective_text);
		}

		if (value.isEmpty()) {
			m_events[cur_event].objective_text = nullptr;
		} else {
			m_events[cur_event].objective_text = strdup(value.toUtf8().constData());
		}

		updateEventBitmap();
	});
	connect(ui->editDirectiveKeypressText, &QLineEdit::textChanged, this, [this](const QString& value) {
		if (cur_event < 0) {
			return;
		}

		if (m_events[cur_event].objective_key_text) {
			free(m_events[cur_event].objective_key_text);
		}

		if (value.isEmpty()) {
			m_events[cur_event].objective_key_text = NULL;
		} else {
			m_events[cur_event].objective_key_text = strdup(value.toUtf8().constData());
		}
	});
	connectLogState(ui->checkLogTrue, MLF_SEXP_TRUE);
	connectLogState(ui->checkLogFalse, MLF_SEXP_FALSE);
	connectLogState(ui->checkLogAlwaysFalse, MLF_SEXP_KNOWN_FALSE);
	connectLogState(ui->checkLogFirstRepeat, MLF_FIRST_REPEAT_ONLY);
	connectLogState(ui->checkLogLastRepeat, MLF_LAST_REPEAT_ONLY);
	connectLogState(ui->checkLogFirstTrigger, MLF_FIRST_TRIGGER_ONLY);
	connectLogState(ui->checkLogLastTrigger, MLF_LAST_TRIGGER_ONLY);
	connectLogState(ui->checkLogPrevious, MLF_STATE_CHANGE);

	connect(ui->btnNewEvent, &QPushButton::clicked, this, &EventEditorDialog::newEventHandler);
	connect(ui->btnInsertEvent, &QPushButton::clicked, this, &EventEditorDialog::insertEventHandler);
	connect(ui->btnDeleteEvent, &QPushButton::clicked, this, &EventEditorDialog::deleteEventHandler);

	set_current_event(-1);
}
void EventEditorDialog::initMessageWidgets() {
	initMessageList();

	initHeadCombo();

	initWaveFilenames();

	initPersonas();

	ui->messageName->setMaxLength(NAME_LENGTH - 1);

	connect(ui->aniCombo,
			QOverload<const QString&>::of(&QComboBox::currentTextChanged),
			this,
			[this](const QString& text) {
				if (m_cur_msg < 0) {
					return;
				}

				if (m_messages[m_cur_msg].avi_info.name) {
					free(m_messages[m_cur_msg].avi_info.name);
					m_messages[m_cur_msg].avi_info.name = nullptr;
				}

				auto ptr = text.toUtf8();
				if (ptr.isEmpty() || !VALID_FNAME(ptr)) {
					m_messages[m_cur_msg].avi_info.name = NULL;
				} else {
					m_messages[m_cur_msg].avi_info.name = strdup(ptr);
				}
			});
	connect(ui->waveCombo,
			QOverload<const QString&>::of(&QComboBox::currentTextChanged),
			this,
			[this](const QString& text) {
				if (m_cur_msg < 0) {
					return;
				}

				if (m_messages[m_cur_msg].wave_info.name) {
					free(m_messages[m_cur_msg].wave_info.name);
					m_messages[m_cur_msg].wave_info.name = nullptr;
				}

				auto ptr = text.toUtf8();
				if (ptr.isEmpty() || !VALID_FNAME(ptr)) {
					m_messages[m_cur_msg].wave_info.name = NULL;
				} else {
					m_messages[m_cur_msg].wave_info.name = strdup(ptr);
				}
				updatePersona();
				set_current_message(m_cur_msg);
			});
	connect(ui->messageName, QOverload<const QString&>::of(&QLineEdit::textChanged), this, [this](const QString& text) {
		if (m_cur_msg < 0) {
			return;
		}

		auto conflict = false;
		auto ptr = text.toUtf8();
		for (auto i = 0; i < Num_builtin_messages; i++) {
			if (!stricmp(ptr, Messages[i].name)) {
				conflict = true;
				break;
			}
		}

		for (auto i = 0; i < m_num_messages; i++) {
			if ((i != m_cur_msg) && (!stricmp(ptr, m_messages[i].name))) {
				conflict = true;
				break;
			}
		}

		if (!conflict) {  // update name if no conflicts, otherwise keep old name
			strncpy_s(m_messages[m_cur_msg].name, text.toUtf8().constData(), NAME_LENGTH - 1);

			auto item = ui->messageList->item(m_cur_msg);
			item->setText(text);
		}
	});
	connect(ui->messageContent, &QPlainTextEdit::textChanged, this, [this]() {
		if (m_cur_msg < 0) {
			return;
		}

		auto msg = ui->messageContent->toPlainText();

		strncpy_s(m_messages[m_cur_msg].message, msg.toUtf8().constData(), MESSAGE_LENGTH - 1);
		lcl_fred_replace_stuff(m_messages[m_cur_msg].message, MESSAGE_LENGTH - 1);
	});
	connect(ui->messageTeamCombo, QOverload<int>::of(&QComboBox::activated), this, [this](int id) {
		if (m_cur_msg < 0) {
			return;
		}

		if (id >= MAX_TVT_TEAMS) {
			m_messages[m_cur_msg].multi_team = -1;
		} else {
			m_messages[m_cur_msg].multi_team = id;
		}
	});
	connect(ui->personaCombo, QOverload<int>::of(&QComboBox::activated), this, [this](int id) {
		if (m_cur_msg < 0) {
			return;
		}

		// update the persona to the message.  We subtract 1 for the "None" at the beginning of the combo
		// box list.
		m_messages[m_cur_msg].persona_index = id - 1;
	});

	connect(ui->messageList, &QListWidget::currentRowChanged, this, [this](int row) { set_current_message(row); });
	connect(ui->messageList, &QListWidget::itemDoubleClicked, this, &EventEditorDialog::messageDoubleClicked);

	connect(ui->btnNewMsg, &QPushButton::clicked, this, [this](bool) { createNewMessage(); });
	connect(ui->btnDeleteMsg, &QPushButton::clicked, this, [this](bool) { deleteMessage(); });

	connect(ui->btnAniBrowse, &QPushButton::clicked, this, [this](bool) { browseAni(); });
	connect(ui->btnBrowseWave, &QPushButton::clicked, this, [this](bool) { browseWave(); });

	connect(ui->btnWavePlay, &QPushButton::clicked, this, [this](bool){ playWave(); });

	connect(ui->btnUpdateStuff, &QPushButton::clicked, this, [this](bool) { updateStuff(); });
}
EventEditorDialog::~EventEditorDialog() = default;
void EventEditorDialog::initEventTree() {
	load_tree();

	create_tree();

}
void EventEditorDialog::load_tree() {
	ui->eventTree->clear_tree();

	m_num_events = Num_mission_events;
	for (auto i = 0; i < m_num_events; i++) {
		m_events[i] = Mission_events[i];
		if (Mission_events[i].objective_text) {
			m_events[i].objective_text = strdup(Mission_events[i].objective_text);
		} else {
			m_events[i].objective_text = NULL;
		}

		if (Mission_events[i].objective_key_text) {
			m_events[i].objective_key_text = strdup(Mission_events[i].objective_key_text);
		} else {
			m_events[i].objective_key_text = NULL;
		}

		m_sig[i] = i;
		if (!(*m_events[i].name)) {
			strcpy_s(m_events[i].name, "<Unnamed>");
		}

		m_events[i].formula = ui->eventTree->load_sub_tree(Mission_events[i].formula, false, "do-nothing");

		// we must check for the case of the repeat count being 0.  This would happen if the repeat
		// count is not specified in a mission
		if (m_events[i].repeat_count <= 0) {
			m_events[i].repeat_count = 1;
		}
	}

	ui->eventTree->post_load();
	cur_event = -1;
}
void EventEditorDialog::create_tree() {
	ui->eventTree->clear();
	for (auto i = 0; i < m_num_events; i++) {
		// set the proper bitmap
		NodeImage image;
		if (m_events[i].chain_delay >= 0) {
			image = NodeImage::CHAIN;
			if (m_events[i].objective_text) {
				image = NodeImage::CHAIN_DIRECTIVE;
			}
		} else {
			image = NodeImage::ROOT;
			if (m_events[i].objective_text) {
				image = NodeImage::ROOT_DIRECTIVE;
			}
		}

		auto h = ui->eventTree->insert(m_events[i].name, image);
		h->setData(0, sexp_tree::FormulaDataRole, m_events[i].formula);
		ui->eventTree->add_sub_tree(m_events[i].formula, h);
	}

	cur_event = -1;
}
void EventEditorDialog::rootNodeDeleted(int node) {
	int i;
	for (i = 0; i < m_num_events; i++) {
		if (m_events[i].formula == node) {
			break;
		}
	}

	Assert(i < m_num_events);
	auto index = i;
	while (i < m_num_events - 1) {
		m_events[i] = m_events[i + 1];
		m_sig[i] = m_sig[i + 1];
		i++;
	}

	m_num_events--;
	ui->btnNewEvent->setEnabled(true);

	set_current_event(index);
}
void EventEditorDialog::rootNodeRenamed(int /*node*/) {
}
void EventEditorDialog::rootNodeFormulaChanged(int old, int node) {
	int i;

	for (i = 0; i < m_num_events; i++) {
		if (m_events[i].formula == old) {
			break;
		}
	}

	Assert(i < m_num_events);
	m_events[i].formula = node;
}
void EventEditorDialog::initMessageList() {
	if (m_num_events >= MAX_MISSION_EVENTS) {
		ui->btnNewMsg->setEnabled(false);
	}
	m_num_messages = Num_messages - Num_builtin_messages;
	m_messages.clear();
	m_messages.reserve(m_num_messages);
	for (auto i = 0; i < m_num_messages; i++) {
		auto msg = Messages[i + Num_builtin_messages];
		m_messages.push_back(msg);
		if (m_messages[i].avi_info.name) {
			m_messages[i].avi_info.name = strdup(m_messages[i].avi_info.name);
		}
		if (m_messages[i].wave_info.name) {
			m_messages[i].wave_info.name = strdup(m_messages[i].wave_info.name);
		}
	}

	rebuildMessageList();

	if (Num_messages > Num_builtin_messages) {
		set_current_message(0);
	} else {
		set_current_message(-1);
	}
}
void EventEditorDialog::rebuildMessageList() {
	// Block signals so that the current item index isn't overwritten by this
	QSignalBlocker blocker(ui->messageList);

	ui->messageList->clear();
	for (auto& msg : m_messages) {
		auto item = new QListWidgetItem(msg.name, ui->messageList);
		ui->messageList->addItem(item);
	}
}
void EventEditorDialog::set_current_event(int evt) {
	util::SignalBlockers blockers(this);

	cur_event = evt;

	if (cur_event < 0) {
		ui->repeatCountBox->setValue(1);
		ui->triggerCountBox->setValue(1);
		ui->intervalTimeBox->setValue(1);
		ui->chainDelayBox->setValue(0);
		ui->teamCombo->setCurrentIndex(MAX_TVT_TEAMS);
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

	if (m_events[cur_event].team < 0) {
		ui->teamCombo->setCurrentIndex(MAX_TVT_TEAMS);
	} else {
		ui->teamCombo->setCurrentIndex(m_events[cur_event].team);
	}

	ui->repeatCountBox->setValue(m_events[cur_event].repeat_count);
	ui->triggerCountBox->setValue(m_events[cur_event].trigger_count);
	ui->intervalTimeBox->setValue(m_events[cur_event].interval);
	ui->scoreBox->setValue(m_events[cur_event].score);
	if (m_events[cur_event].chain_delay >= 0) {
		ui->chainedCheckBox->setChecked(true);
		ui->chainDelayBox->setValue(m_events[cur_event].chain_delay);
		ui->chainDelayBox->setEnabled(true);
	} else {
		ui->chainedCheckBox->setChecked(false);
		ui->chainDelayBox->setValue(0);
		ui->chainDelayBox->setEnabled(false);
	}

	if (m_events[cur_event].objective_text){
		ui->editDirectiveText->setText(QString::fromUtf8(m_events[cur_event].objective_text));
	} else {
		ui->editDirectiveText->setText("");
	}

	if (m_events[cur_event].objective_key_text){
		ui->editDirectiveKeypressText->setText(QString::fromUtf8(m_events[cur_event].objective_key_text));
	} else {
		ui->editDirectiveKeypressText->setText("");
	}

	ui->repeatCountBox->setEnabled(true);
	ui->triggerCountBox->setEnabled(true);

	if (( m_events[cur_event].repeat_count <= 1) && (m_events[cur_event].trigger_count <= 1)) {
		ui->intervalTimeBox->setValue(1);
		ui->intervalTimeBox->setEnabled(false);
	} else {
		ui->intervalTimeBox->setEnabled(true);
	}

	ui->scoreBox->setEnabled(true);
	ui->chainedCheckBox->setEnabled(true);
	ui->editDirectiveText->setEnabled(true);
	ui->editDirectiveKeypressText->setEnabled(true);
	ui->teamCombo->setEnabled(false);
	if ( The_mission.game_type & MISSION_TYPE_MULTI_TEAMS ){
		ui->teamCombo->setEnabled(true);
	}

	// handle event log flags
	ui->checkLogTrue->setChecked((m_events[cur_event].mission_log_flags & MLF_SEXP_TRUE) != 0);
	ui->checkLogFalse->setChecked((m_events[cur_event].mission_log_flags & MLF_SEXP_FALSE) != 0);
	ui->checkLogAlwaysFalse->setChecked((m_events[cur_event].mission_log_flags & MLF_SEXP_KNOWN_FALSE) != 0);
	ui->checkLogFirstRepeat->setChecked((m_events[cur_event].mission_log_flags & MLF_FIRST_REPEAT_ONLY) != 0);
	ui->checkLogLastRepeat->setChecked((m_events[cur_event].mission_log_flags & MLF_LAST_REPEAT_ONLY) != 0);
	ui->checkLogFirstTrigger->setChecked((m_events[cur_event].mission_log_flags & MLF_FIRST_TRIGGER_ONLY) != 0);
	ui->checkLogLastTrigger->setChecked((m_events[cur_event].mission_log_flags & MLF_LAST_TRIGGER_ONLY) != 0);
	ui->checkLogPrevious->setChecked((m_events[cur_event].mission_log_flags & MLF_STATE_CHANGE) != 0);
}
void EventEditorDialog::initHeadCombo() {
	auto box = ui->aniCombo;
	box->clear();
	box->addItem("<None>");
	for (auto i = 0; i < Num_messages; i++) {
		if (Messages[i].avi_info.name) {
			maybe_add_head(box, Messages[i].avi_info.name);
		}
	}

	if (!Disable_hc_message_ani) {
		maybe_add_head(box, "Head-TP2");
		maybe_add_head(box, "Head-VC2");
		maybe_add_head(box, "Head-TP4");
		maybe_add_head(box, "Head-TP5");
		maybe_add_head(box, "Head-TP6");
		maybe_add_head(box, "Head-TP7");
		maybe_add_head(box, "Head-TP8");
		maybe_add_head(box, "Head-VP2");
		maybe_add_head(box, "Head-VP2");
		maybe_add_head(box, "Head-CM2");
		maybe_add_head(box, "Head-CM3");
		maybe_add_head(box, "Head-CM4");
		maybe_add_head(box, "Head-CM5");
		maybe_add_head(box, "Head-BSH");
	}
}
void EventEditorDialog::initWaveFilenames() {
	auto box = ui->waveCombo;
	box->clear();
	box->addItem("<None>");
	for (auto i = 0; i < Num_messages; i++) {
		if (Messages[i].wave_info.name) {
			auto id = box->findText(Messages[i].wave_info.name);
			if (id < 0) {
				box->addItem(Messages[i].wave_info.name);
			}
		}
	}
}
void EventEditorDialog::initPersonas() {
	// add the persona names into the combo box
	auto box = ui->personaCombo;
	box->clear();
	box->addItem("<None>");
	for (auto i = 0; i < Num_personas; i++) {
		box->addItem(Personas[i].name);
	}
}
void EventEditorDialog::set_current_message(int msg) {
	ui->messageList->setCurrentItem(ui->messageList->item(msg));
	m_cur_msg = msg;

	auto enable = true;

	audiostream_close_file(m_wave_id, false);
	m_wave_id = -1;

	if (m_cur_msg < 0) {
		enable = false;

		ui->messageName->setText("");
		ui->messageContent->setPlainText("");
		ui->aniCombo->setEditText("");
		ui->personaCombo->setCurrentIndex(0);
		ui->waveCombo->setEditText("");
		ui->teamCombo->setCurrentIndex(-1);
		ui->messageTeamCombo->setCurrentIndex(-1);
	} else {
		auto& message = m_messages[m_cur_msg];

		ui->messageName->setText(message.name);
		ui->messageContent->setPlainText(message.message);
		ui->aniCombo->setEditText(message.avi_info.name);
		ui->personaCombo->setCurrentIndex(
			message.persona_index + 1); // add one for the "none" at the beginning of the list
		ui->waveCombo->setEditText(message.wave_info.name);

		// m_message_team == -1 maps to 2
		if (m_messages[m_cur_msg].multi_team == -1) {
			ui->messageTeamCombo->setCurrentIndex(MAX_TVT_TEAMS);
		} else {
			ui->messageTeamCombo->setCurrentIndex(m_messages[m_cur_msg].multi_team);
		}
	}

	ui->messageName->setEnabled(enable);
	ui->messageContent->setEnabled(enable);
	ui->aniCombo->setEnabled(enable);
	ui->btnAniBrowse->setEnabled(enable);
	ui->btnBrowseWave->setEnabled(enable);
	ui->waveCombo->setEnabled(enable);
	ui->btnDeleteMsg->setEnabled(enable);
	ui->personaCombo->setEnabled(enable);
	ui->teamCombo->setEnabled(enable);
}
void EventEditorDialog::applyChanges()
{
	audiostream_close_file(m_wave_id, 0);
	m_wave_id = -1;

	auto changes_detected = query_modified();

	for (int i = 0; i < Num_mission_events; i++) {
		free_sexp2(Mission_events[i].formula);
		if (Mission_events[i].objective_text)
			free(Mission_events[i].objective_text);
		if (Mission_events[i].objective_key_text)
			free(Mission_events[i].objective_key_text);
	}

	SCP_vector<std::pair<SCP_string, SCP_string>> names;

	for (int i = 0; i < Num_mission_events; i++)
		Mission_events[i].result = 0; // use this as a processed flag

	// rename all sexp references to old events
	for (int i = 0; i < m_num_events; i++)
		if (m_sig[i] >= 0) {
			names.emplace_back(Mission_events[m_sig[i]].name, m_events[i].name);
			Mission_events[m_sig[i]].result = 1;
		}

	// invalidate all sexp references to deleted events.
	for (int i = 0; i < Num_mission_events; i++)
		if (!Mission_events[i].result) {
			names.emplace_back(Mission_events[i].name, SCP_string("<") + Mission_events[i].name + ">");
		}

	Num_mission_events = m_num_events;
	for (int i = 0; i < m_num_events; i++) {
		Mission_events[i]                    = m_events[i];
		Mission_events[i].formula            = ui->eventTree->save_tree(m_events[i].formula);
		Mission_events[i].objective_text     = m_events[i].objective_text;
		Mission_events[i].objective_key_text = m_events[i].objective_key_text;
		Mission_events[i].mission_log_flags  = m_events[i].mission_log_flags;
	}

	// now update all sexp references
	for (const auto& entry : names) {
		update_sexp_references(entry.first.c_str(), entry.second.c_str(), OPF_EVENT_NAME);
	}

	for (int i = Num_builtin_messages; i < Num_messages; i++) {
		if (Messages[i].avi_info.name)
			free(Messages[i].avi_info.name);

		if (Messages[i].wave_info.name)
			free(Messages[i].wave_info.name);
	}

	Num_messages = m_num_messages + Num_builtin_messages;
	Messages.resize(Num_messages);
	for (int i = 0; i < m_num_messages; i++)
		Messages[i + Num_builtin_messages] = m_messages[i];

	// Only fire the signal after the changes have been applied to make sure the other parts of the code see the updated
	// state
	if (changes_detected) {
		_editor->missionChanged();
	}
}
void EventEditorDialog::closeEvent(QCloseEvent* event) {
	audiostream_close_file(m_wave_id, false);
	m_wave_id = -1;

	if (query_modified()) {
		auto result = QMessageBox::question(this,
											"Close",
											"Do you want to keep your changes?",
											QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
											QMessageBox::Cancel);

		if (result == QMessageBox::Cancel) {
			event->ignore();
			return;
		}

		if (result == QMessageBox::Yes) {
			applyChanges();
			event->accept();
			return;
		}
	}
}
void EventEditorDialog::rejectChanges() {
	audiostream_close_file(m_wave_id, false);
	m_wave_id = -1;

	// Nothing else to do here
}
bool EventEditorDialog::query_modified() {
	if (modified) {
		return true;
	}

	if (Num_mission_events != m_num_events) {
		return true;
	}

	for (auto i = 0; i < m_num_events; i++) {
		if (stricmp(m_events[i].name, Mission_events[i].name) != 0) {
			return true;
		}
		if (m_events[i].repeat_count != Mission_events[i].repeat_count) {
			return true;
		}
		if (m_events[i].trigger_count != Mission_events[i].trigger_count) {
			return true;
		}
		if (m_events[i].interval != Mission_events[i].interval) {
			return true;
		}
		if (m_events[i].score != Mission_events[i].score) {
			return true;
		}
		if (m_events[i].chain_delay != Mission_events[i].chain_delay) {
			return true;
		}
		if (safe_stricmp(m_events[i].objective_text, Mission_events[i].objective_text)) {
			return true;
		}
		if (safe_stricmp(m_events[i].objective_key_text, Mission_events[i].objective_key_text)) {
			return true;
		}
		if (m_events[i].mission_log_flags != Mission_events[i].mission_log_flags) {
			return true;
		}
	}

	if (m_num_messages != Num_messages) {
		return true;
	}

	for (auto i = 0; i < m_num_messages; ++i) {
		auto& local = m_messages[i];
		auto& ref = Messages[i];

		if (stricmp(local.name, ref.name) != 0) {
			return true;
		}
		if (stricmp(local.message, ref.message) != 0) {
			return true;
		}
		if (local.persona_index != ref.persona_index) {
			return true;
		}
		if (local.multi_team != ref.multi_team) {
			return true;
		}
		if (safe_stricmp(local.avi_info.name, ref.avi_info.name)) {
			return true;
		}
		if (safe_stricmp(local.wave_info.name, ref.avi_info.name)) {
			return true;
		}
	}

	return false;
}
bool EventEditorDialog::hasDefaultMessageParamter() {
	return m_num_messages > 0;
}
SCP_vector<SCP_string> EventEditorDialog::getMessages() {
	SCP_vector<SCP_string> messages;
	messages.reserve(m_num_messages);

	for (auto i = 0; i < m_num_messages; ++i) {
		messages.push_back(m_messages[i].name);
	}

	return messages;
}
int EventEditorDialog::getRootReturnType() const {
	return OPR_NULL;
}
void EventEditorDialog::messageDoubleClicked(QListWidgetItem* item) {
	auto message_name = item->text();

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
	}
}
void EventEditorDialog::createNewMessage() {
	MMessage msg;

	strcpy_s(msg.name, "<new message>");

	strcpy_s(msg.message, "<put description here>");
	msg.avi_info.name = NULL;
	msg.wave_info.name = NULL;
	msg.persona_index = -1;
	msg.multi_team = -1;
	m_messages.push_back(msg);
	auto id = m_num_messages++;

	modified = true;

	ui->messageList->addItem(QString::fromUtf8(msg.name));
	set_current_message(id);
}
void EventEditorDialog::deleteMessage() {
	// handle this case somewhat gracefully
	Assertion((m_cur_msg >= -1) && (m_cur_msg < m_num_messages),
			  "Unexpected m_cur_msg value (%d); expected either -1, or between 0-%d. Get a coder!\n",
			  m_cur_msg,
			  m_num_messages - 1);
	if ((m_cur_msg < 0) || (m_cur_msg >= m_num_messages)) {
		return;
	}

	if (m_messages[m_cur_msg].avi_info.name) {
		free(m_messages[m_cur_msg].avi_info.name);
		m_messages[m_cur_msg].avi_info.name = nullptr;
	}
	if (m_messages[m_cur_msg].wave_info.name) {
		free(m_messages[m_cur_msg].wave_info.name);
		m_messages[m_cur_msg].wave_info.name = nullptr;
	}

	SCP_string buf;
	sprintf(buf, "<%s>", m_messages[m_cur_msg].name);
	update_sexp_references(m_messages[m_cur_msg].name, buf.c_str(), OPF_MESSAGE);
	update_sexp_references(m_messages[m_cur_msg].name, buf.c_str(), OPF_MESSAGE_OR_STRING);

	m_messages.erase(m_messages.begin() + m_cur_msg);
	m_num_messages--;

	if (m_cur_msg >= m_num_messages) {
		m_cur_msg = m_num_messages - 1;
	}

	rebuildMessageList();
	set_current_message(m_cur_msg);

	ui->btnNewMsg->setEnabled(true);
	modified = true;

	// The list loses focus when the current image is removed so we fix that here
	ui->messageList->setFocus();
}
void EventEditorDialog::browseAni() {
	if (m_cur_msg < 0 || m_cur_msg >= m_num_messages) {
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

	modified = true;
}
void EventEditorDialog::browseWave() {
	if (m_cur_msg < 0 || m_cur_msg >= m_num_messages) {
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
											 "Voice Files (*.ogg, *.wav);;Ogg Vorbis Files (*.ogg);;"
											 "Wave Files (*.wav)");

	if (name.isEmpty()) {
		// Nothing was selected
		return;
	}

	QFileInfo info(name);

	if (m_messages[m_cur_msg].wave_info.name) {
		free(m_messages[m_cur_msg].wave_info.name);
		m_messages[m_cur_msg].wave_info.name = nullptr;
	}
	m_messages[m_cur_msg].wave_info.name = strdup(info.fileName().toUtf8().constData());
	updatePersona();

	set_current_message(m_cur_msg);

	modified = true;
}
void EventEditorDialog::updatePersona() {
	if (m_cur_msg < 0 || m_cur_msg >= m_num_messages) {
		return;
	}

	SCP_string wave_name = m_messages[m_cur_msg].wave_info.name;
	SCP_string avi_name = m_messages[m_cur_msg].avi_info.name;

	if ((wave_name[0] >= '1') && (wave_name[0] <= '9') && (wave_name[1] == '_')) {
		auto i = wave_name[0] - '1';
		if ((i < Num_personas) && (Personas[i].flags & PERSONA_FLAG_WINGMAN)) {
			m_messages[m_cur_msg].persona_index = i;
			if (i == 0 || i == 1) {
				avi_name = "HEAD-TP1";
			} else if (i == 2 || i == 3) {
				avi_name = "HEAD-TP2";
			} else if (i == 4) {
				avi_name = "HEAD-TP3";
			} else if (i == 5) {
				avi_name = "HEAD-VP1";
			}
		}
	} else {
		auto mask = 0;
		if (!strnicmp(wave_name.c_str(), "S_", 2)) {
			mask = PERSONA_FLAG_SUPPORT;
			avi_name = "HEAD-CM1";
		} else if (!strnicmp(wave_name.c_str(), "L_", 2)) {
			mask = PERSONA_FLAG_LARGE;
			avi_name = "HEAD-CM1";
		} else if (!strnicmp(wave_name.c_str(), "TC_", 3)) {
			mask = PERSONA_FLAG_COMMAND;
			avi_name = "HEAD-CM1";
		}

		for (auto i = 0; i < Num_personas; i++) {
			if (Personas[i].flags & mask) {
				m_messages[m_cur_msg].persona_index = i;
			}
		}
	}

	if (m_messages[m_cur_msg].avi_info.name) {
		free(m_messages[m_cur_msg].avi_info.name);
		m_messages[m_cur_msg].avi_info.name = nullptr;
	}
	m_messages[m_cur_msg].avi_info.name = strdup(avi_name.c_str());

	modified = true;
}
void EventEditorDialog::playWave() {
	if (m_wave_id >= 0) {
		audiostream_close_file(m_wave_id, false);
		m_wave_id = -1;
		return;
	}

	// we use ASF_EVENTMUSIC here so that it will keep the extension in place
	m_wave_id = audiostream_open(m_messages[m_cur_msg].wave_info.name, ASF_EVENTMUSIC);

	if (m_wave_id >= 0) {
		audiostream_play(m_wave_id, 1.0f, 0);
	}
}
void EventEditorDialog::updateStuff() {
	updatePersona();
	set_current_message(m_cur_msg);
}
void EventEditorDialog::updateEventBitmap() {
	auto chained = m_events[cur_event].chain_delay != -1;
	auto hasObjectiveText =
		m_events[cur_event].objective_text != nullptr ? strlen(m_events[cur_event].objective_text) > 0 : false;

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

		if (item->data(0, sexp_tree::FormulaDataRole).toInt() == m_events[cur_event].formula) {
			item->setIcon(0, sexp_tree::convertNodeImageToIcon(bitmap));
			return;
		}
	}
}
void EventEditorDialog::connectLogState(QCheckBox* box, uint32_t flag) {
	connect(box, &QCheckBox::stateChanged, this, [this, flag](int state) {
		if (cur_event < 0) {
			return;
		}

		bool enable = state == Qt::Checked;
		if (enable) {
			m_events[cur_event].mission_log_flags |= flag;
		} else {
			m_events[cur_event].mission_log_flags &= ~flag;
		}
	});
}
void EventEditorDialog::newEventHandler() {
	if (m_num_events >= MAX_MISSION_EVENTS) {
		QMessageBox::critical(this, "Too many events", "You have reached the limit on mission events.\n"
													   "Can't add any more.");
		return;
	}

	reset_event(m_num_events++, nullptr);
}
void EventEditorDialog::insertEventHandler() {
	if (m_num_events >= MAX_MISSION_EVENTS) {
		QMessageBox::critical(this, "Too many events", "You have reached the limit on mission events.\n"
													   "Can't add any more.");
		return;
	}

	if (cur_event < 0 || m_num_events == 0) {
		//There are no events yet, so just create one
		reset_event(m_num_events++, nullptr);
	} else {
		for (auto i = m_num_events; i > cur_event; i--) {
			m_events[i] = m_events[i - 1];
			m_sig[i] = m_sig[i - 1];
		}

		if (cur_event != 0) {
			reset_event(cur_event, get_event_handle(cur_event - 1));
		} else {
			reset_event(cur_event, nullptr);

			// Since there is no TVI_FIRST in Qt we need to do some additional work to get this to work right
			auto new_item = get_event_handle(cur_event);
			auto index = ui->eventTree->indexOfTopLevelItem(new_item);
			ui->eventTree->takeTopLevelItem(index);
			ui->eventTree->insertTopLevelItem(0, new_item);
		}

		m_num_events++;
	}
}
void EventEditorDialog::deleteEventHandler() {
	if (cur_event < 0) {
		return;
	}

	// This is such an ugly hack but I don't want to rewrite sexp_tree just for this..
	auto item = ui->eventTree->currentItem();
	while (item->parent() != nullptr) {
		item = item->parent();
	}
	ui->eventTree->setCurrentItem(item);

	ui->eventTree->deleteCurrentItem();
}

QTreeWidgetItem* EventEditorDialog::get_event_handle(int num)
{
	for (auto i = 0; i < ui->eventTree->topLevelItemCount(); ++i) {
		auto item = ui->eventTree->topLevelItem(i);

		if (item->data(0, sexp_tree::FormulaDataRole).toInt() == m_events[num].formula) {
			return item;
		}
	}
	return nullptr;
}
void EventEditorDialog::reset_event(int num, QTreeWidgetItem* after) {
	strcpy_s(m_events[num].name, "Event name");
	auto h = ui->eventTree->insert(m_events[num].name, NodeImage::ROOT, nullptr, after);

	m_events[num].repeat_count = 1;
	m_events[num].trigger_count = 1;
	m_events[num].interval = 1;
	m_events[num].score = 0;
	m_events[num].chain_delay = -1;
	m_events[num].objective_text = NULL;
	m_events[num].objective_key_text = NULL;
	m_events[num].team = -1;
	m_events[num].mission_log_flags = 0;
	m_sig[num] = -1;

	ui->eventTree->setCurrentItemIndex(-1);
	auto index = m_events[num].formula = ui->eventTree->add_operator("when", h);
	h->setData(0, sexp_tree::FormulaDataRole, index);
	ui->eventTree->add_operator("true");
	ui->eventTree->setCurrentItemIndex(index);
	ui->eventTree->add_operator("do-nothing");

	// First clear the current selection since the add_operator calls added new items and select them by default
	ui->eventTree->clearSelection();
	// This will automatically call set_cur_event
	h->setSelected(true);

	if (num >= MAX_MISSION_EVENTS) {
		ui->btnNewEvent->setEnabled(false);
	}
}
void EventEditorDialog::keyPressEvent(QKeyEvent* event) {
	if (event->key() == Qt::Key_Escape) {
		// Instead of calling reject when we close a dialog it should try to close the window which will will allow the
		// user to save unsaved changes
		event->ignore();
		this->close();
		return;
	}
	QDialog::keyPressEvent(event);
}

}
}
}

