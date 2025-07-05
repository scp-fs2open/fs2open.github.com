#pragma once

#include "scripting/ade_api.h"

#include "model/model.h"

namespace scripting {
namespace api {

class modelinstance_h
{
 protected:
	int _pmi_id;

 public:
	explicit modelinstance_h(int pmi_id);
	explicit modelinstance_h(polymodel_instance *pmi);
	modelinstance_h();

	polymodel_instance *Get() const;
	int GetID() const;

	polymodel *GetModel() const;
	int GetModelID() const;

	bool isValid() const;
};
DECLARE_ADE_OBJ(l_ModelInstanceTextures, modelinstance_h);
DECLARE_ADE_OBJ(l_ModelInstance, modelinstance_h);

class submodelinstance_h
{
protected:
	int _pmi_id;
	int _submodel_num;

public:
	explicit submodelinstance_h(int pmi_id, int submodel_num);
	explicit submodelinstance_h(polymodel_instance *pmi, int submodel_num);
	submodelinstance_h();

	polymodel_instance *GetModelInstance() const;
	int GetModelInstanceID() const;

	polymodel *GetModel() const;
	int GetModelID() const;

	submodel_instance *Get() const;
	bsp_info *GetSubmodel() const;
	int GetSubmodelIndex() const;

	bool isValid() const;
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




