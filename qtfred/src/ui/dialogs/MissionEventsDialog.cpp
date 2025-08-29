#include "MissionEventsDialog.h"
#include "ui_MissionEventsDialog.h"
#include "ui/util/SignalBlockers.h"
#include "ui/dialogs/General/ImagePickerDialog.h"

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
	SexpTreeEditorInterface({ TreeFlags::LabeledRoot, TreeFlags::RootDeletable, TreeFlags::RootEditable, TreeFlags::AnnotationsAllowed }),
	  ui(new Ui::MissionEventsDialog()), _viewport(viewport)
{
	ui->setupUi(this);

	// Build the Qt adapter for our data model
	// This is kinda messy but the sexp_tree widget owns both the ui and the data for the tree
	// Simultaneously our tree model needs to be able to tell the tree when things change and also
	// be able to read data from the tree as needed. So we pass in this small adapter object with
	// the relevant tree operations allowing the model to do all the cross talk it needs
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

		Handle parent_of(Handle node) override
		{
			auto* it = static_cast<QTreeWidgetItem*>(node);
			return static_cast<Handle>(it ? it->parent() : nullptr);
		}

		int index_in_parent(Handle node) override
		{
			auto* it = static_cast<QTreeWidgetItem*>(node);
			if (!it)
				return -1;
			auto* p = it->parent();
			return p ? p->indexOfChild(it) : -1;
		}

		int root_formula_of(Handle node) override
		{
			auto* it = static_cast<QTreeWidgetItem*>(node);
			if (!it)
				return -1;
			while (it->parent())
				it = it->parent();
			return it->data(0, sexp_tree::FormulaDataRole).toInt();
		}

		bool is_handle_valid(Handle h) override
		{
			auto* it = static_cast<QTreeWidgetItem*>(h);
			return it && it->treeWidget() == &tree;
		}

		Handle get_root_by_formula(int formula) override
		{
			return static_cast<Handle>(findRootByFormula(formula));
		}

		int child_count(Handle node) override
		{
			auto* it = static_cast<QTreeWidgetItem*>(node);
			return it ? it->childCount() : 0;
		}

		Handle child_at(Handle node, int idx) override
		{
			auto* it = static_cast<QTreeWidgetItem*>(node);
			if (!it || idx < 0 || idx >= it->childCount())
				return nullptr;
			return static_cast<Handle>(it->child(idx));
		}

		void set_node_note(Handle node, const SCP_string& note) override
		{
			if (auto* it = static_cast<QTreeWidgetItem*>(node)) {
				const QString q = QString::fromStdString(note);
				it->setData(0, sexp_tree::NoteRole, q);
				it->setToolTip(0, q);
				sexp_tree::applyVisuals(it);
			}
		}

		void set_node_bg_color(Handle node, int r, int g, int b, bool has_color) override
		{
			if (auto* it = static_cast<QTreeWidgetItem*>(node)) {
				it->setData(0, sexp_tree::BgColorRole, QColor(r, g, b));
				it->setBackground(0, has_color ? QBrush(QColor(r, g, b)) : QBrush());
				sexp_tree::applyVisuals(it);
			}
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

	connect(ui->eventTree, &sexp_tree::nodeAnnotationChanged, this, [this](void* h, const QString& note) {
		SCP_string text = note.toUtf8().constData();
		_model->setNodeAnnotation(h, text);
	});

	connect(ui->eventTree, &sexp_tree::nodeBgColorChanged, this, [this](void* h, const QColor& c) {
		_model->setNodeBgColor(h, c.red(), c.green(), c.blue(), c.isValid());
	});

	connect(ui->eventTree, &sexp_tree::rootOrderChanged, this, [this] {
		SCP_vector<int> order;
		order.reserve(ui->eventTree->topLevelItemCount());
		for (int i = 0; i < ui->eventTree->topLevelItemCount(); ++i) {
			auto* it = ui->eventTree->topLevelItem(i);
			order.push_back(it->data(0, sexp_tree::FormulaDataRole).toInt());
		}
		_model->reorderByRootFormulaOrder(order);
		m_last_message_node = -1;
	});

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

SCP_vector<SCP_string> MissionEventsDialog::getMessages()
{
	SCP_vector<SCP_string> out;
	const auto& msgs = _model->getMessageList();
	out.reserve(msgs.size());
	for (const auto& m : msgs) {
		out.emplace_back(m.name);
	}
	return out;
}

bool MissionEventsDialog::hasDefaultMessageParamter()
{
	return !_model->getMessageList().empty();
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

	_model->setCurrentlySelectedMessage(_model->getMessageList().empty() ? -1 : 0);
}

void MissionEventsDialog::rebuildMessageList() {
	// Block signals so that the current item index isn't overwritten by this
	QSignalBlocker blocker(ui->messageList);

	const int curRow = _model->getCurrentlySelectedMessage();

	ui->messageList->clear();
	for (auto& msg : _model->getMessageList()) {
		auto item = new QListWidgetItem(msg.name, ui->messageList);
		ui->messageList->addItem(item);
	}

	if (curRow >= 0 && curRow < ui->messageList->count()) {
		ui->messageList->setCurrentRow(curRow);
	}
}

void MissionEventsDialog::updateEventUi() {
	util::SignalBlockers blockers(this);

	updateEventMoveButtons();

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

void MissionEventsDialog::updateEventMoveButtons()
{
	auto* cur = ui->eventTree->currentItem();

	const bool isRoot = (cur && !cur->parent());
	const int count = ui->eventTree->topLevelItemCount();

	bool canUp = false, canDown = false;

	if (isRoot && count > 1) {
		const int idx = ui->eventTree->indexOfTopLevelItem(cur);
		canUp = (idx > 0);
		canDown = (idx >= 0 && idx < count - 1);
	}

	ui->eventUpBtn->setEnabled(canUp);
	ui->eventDownBtn->setEnabled(canDown);
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

	updateMessageMoveButtons();
}

void MissionEventsDialog::updateMessageMoveButtons()
{
	const int count = ui->messageList->count();
	const int row = ui->messageList->currentItem() ? ui->messageList->row(ui->messageList->currentItem()) : -1;

	const bool hasSel = (row >= 0);
	const bool canUp = hasSel && row > 0;
	const bool canDown = hasSel && row < count - 1;

	ui->msgUpBtn->setEnabled(canUp);
	ui->msgDownBtn->setEnabled(canDown);
}

SCP_vector<int> MissionEventsDialog::read_root_formula_order(sexp_tree* tree)
{
	SCP_vector<int> order;
	order.reserve(tree->topLevelItemCount());
	for (int i = 0; i < tree->topLevelItemCount(); ++i) {
		auto* it = tree->topLevelItem(i);
		order.push_back(it->data(0, sexp_tree::FormulaDataRole).toInt());
	}
	return order;
}

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

void MissionEventsDialog::on_eventUpBtn_clicked()
{
	auto* cur = ui->eventTree->currentItem();
	if (!cur || cur->parent())
		return; // roots only
	const int idx = ui->eventTree->indexOfTopLevelItem(cur);
	if (idx <= 0)
		return; // already at top

	QTreeWidgetItem* dest = ui->eventTree->topLevelItem(idx - 1);
	ui->eventTree->move_root(cur, dest, /*insert_before=*/true); // visual move + modified()

	// Keep model in sync with the new root order TODO remove/add this pending sexp_tree widget refactor
	//_model->reorderByRootFormulaOrder(read_root_formula_order(ui->eventTree));

	// Ensure it stays selected and visible
	ui->eventTree->setCurrentItem(cur);
	ui->eventTree->scrollToItem(cur);
	updateEventMoveButtons();
}

void MissionEventsDialog::on_eventDownBtn_clicked()
{
	auto* cur = ui->eventTree->currentItem();
	if (!cur || cur->parent())
		return; // roots only
	const int idx = ui->eventTree->indexOfTopLevelItem(cur);
	const int last = ui->eventTree->topLevelItemCount() - 1;
	if (idx < 0 || idx >= last)
		return; // already at bottom

	QTreeWidgetItem* dest = ui->eventTree->topLevelItem(idx + 1);
	ui->eventTree->move_root(cur, dest, /*insert_before=*/false); // visual move + modified()

	// Keep model in sync with the new root order TODO remove/add this pending sexp_tree widget refactor
	//_model->reorderByRootFormulaOrder(read_root_formula_order(ui->eventTree));

	ui->eventTree->setCurrentItem(cur);
	ui->eventTree->scrollToItem(cur);
	updateEventMoveButtons();
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
	if (!item || !ui->eventTree)
		return;

	const QString name = item->text();
	if (name != m_last_message_name) {
		m_last_message_name = name;
		m_last_message_node = -1; // reset cycle when switching message
	}

	int nodes[MAX_SEARCH_MESSAGE_DEPTH];
	const int num = ui->eventTree->find_text(name.toUtf8().constData(), nodes, MAX_SEARCH_MESSAGE_DEPTH);
	if (num <= 0) {
		QMessageBox::information(this, tr("Error"), tr("No events using message '%1'").arg(name));
		return;
	}

	// cycle to next
	int next = nodes[0];
	if (m_last_message_node != -1) {
		int pos = -1;
		for (int i = 0; i < num; ++i) {
			if (nodes[i] == m_last_message_node) {
				pos = i;
				break;
			}
		}
		next = (pos == -1 || pos == num - 1) ? nodes[0] : nodes[pos + 1];
	}

	m_last_message_node = next;
	ui->eventTree->hilite_item(next);
}

void MissionEventsDialog::on_btnNewMsg_clicked()
{
	_model->createMessage();

	rebuildMessageList();
	updateMessageUi();
}

void MissionEventsDialog::on_btnInsertMsg_clicked()
{
	_model->insertMessage();

	// Refresh list UI (replace with your actual refresh)
	rebuildMessageList();

	// Keep selection/visibility in sync
	const int sel = _model->getCurrentlySelectedMessage(); // or expose accessor
	if (auto* w = ui->messageList) {                            // your list widget id
		w->setCurrentRow(sel);
		if (auto* it = w->item(sel))
			w->scrollToItem(it);
	}
	updateMessageUi();
}

void MissionEventsDialog::on_btnDeleteMsg_clicked()
{
	_model->deleteMessage();

	rebuildMessageList();
	updateMessageUi();
}

void MissionEventsDialog::on_msgUpBtn_clicked()
{
	_model->moveMessageUp();
	rebuildMessageList();
	const int sel = _model->getCurrentlySelectedMessage();
	if (auto* w = ui->messageList) {
		w->setCurrentRow(sel);
		if (auto* it = w->item(sel))
			w->scrollToItem(it);
	}
	updateMessageUi();
}

void MissionEventsDialog::on_msgDownBtn_clicked()
{
	_model->moveMessageDown();
	rebuildMessageList();
	const int sel = _model->getCurrentlySelectedMessage();
	if (auto* w = ui->messageList) {
		w->setCurrentRow(sel);
		if (auto* it = w->item(sel))
			w->scrollToItem(it);
	}
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
	// TODO Build gallery from the model's known head ANIs
	const QString filters =
		"FSO Images (*.ani *.eff *.png);;All files (*.*)";
	const QString file = QFileDialog::getOpenFileName(this, tr("Select Head Animation"), QString(), filters);
	if (file.isEmpty())
		return;
	_model->setMessageAni(file.toUtf8().constData());
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

