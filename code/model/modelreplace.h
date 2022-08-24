#pragma once

#include "globalincs/vmallocator.h"
#include "model/model.h"

#include <memory>

bool model_exists(const SCP_string& filename);

bool read_virtual_model_file(polymodel* pm, const SCP_string& filename, model_parse_depth depth, int ferror, model_read_deferred_tasks& deferredTasks);

void virtual_pof_init();

class VirtualPOFOperation {
public:
	virtual ~VirtualPOFOperation() = default;
	virtual void process(polymodel*, model_read_deferred_tasks& subsysList, model_parse_depth depth) const = 0;
};

struct VirtualPOFDefinition {
	SCP_string name, basePOF;
	std::vector<std::unique_ptr<VirtualPOFOperation>> operationList;
};

class VirtualPOFOperationRenameSubobjects : public VirtualPOFOperation {
	std::unordered_map<SCP_string, SCP_string> replacements;
public:
	VirtualPOFOperationRenameSubobjects();
	void process(polymodel* pm, model_read_deferred_tasks& deferredTasks, model_parse_depth depth) const override;
};

class VirtualPOFOperationAddSubmodel : public VirtualPOFOperation {
	SCP_string subobjNameSrc, subobjNameDest;
	SCP_string appendingPOF;
	std::unique_ptr<VirtualPOFOperationRenameSubobjects> rename = nullptr;
	bool copyChildred = true;
public:
	VirtualPOFOperationAddSubmodel();
	void process(polymodel* pm, model_read_deferred_tasks& deferredTasks, model_parse_depth depth) const override;
};

class VirtualPOFOperationChangeData : public VirtualPOFOperation {
	SCP_string submodel;
	//TODO Refactor into proper optional when available
	std::unique_ptr<vec3d> setOffset = nullptr;
public:
	VirtualPOFOperationChangeData();
	void process(polymodel* pm, model_read_deferred_tasks& deferredTasks, model_parse_depth depth) const override;
};

class VirtualPOFOperationHeaderData : public VirtualPOFOperation {
	//TODO Refactor into proper optional when available
	std::unique_ptr<float> radius = nullptr;
	std::unique_ptr<std::pair<vec3d, vec3d>> boundingbox = nullptr;
public:
	VirtualPOFOperationHeaderData();
	void process(polymodel* pm, model_read_deferred_tasks& deferredTasks, model_parse_depth depth) const override;
};