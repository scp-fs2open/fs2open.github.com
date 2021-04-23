#ifndef CAMPAIGNEDITORDIALOGMODEL_H
#define CAMPAIGNEDITORDIALOGMODEL_H

#include "mission/dialogs/AbstractDialogModel.h"

namespace fso {
namespace fred {
namespace dialogs {

class CampaignEditorDialogModel : public AbstractDialogModel
{
	Q_OBJECT
public:
	explicit CampaignEditorDialogModel(QObject* parent, EditorViewport* viewport);
	~CampaignEditorDialogModel() override = default;
	bool apply() override;

	void reject() override;

	bool query_modified() const { return modified; }
private slots:
	void flagModified() { modified = true;}

private:
	void initializeData();

	template<typename T>
	void modify(T &a, const T &b);

	bool modified = false;
};


template<typename T>
inline void CampaignEditorDialogModel::modify(T &a, const T &b) {
	if (a != b) {
		a = b;
		modelChanged();
	}
}

}
}
}
#endif // CAMPAIGNEDITORDIALOGMODEL_H
