#pragma once

#include "globalincs/vmallocator.h"
#include "model/model.h"

#include <memory>

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

class VirtualPOFOperationRenameSubobjects : public VirtualPOFOperation {
	std::unordered_map<SCP_string, SCP_string> replacements;
public:
	VirtualPOFOperationRenameSubobjects();
	void process(polymodel* pm, model_read_deferred_tasks& deferredTasks, int depth) const override;
};

class VirtualPOFOperationAddSubmodel : public VirtualPOFOperation {
	SCP_string subobjNameSrc, subobjNameDest;
	SCP_string appendingPOF;
	std::unique_ptr<VirtualPOFOperationRenameSubobjects> rename = nullptr;
public:
	VirtualPOFOperationAddSubmodel();
	void process(polymodel* pm, model_read_deferred_tasks& deferredTasks, int depth) const override;
};
