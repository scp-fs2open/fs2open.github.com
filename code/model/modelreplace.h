#pragma once

#include "globalincs/vmallocator.h"
#include "model/model.h"

#include <memory>

#include <tl/optional.hpp>
#include <mpark/variant.hpp>

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

using VirtualPOFNameReplacementMap = SCP_unordered_map<SCP_string, SCP_string, SCP_string_lcase_hash, SCP_string_lcase_equal_to>;

class VirtualPOFOperationRenameSubobjects : public VirtualPOFOperation {
	VirtualPOFNameReplacementMap replacements;
	friend class VirtualPOFOperationAddSubmodel;
	friend class VirtualPOFOperationAddTurret;
public:
	VirtualPOFOperationRenameSubobjects();
	void process(polymodel* pm, model_read_deferred_tasks& deferredTasks, model_parse_depth depth, const VirtualPOFDefinition& virtualPof) const override;
};

class VirtualPOFOperationAddSubmodel : public VirtualPOFOperation {
	SCP_string subobjNameSrc, subobjNameDest;
	SCP_string appendingPOF;
	std::unique_ptr<VirtualPOFOperationRenameSubobjects> rename = nullptr;
	bool copyChildren = true;
	bool copyTurrets = false;
	bool copyGlowpoints = false;
public:
	VirtualPOFOperationAddSubmodel();
	void process(polymodel* pm, model_read_deferred_tasks& deferredTasks, model_parse_depth depth, const VirtualPOFDefinition& virtualPof) const override;
};

class VirtualPOFOperationAddTurret : public VirtualPOFOperation {
	SCP_string baseNameSrc, baseNameDest;
	tl::optional<SCP_string> barrelNameDest;
	SCP_string appendingPOF;
	std::unique_ptr<VirtualPOFOperationRenameSubobjects> rename = nullptr;
public:
	VirtualPOFOperationAddTurret();
	void process(polymodel* pm, model_read_deferred_tasks& deferredTasks, model_parse_depth depth, const VirtualPOFDefinition& virtualPof) const override;
};

class VirtualPOFOperationAddEngine : public VirtualPOFOperation {
	mpark::variant<SCP_string, int> sourceId;
	tl::optional<SCP_string> renameSubsystem;
	tl::optional<vec3d> moveEngine;
	SCP_string appendingPOF;
public:
	VirtualPOFOperationAddEngine();
	void process(polymodel* pm, model_read_deferred_tasks& deferredTasks, model_parse_depth depth, const VirtualPOFDefinition& virtualPof) const override;
};

class VirtualPOFOperationAddGlowpoint : public VirtualPOFOperation {
	int sourceId;
	SCP_string renameSubmodel;
	tl::optional<vec3d> moveGlowpoint;
	SCP_string appendingPOF;
public:
	VirtualPOFOperationAddGlowpoint();
	void process(polymodel* pm, model_read_deferred_tasks& deferredTasks, model_parse_depth depth, const VirtualPOFDefinition& virtualPof) const override;
};

class VirtualPOFOperationAddSpecialSubsystem : public VirtualPOFOperation {
	SCP_string sourceSubsystem;
	tl::optional<SCP_string> renameSubsystem;
	SCP_string appendingPOF;
public:
	VirtualPOFOperationAddSpecialSubsystem();
	void process(polymodel* pm, model_read_deferred_tasks& deferredTasks, model_parse_depth depth, const VirtualPOFDefinition& virtualPof) const override;
};

class VirtualPOFOperationAddWeapons : public VirtualPOFOperation {
	int sourcebank, destbank = -1;
	bool primary;
	SCP_string appendingPOF;
public:
	VirtualPOFOperationAddWeapons();
	void process(polymodel* pm, model_read_deferred_tasks& deferredTasks, model_parse_depth depth, const VirtualPOFDefinition& virtualPof) const override;
};

class VirtualPOFOperationAddDockPoint : public VirtualPOFOperation {
	SCP_string sourcedock;
	tl::optional<SCP_string> renameDock;
	SCP_unordered_map<SCP_string, SCP_string> renamePaths;
	tl::optional<SCP_string> targetParentSubsystem;
	SCP_string appendingPOF;
public:
	VirtualPOFOperationAddDockPoint();
	void process(polymodel* pm, model_read_deferred_tasks& deferredTasks, model_parse_depth depth, const VirtualPOFDefinition& virtualPof) const override;
};

class VirtualPOFOperationAddPath : public VirtualPOFOperation {
	SCP_string sourcepath;
	tl::optional<SCP_string> renamePath;
	tl::optional<SCP_string> targetParentSubsystem;
	SCP_string appendingPOF;
public:
	VirtualPOFOperationAddPath();
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