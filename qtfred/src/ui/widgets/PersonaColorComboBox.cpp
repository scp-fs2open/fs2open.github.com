#include "PersonaColorComboBox.h"
#include <mission/missionmessage.h>
#include <species_defs/species_defs.h>

namespace fso::fred {
PersonaColorComboBox::PersonaColorComboBox(QWidget* parent) : QComboBox(parent)
{
	fredApp->runAfterInit([this]() { setModel(getPersonaModel()); });
}
QStandardItemModel* PersonaColorComboBox::getPersonaModel()
{
	auto itemModel = new QStandardItemModel();
	auto topitem = new QStandardItem("<none>");
	topitem->setData(-1, Qt::UserRole);
	itemModel->appendRow(topitem);
	for (size_t i = 0; i < Personas.size(); i++) {
		if (Personas[i].flags & PERSONA_FLAG_WINGMAN) {
			SCP_string persona_name = Personas[i].name;

			// see if the bitfield matches one and only one species
			int species = -1;
			for (size_t j = 0; j < 32 && j < Species_info.size(); j++) {
				if (Personas[i].species_bitfield == (1 << j)) {
					species = static_cast<int>(j);
					break;
				}
			}
			auto item = new QStandardItem(persona_name.c_str());
			// if it is an exact species that isn't the first
			if (species >= 0) {
				species_info* sinfo = &Species_info[species];
				auto brush = QBrush(QColor(sinfo->fred_color.rgb.r, sinfo->fred_color.rgb.g, sinfo->fred_color.rgb.b));
				item->setData(brush, Qt::ForegroundRole);
				item->setData(static_cast<int>(i), Qt::UserRole);
				itemModel->appendRow(item);
			}
		}
	}
	return itemModel;
}
} // namespace fso::fred