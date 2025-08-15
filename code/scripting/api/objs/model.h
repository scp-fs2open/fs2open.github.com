#pragma once

#include "scripting/ade_api.h"

#include "model/model.h"

namespace scripting {
namespace api {

//**********HANDLE: model
class model_h
{
 protected:
	int model_num;

 public:
	explicit model_h(int n_modelnum);
	explicit model_h(polymodel *n_model);
	model_h();

	polymodel *Get() const;
	int GetID() const;

	bool isValid() const;
};
DECLARE_ADE_OBJ(l_Model, model_h);

class submodel_h
{
protected:
	int model_num;
	int submodel_num;

public:
	explicit submodel_h(int n_modelnum, int n_submodelnum);
	explicit submodel_h(polymodel *n_model, int n_submodelnum);
	submodel_h();

	polymodel *GetModel() const;
	int GetModelID() const;

	bsp_info *GetSubmodel() const;
	int GetSubmodelIndex() const;

	bool isValid() const;
};
DECLARE_ADE_OBJ(l_Submodel, submodel_h);

DECLARE_ADE_OBJ(l_ModelSubmodels, model_h);

DECLARE_ADE_OBJ(l_ModelTextures, model_h);

DECLARE_ADE_OBJ(l_ModelEyepoints, model_h);

// Thrusters:
DECLARE_ADE_OBJ(l_ModelThrusters, model_h);

// Thrusterbank:
struct thrusterbank_h
{
	model_h modelh;
	int thrusterbank_index;

	thrusterbank_h();
	thrusterbank_h(int in_model_num, int in_thrusterbank_index);

	thruster_bank *Get() const;

	bool isValid() const;
};
DECLARE_ADE_OBJ(l_Thrusterbank, thrusterbank_h);

// Glow points:
DECLARE_ADE_OBJ(l_ModelGlowpointbanks, model_h);

// Glowpointbank:
struct glowpointbank_h
{
	model_h modelh;
	int glowpointbank_index;

	glowpointbank_h();
	glowpointbank_h(int in_model_num, int in_glowpointbank_index);

	glow_point_bank *Get() const;

	bool isValid() const;
};
DECLARE_ADE_OBJ(l_Glowpointbank, glowpointbank_h);

// Glowpoint:
struct glowpoint_h
{
	glowpointbank_h glowpointbankh;
	thrusterbank_h thrusterbankh;
	int glowpoint_index;

	glowpoint_h();
	glowpoint_h(int in_model_num, int in_glowpointbank_index, int in_thrusterbank_index, int in_glowpoint_index);

	glow_point* Get() const;

	bool isValid() const;
};
DECLARE_ADE_OBJ(l_Glowpoint, glowpoint_h);

// Docking bays:
DECLARE_ADE_OBJ(l_ModelDockingbays, model_h);

class dockingbay_h
{
 private:
	model_h modelh;
	int dock_id;

 public:
	dockingbay_h(polymodel *pm, int dock_idx);
	dockingbay_h();

	dock_bay* getDockingBay() const;

	bool isValid() const;
};
DECLARE_ADE_OBJ(l_Dockingbay, dockingbay_h);


}
}




