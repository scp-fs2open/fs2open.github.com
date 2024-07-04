#include "VoiceActingManager.h"

#include "ui_VoiceActingManager.h"


namespace fso {
namespace fred {
namespace dialogs {

VoiceActingManager::VoiceActingManager(FredView* parent, EditorViewport* viewport) :
	QDialog(parent), ui(new Ui::VoiceActingManager()),
	_viewport(viewport) {
    ui->setupUi(this);
	
	// Resize the dialog to the minimum size
	resize(QDialog::sizeHint());
}

VoiceActingManager::~VoiceActingManager() {
}

}
}
}
