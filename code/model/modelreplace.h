#pragma once

#include "globalincs/vmallocator.h"
#include "model/model.h"

bool model_exists(const SCP_string& filename);

bool read_virtual_model_file(polymodel* pm, const SCP_string& filename, int depth);

void virtual_pof_init();

class VirtualPOFDefinition {

};


class VirtualPOFOperation {
public:
	virtual ~VirtualPOFOperation() = default;
	virtual void process(polymodel*, subsystem_parse_list& subsysList, int depth) const = 0;
};

class VirtualPOFOperationAddSubmodel : public VirtualPOFOperation {
	SCP_string subobjNameSrc, subobjNameDest;
	SCP_string appendingPOF;
public:
	VirtualPOFOperationAddSubmodel();
	void process(polymodel* pm, subsystem_parse_list& subsysList, int depth) const override;
};