#pragma once

#include "scripting/ade_api.h"

#include "model/model.h"

namespace scripting {
namespace api {

//**********HANDLE: model
class model_h
{
 protected:
	polymodel *model;

 public:
	explicit model_h(int n_modelnum);
	explicit model_h(polymodel *n_model);
	model_h();

	polymodel *Get();

	int GetID();

	bool IsValid();
};
DECLARE_ADE_OBJ(l_Model, model_h);

class submodel_h
{
protected:
	polymodel *model;
	int submodel_num;

public:
	explicit submodel_h(int n_modelnum, int n_submodelnum);
	explicit submodel_h(polymodel *n_model, int n_submodelnum);
	submodel_h();

	polymodel *GetModel();
	int GetModelID();

	bsp_info *GetSubmodel();
	int GetSubmodelIndex();

	bool IsValid();
};
DECLARE_ADE_OBJ(l_Submodel, submodel_h);

class modelsubmodels_h : public model_h
{
 public:
	 modelsubmodels_h(polymodel *pm);
	 modelsubmodels_h();
};
DECLARE_ADE_OBJ(l_ModelSubmodels, modelsubmodels_h);

class modeltextures_h : public model_h
{
 public:
	modeltextures_h(polymodel *pm);
	modeltextures_h();
};
DECLARE_ADE_OBJ(l_ModelTextures, modeltextures_h);

class eyepoints_h : public model_h
{
 public:
	eyepoints_h(polymodel *pm);
	eyepoints_h();
};
DECLARE_ADE_OBJ(l_Eyepoints, eyepoints_h);

// Thrusters:
class thrusters_h : public model_h
{
 public:
	thrusters_h(polymodel *pm);
	thrusters_h();
};
DECLARE_ADE_OBJ(l_Thrusters, thrusters_h);

// Thrusterbank:
struct thrusterbank_h
{
	thruster_bank *bank;

	thrusterbank_h();

	thrusterbank_h(thruster_bank* ba);

	thruster_bank *Get();

	bool isValid();
};
DECLARE_ADE_OBJ(l_Thrusterbank, thrusterbank_h);

// Glowpoint:
struct glowpoint_h
{
	glow_point *point;

	glowpoint_h();

	glowpoint_h(glow_point* np);

	glow_point* Get();

	bool isValid();

};
DECLARE_ADE_OBJ(l_Glowpoint, glowpoint_h);

// Glowbanks:
class dockingbays_h : public model_h
{
 public:
	dockingbays_h(polymodel *pm);
	dockingbays_h();
};
DECLARE_ADE_OBJ(l_Dockingbays, dockingbays_h);

class dockingbay_h : public model_h
{
 private:
	int dock_id;

 public:
	dockingbay_h(polymodel *pm, int dock_idx);
	dockingbay_h();

	bool IsValid();

	dock_bay* getDockingBay();
};
DECLARE_ADE_OBJ(l_Dockingbay, dockingbay_h);


}
}




