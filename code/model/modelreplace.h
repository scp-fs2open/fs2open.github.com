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
	virtual void process(polymodel*) const = 0;
};

class VirtualPOFOperationReplaceProps : public VirtualPOFOperation {
public:
	void process(polymodel* pm) const override;
};