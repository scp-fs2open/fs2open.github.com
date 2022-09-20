#pragma once

#include "globalincs/vmallocator.h"
#include "model/model.h"

#include <memory>

#include <tl/optional.hpp>

bool model_exists(const SCP_string& filename);
bool read_virtual_model_file(polymodel* pm, const SCP_string& filename, model_parse_depth depth, int ferror, model_read_deferred_tasks& deferredTasks);
void virtual_pof_purge_cache();

void virtual_pof_init();


struct VirtualPOFDefinition;

class VirtualPOFOperation {
public:
	virtual ~VirtualPOFOperation() = default;
	virtual void process(polymodel* pm, model_read_deferred_tasks& subsysList, model_parse_depth depth, const VirtualPOFDefinition& virtualPof) const = 0;
};

struct VirtualPOFDefinition {
	SCP_string name, basePOF;
	SCP_vector<std::unique_ptr<VirtualPOFOperation>> operationList;
};

class VirtualPOFOperationRenameSubobjects : public VirtualPOFOperation {
	using replacement_map = SCP_unordered_map<SCP_string, SCP_string, SCP_string_lcase_hash, SCP_string_lcase_equal_to>;
	replacement_map replacements;
	friend class VirtualPOFOperationAddSubmodel;
public:
	VirtualPOFOperationRenameSubobjects();
	void process(polymodel* pm, model_read_deferred_tasks& deferredTasks, model_parse_depth depth, const VirtualPOFDefinition& virtualPof) const override;
};

class VirtualPOFOperationAddSubmodel : public VirtualPOFOperation {
	SCP_string subobjNameSrc, subobjNameDest;
	SCP_string appendingPOF;
	std::unique_ptr<VirtualPOFOperationRenameSubobjects> rename = nullptr;
	bool copyChildren = true;
public:
	VirtualPOFOperationAddSubmodel();
	void process(polymodel* pm, model_read_deferred_tasks& deferredTasks, model_parse_depth depth, const VirtualPOFDefinition& virtualPof) const override;
};

class VirtualPOFOperationChangeData : public VirtualPOFOperation {
	SCP_string submodel;
	tl::optional<vec3d> setOffset = tl::nullopt;
public:
	VirtualPOFOperationChangeData();
	void process(polymodel* pm, model_read_deferred_tasks& deferredTasks, model_parse_depth depth, const VirtualPOFDefinition& virtualPof) const override;
};

class VirtualPOFOperationHeaderData : public VirtualPOFOperation {
	tl::optional<float> radius = tl::nullopt;
	tl::optional<std::pair<vec3d, vec3d>> boundingbox = tl::nullopt;
public:
	VirtualPOFOperationHeaderData();
	void process(polymodel* pm, model_read_deferred_tasks& deferredTasks, model_parse_depth depth, const VirtualPOFDefinition& virtualPof) const override;
};