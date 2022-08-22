#pragma once

#include "globalincs/vmallocator.h"
#include "model/model.h"

bool model_exists(const SCP_string& filename);

bool model_load_virtual(polymodel* pm, const SCP_string& filename, int depth);

void virtual_pof_init();

class VirtualPOFDefinition {

};


class VirtualPOFOperation {
public:
	virtual ~VirtualPOFOperation() = default;
	virtual void process(polymodel*) const = 0;
};

class VirtualPOFOperationReplaceProps : public VirtualPOFOperation {
	SCP_string subobjName;
	SCP_string replaceWith;
public:
	VirtualPOFOperationReplaceProps();
	void process(polymodel* pm) const override;
};