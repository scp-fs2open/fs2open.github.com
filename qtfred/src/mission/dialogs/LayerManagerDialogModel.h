#pragma once

#include "AbstractDialogModel.h"

namespace fso::fred::dialogs {

class LayerManagerDialogModel : public AbstractDialogModel {
	Q_OBJECT

public:
	LayerManagerDialogModel(QObject* parent, EditorViewport* viewport);
	~LayerManagerDialogModel() override = default;

	bool apply() override;
	void reject() override;

	// Layers
	SCP_vector<SCP_string> getLayerNames() const;
	bool getLayerVisibility(const SCP_string& name) const;
	bool setLayerVisibility(const SCP_string& name, bool visible, SCP_string* error);
	bool addLayer(const SCP_string& name, SCP_string* error);
	bool deleteLayer(const SCP_string& name, SCP_string* error);
	static bool isDefaultLayer(const SCP_string& name);

	// Object type filters
	bool getShowShips() const;
	void setShowShips(bool value);
	bool getShowStarts() const;
	void setShowStarts(bool value);
	bool getShowWaypoints() const;
	void setShowWaypoints(bool value);

	// IFF team filters
	static int getIffCount();
	static SCP_string getIffName(int index);
	bool getShowIff(int index) const;
	void setShowIff(int index, bool value);
};

} // namespace fso::fred::dialogs
