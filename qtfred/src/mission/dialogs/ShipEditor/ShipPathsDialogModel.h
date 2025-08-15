#pragma once
#include "../AbstractDialogModel.h"
namespace fso {
namespace fred {
namespace dialogs {

class ShipPathsDialogModel : public AbstractDialogModel {
  private:
	bool departureMode;
	void initalizeData(const int ship, const int target_class, const bool departure);
	polymodel* m_model;
	int m_num_paths;
	SCP_vector<bool> m_path_list;
	int m_path_mask;
	int m_ship;
  public:
	ShipPathsDialogModel(QObject* parent,
		EditorViewport* viewport,
		const int ship,
		const int target_class, const bool departure = false);
	bool apply() override;
	void reject() override;
	bool modify(const int, const bool);
	SCP_vector<bool> getPathList() const;
	polymodel* getModel() const;
};
} // namespace dialogs
} // namespace fred
} // namespace fso