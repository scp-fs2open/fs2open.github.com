#pragma once

#include "globalincs/vmallocator.h"
#include "model/model.h"

bool model_exists(const SCP_string& filename);

bool read_virtual_model_file(polymodel* pm, const SCP_string& filename, int depth, int ferror, model_read_deferred_tasks& deferredTasks);

void virtual_pof_init();

class VirtualPOFOperation {
public:
	virtual ~VirtualPOFOperation() = default;
	virtual void process(polymodel*, model_read_deferred_tasks& subsysList, int depth) const = 0;
};

struct VirtualPOFDefinition {
	SCP_string name, basePOF;
	std::vector<std::unique_ptr<VirtualPOFOperation>> operationList;
};

class VirtualPOFOperationAddSubmodel : public VirtualPOFOperation {
	SCP_string subobjNameSrc, subobjNameDest;
	SCP_string appendingPOF;
public:
	VirtualPOFOperationAddSubmodel();
	void process(polymodel* pm, model_read_deferred_tasks& subsysList, int depth) const override;
};