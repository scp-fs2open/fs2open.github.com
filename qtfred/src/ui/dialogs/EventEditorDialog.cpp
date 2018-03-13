//
//

#include "EventEditorDialog.h"
#include "ui_EventEditorDialog.h"

#include <mission/missiongoals.h>

namespace fso {
namespace fred {
namespace dialogs {

EventEditorDialog::EventEditorDialog(QWidget* parent, EditorViewport* viewport) :
	QDialog(parent), ui(new Ui::EventEditorDialog()), _model(new EventEditorDialogModel(this, viewport)) {
	ui->setupUi(this);

	ui->eventTree->initializeEditor(viewport->editor);

	initEventTree();

	connect(this, &QDialog::accepted, _model.get(), &EventEditorDialogModel::apply);
	connect(this, &QDialog::rejected, _model.get(), &EventEditorDialogModel::reject);
}
EventEditorDialog::~EventEditorDialog() {
}
void EventEditorDialog::initEventTree() {
	load_tree();

	create_tree();

}
void EventEditorDialog::load_tree() {
	ui->eventTree->clear_tree();

	m_num_events = Num_mission_events;
	for (auto i=0; i< m_num_events; i++) {
		m_events[i] = Mission_events[i];
		if (Mission_events[i].objective_text){
			m_events[i].objective_text = strdup(Mission_events[i].objective_text);
		} else {
			m_events[i].objective_text = NULL;
		}

		if (Mission_events[i].objective_key_text){
			m_events[i].objective_key_text = strdup(Mission_events[i].objective_key_text);
		} else {
			m_events[i].objective_key_text = NULL;
		}

		m_sig[i] = i;
		if (!(*m_events[i].name)){
			strcpy_s(m_events[i].name, "<Unnamed>");
		}

		m_events[i].formula = ui->eventTree->load_sub_tree(Mission_events[i].formula, false, "do-nothing");

		// we must check for the case of the repeat count being 0.  This would happen if the repeat
		// count is not specified in a mission
		if ( m_events[i].repeat_count <= 0 ){
			m_events[i].repeat_count = 1;
		}
	}

	ui->eventTree->post_load();
}
void EventEditorDialog::create_tree() {
	ui->eventTree->clear();
	for (auto i=0; i<m_num_events; i++) {
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
}

}
}
}

