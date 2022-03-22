#pragma once

#include "scripting/ade_api.h"

#include "model/model.h"

namespace scripting {
namespace api {

class modelinstance_h
{
 protected:
	polymodel_instance *_pmi;

 public:
	explicit modelinstance_h(int pmi_id);
	explicit modelinstance_h(polymodel_instance *pmi);
	modelinstance_h();

	polymodel_instance *Get();

	bool IsValid();
};
DECLARE_ADE_OBJ(l_ModelInstance, modelinstance_h);

class submodelinstance_h
{
protected:
	polymodel_instance *_pmi;
	polymodel *_pm;
	int _submodel_num;

public:
	explicit submodelinstance_h(int pmi_id, int submodel_num);
	explicit submodelinstance_h(polymodel_instance *pmi, int submodel_num);
	submodelinstance_h();

	polymodel_instance *GetModelInstance();
	submodel_instance *Get();

	polymodel *GetModel();
	bsp_info *GetSubmodel();
	int GetSubmodelIndex();

	bool IsValid();
};
DECLARE_ADE_OBJ(l_SubmodelInstance, submodelinstance_h);

class modelsubmodelinstances_h : public modelinstance_h
{
 public:
	 modelsubmodelinstances_h(polymodel_instance *pmi);
	 modelsubmodelinstances_h();
};
DECLARE_ADE_OBJ(l_ModelSubmodelInstances, modelsubmodelinstances_h);

}
}




