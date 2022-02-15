//
//

#include "model.h"
#include "vecmath.h"
#include "eye.h"
#include "texture.h"

extern void model_calc_bound_box( vec3d *box, vec3d *big_mn, vec3d *big_mx);

namespace scripting {
namespace api {

ADE_OBJ(l_Model, model_h, "model", "3D Model (POF) handle");

polymodel* model_h::Get() {
	if(!this->IsValid())
		return NULL;

	return model;
}
int model_h::GetID() {
	if(!this->IsValid())
		return -1;

	return mid;
}
bool model_h::IsValid() {
	return (model != NULL && mid > -1 && model_get(mid) == model);
}
model_h::model_h(int n_modelnum) {
	mid = n_modelnum;
	if(mid > 0)
		model = model_get(mid);
	else
		model = NULL;
}
model_h::model_h(polymodel* n_model) {
	model = n_model;
	if(n_model != NULL)
		mid = n_model->id;
	else
		mid = -1;
}
model_h::model_h() {
	mid=-1;
	model = NULL;
}

ADE_VIRTVAR(Textures, l_Model, "modeltextures_h", "Model textures", "modeltextures_h", "Model textures, or an invalid modeltextures handle if the model handle is invalid")
{
	model_h *mdl = NULL;
	modeltextures_h *oth = NULL;
	if(!ade_get_args(L, "o|o", l_Model.GetPtr(&mdl), l_ModelTextures.GetPtr(&oth)))
		return ade_set_error(L, "o", l_ModelTextures.Set(modeltextures_h()));

	polymodel *pm = mdl->Get();
	if(pm == NULL)
		return ade_set_error(L, "o", l_ModelTextures.Set(modeltextures_h()));

	if(ADE_SETTING_VAR && oth && oth->IsValid()) {
		//WMC TODO: Copy code
		LuaError(L, "Attempt to use Incomplete Feature: Modeltextures copy");
	}

	return ade_set_args(L, "o", l_ModelTextures.Set(modeltextures_h(pm)));
}

ADE_VIRTVAR(Thrusters, l_Model, "thrusters", "Model thrusters", "thrusters", "Thrusters of the model or invalid handle")
{
	model_h *mdl = NULL;
	thrusters_h *oth = NULL;
	if(!ade_get_args(L, "o|o", l_Model.GetPtr(&mdl), l_Thrusters.GetPtr(&oth)))
		return ade_set_error(L, "o", l_Thrusters.Set(thrusters_h()));

	polymodel *pm = mdl->Get();
	if(pm == NULL)
		return ade_set_error(L, "o", l_Thrusters.Set(thrusters_h()));

	if(ADE_SETTING_VAR && oth && oth->IsValid()) {
		LuaError(L, "Attempt to use Incomplete Feature: Thrusters copy");
	}

	return ade_set_args(L, "o", l_Thrusters.Set(thrusters_h(pm)));
}

ADE_VIRTVAR(Eyepoints, l_Model, "eyepoints", "Model eyepoints", "eyepoints", "Array of eyepoints or invalid handle on error")
{
	model_h *mdl = NULL;
	eyepoints_h *eph = NULL;
	if(!ade_get_args(L, "o|o", l_Model.GetPtr(&mdl), l_Eyepoints.GetPtr(&eph)))
		return ade_set_error(L, "o", l_Eyepoints.Set(eyepoints_h()));

	polymodel *pm = mdl->Get();
	if(pm == NULL)
		return ade_set_error(L, "o", l_Eyepoints.Set(eyepoints_h()));

	if(ADE_SETTING_VAR && eph && eph->IsValid()) {
		LuaError(L, "Attempt to use Incomplete Feature: Eyepoints copy");
	}

	return ade_set_args(L, "o", l_Eyepoints.Set(eyepoints_h(pm)));
}

ADE_VIRTVAR(Dockingbays, l_Model, "dockingbays", "Docking bays handle of model", "dockingbays", "Array of docking bays on this model, or invalid handle on error")
{
	model_h *mdl = NULL;
	dockingbays_h *dbh = NULL;
	if(!ade_get_args(L, "o|o", l_Model.GetPtr(&mdl), l_Dockingbays.GetPtr(&dbh)))
		return ade_set_error(L, "o", l_Dockingbays.Set(dockingbays_h()));

	polymodel *pm = mdl->Get();
	if(pm == NULL)
		return ade_set_error(L, "o", l_Dockingbays.Set(dockingbays_h()));

	if(ADE_SETTING_VAR && dbh && dbh->IsValid()) {
		LuaError(L, "Attempt to use Incomplete Feature: Docking bays copy");
	}

	return ade_set_args(L, "o", l_Dockingbays.Set(dockingbays_h(pm)));
}

ADE_VIRTVAR(BoundingBoxMax, l_Model, "vector", "Model bounding box maximum", "vector", "Model bounding box, or an empty vector if the handle is invalid")
{
	model_h *mdl = NULL;
	vec3d *v = NULL;
	if(!ade_get_args(L, "o|o", l_Model.GetPtr(&mdl), l_Vector.GetPtr(&v)))
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));

	polymodel *pm = mdl->Get();

	if(pm == NULL)
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));

	if(ADE_SETTING_VAR && v != NULL) {
		pm->maxs = *v;

		//Recalculate this, so it stays valid
		model_calc_bound_box(pm->bounding_box, &pm->mins, &pm->maxs);
	}

	return ade_set_args(L, "o", l_Vector.Set(pm->maxs));
}

ADE_VIRTVAR(BoundingBoxMin, l_Model, "vector", "Model bounding box minimum", "vector", "Model bounding box, or an empty vector if the handle is invalid")
{
	model_h *mdl = NULL;
	vec3d *v = NULL;
	if(!ade_get_args(L, "o|o", l_Model.GetPtr(&mdl), l_Vector.GetPtr(&v)))
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));

	polymodel *pm = mdl->Get();

	if(pm == NULL)
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));

	if(ADE_SETTING_VAR && v != NULL) {
		pm->mins = *v;

		//Recalculate this, so it stays valid
		model_calc_bound_box(pm->bounding_box, &pm->mins, &pm->maxs);
	}

	return ade_set_args(L, "o", l_Vector.Set(pm->mins));
}

ADE_VIRTVAR(Filename, l_Model, "string", "Model filename", "string", "Model filename, or an empty string if the handle is invalid")
{
	model_h *mdl = NULL;
	const char* s = nullptr;
	if(!ade_get_args(L, "o|s", l_Model.GetPtr(&mdl), &s))
		return ade_set_error(L, "s", "");

	polymodel *pm = mdl->Get();

	if(pm == NULL)
		return ade_set_error(L, "s", "");

	if(ADE_SETTING_VAR) {
		auto len = sizeof(pm->filename);
		strncpy(pm->filename, s, len);
		pm->filename[len - 1] = 0;
	}

	return ade_set_args(L, "s", pm->filename);
}

ADE_VIRTVAR(Mass, l_Model, "number", "Model mass", "number", "Model mass, or 0 if the model handle is invalid")
{
	model_h *mdl = NULL;
	float nm = 0.0f;
	if(!ade_get_args(L, "o|f", l_Model.GetPtr(&mdl), &nm))
		return ade_set_error(L, "f", 0.0f);

	polymodel *pm = mdl->Get();

	if(pm == NULL)
		return ade_set_error(L, "f", 0.0f);

	if(ADE_SETTING_VAR) {
		pm->mass = nm;
	}

	return ade_set_args(L, "f", pm->mass);
}

ADE_VIRTVAR(MomentOfInertia, l_Model, "orientation", "Model moment of inertia", "orientation", "Moment of Inertia matrix or identity matrix if invalid" )
{
	model_h *mdl = NULL;
	matrix_h *mh = NULL;
	if(!ade_get_args(L, "o|o", l_Model.GetPtr(&mdl), l_Matrix.GetPtr(&mh)))
		return ade_set_error(L, "o", l_Matrix.Set(matrix_h()));

	polymodel *pm = mdl->Get();

	if(pm == NULL)
		return ade_set_error(L, "o", l_Matrix.Set(matrix_h()));

	if(ADE_SETTING_VAR && mh != NULL) {
		matrix *mtx = mh->GetMatrix();
		memcpy(&pm->moment_of_inertia, mtx, sizeof(*mtx));
	}

	return ade_set_args(L, "o", l_Matrix.Set(matrix_h(&pm->moment_of_inertia)));
}

ADE_VIRTVAR(Radius, l_Model, "number", "Model radius (Used for collision & culling detection)", "number", "Model Radius or 0 if invalid")
{
	model_h *mdl = NULL;
	float nr = 0.0f;
	if(!ade_get_args(L, "o|f", l_Model.GetPtr(&mdl), &nr))
		return ade_set_error(L, "f", 0.0f);

	polymodel *pm = mdl->Get();

	if(pm == NULL)
		return ade_set_error(L, "f", 0.0f);

	if(ADE_SETTING_VAR) {
		pm->rad = nr;
	}

	return ade_set_args(L, "f", pm->rad);
}

ADE_FUNC(isValid, l_Model, nullptr, "True if valid, false or nil if not", "boolean", "Detects whether handle is valid")
{
	model_h *mdl;
	if(!ade_get_args(L, "o", l_Model.GetPtr(&mdl)))
		return ADE_RETURN_NIL;

	return ade_set_args(L, "b", mdl->IsValid());
}


//**********HANDLE: modeltextures
ADE_OBJ(l_ModelTextures, modeltextures_h, "modeltextures_h", "Array of materials");

modeltextures_h::modeltextures_h(polymodel* pm) : model_h(pm){}
modeltextures_h::modeltextures_h() : model_h(){}

ADE_FUNC(__len, l_ModelTextures, NULL, "Number of textures on model", "number", "Number of model textures")
{
	modeltextures_h *mth;
	if(!ade_get_args(L, "o", l_ModelTextures.GetPtr(&mth)))
		return ade_set_error(L, "i", 0);

	if(!mth->IsValid())
		return ade_set_error(L, "i", 0);

	polymodel *pm = mth->Get();

	if(pm == NULL)
		return ade_set_error(L, "i", 0);

	return ade_set_args(L, "i", TM_NUM_TYPES*pm->n_textures);
}

ADE_INDEXER(l_ModelTextures, "texture", "number Index/string TextureName", "texture", "Model textures, or invalid modeltextures handle if model handle is invalid")
{
	modeltextures_h *mth = NULL;
	texture_h* new_tex   = nullptr;
	const char* s        = nullptr;

	if (!ade_get_args(L, "os|o", l_ModelTextures.GetPtr(&mth), &s, l_Texture.GetPtr(&new_tex)))
		return ade_set_error(L, "o", l_Texture.Set(texture_h()));

	polymodel *pm = mth->Get();

	if (!mth->IsValid() || s == NULL || pm == NULL)
		return ade_set_error(L, "o", l_Texture.Set(texture_h()));

	texture_info *tinfo = NULL;
	texture_map *tmap = NULL;

	if(strspn(s, "0123456789") == strlen(s))
	{
		int num_textures = TM_NUM_TYPES*pm->n_textures;
		int idx = atoi(s) - 1;	//Lua->FS2

		if (idx < 0 || idx >= num_textures)
			return ade_set_error(L, "o", l_Texture.Set(texture_h()));

		tmap = &pm->maps[idx / TM_NUM_TYPES];
		tinfo = &tmap->textures[idx % TM_NUM_TYPES];
	}

	if(tinfo == NULL)
	{
		for (int i = 0; i < pm->n_textures; i++)
		{
			tmap = &pm->maps[i];

			int tnum = tmap->FindTexture(s);
			if(tnum > -1)
				tinfo = &tmap->textures[tnum];
		}
	}

	if(tinfo == NULL)
		return ade_set_error(L, "o", l_Texture.Set(texture_h()));

	if (ADE_SETTING_VAR && new_tex != nullptr) {
		tinfo->SetTexture(new_tex->handle);
	}

	return ade_set_args(L, "o", l_Texture.Set(texture_h(tinfo->GetTexture())));
}

ADE_FUNC(isValid, l_ModelTextures, NULL, "Detects whether handle is valid", "boolean", "true if valid, false if handle is invalid, nil if a syntax/type error occurs")
{
	modeltextures_h *mth;
	if(!ade_get_args(L, "o", l_ModelTextures.GetPtr(&mth)))
		return ADE_RETURN_NIL;

	return ade_set_args(L, "b", mth->IsValid());
}


//**********HANDLE: eyepoints
ADE_OBJ(l_Eyepoints, eyepoints_h, "eyepoints", "Array of model eye points");

eyepoints_h::eyepoints_h(polymodel* pm) : model_h(pm){}
eyepoints_h::eyepoints_h() : model_h(){}

ADE_FUNC(__len, l_Eyepoints, NULL, "Gets the number of eyepoints on this model", "number", "Number of eyepoints on this model or 0 on error")
{
	eyepoints_h *eph = NULL;
	if (!ade_get_args(L, "o", l_Eyepoints.GetPtr(&eph)))
	{
		return ade_set_error(L, "i", 0);
	}

	if (!eph->IsValid())
	{
		return ade_set_error(L, "i", 0);
	}

	polymodel *pm = eph->Get();

	if (pm == NULL)
	{
		return ade_set_error(L, "i", 0);
	}

	return ade_set_args(L, "i", pm->n_view_positions);
}

ADE_INDEXER(l_Eyepoints, "eyepoint", "Gets an eyepoint handle", "eyepoint", "eye handle or invalid handle on error")
{
	eyepoints_h *eph = NULL;
	int index = -1;
	eye_h *eh = NULL;

	if (!ade_get_args(L, "oi|o", l_Eyepoints.GetPtr(&eph), &index, l_Eyepoint.GetPtr(&eh)))
	{
		return ade_set_error(L, "o", l_Eyepoint.Set(eye_h()));
	}

	if (!eph->IsValid())
	{
		return ade_set_error(L, "o", l_Eyepoint.Set(eye_h()));
	}

	polymodel *pm = eph->Get();

	if (pm == NULL)
	{
		return ade_set_error(L, "o", l_Eyepoint.Set(eye_h()));
	}

	index--; // Lua -> FS2

	if (index < 0 || index >= pm->n_view_positions)
	{
		return ade_set_error(L, "o", l_Eyepoint.Set(eye_h()));
	}

	if (ADE_SETTING_VAR && eh && eh->IsValid())
	{
		LuaError(L, "Attempted to use incomplete feature: Eyepoint copy");
	}

	return ade_set_args(L, "o", l_Eyepoint.Set(eye_h(eph->GetID(), index)));
}

ADE_FUNC(isValid, l_Eyepoints, NULL, "Detects whether handle is valid or not", "boolean", "true if valid false otherwise")
{
	eyepoints_h *eph;
	if(!ade_get_args(L, "o", l_Eyepoints.GetPtr(&eph)))
		return ADE_RETURN_FALSE;

	return ade_set_args(L, "b", eph->IsValid());
}

//**********HANDLE: thrusters
ADE_OBJ(l_Thrusters, thrusters_h, "thrusters", "The thrusters of a model");

thrusters_h::thrusters_h(polymodel* pm) : model_h(pm){}
thrusters_h::thrusters_h() : model_h(){}

ADE_FUNC(__len, l_Thrusters, NULL, "Number of thruster banks on the model", "number", "Number of thrusterbanks")
{
	thrusters_h *trh;
	if(!ade_get_args(L, "o", l_Thrusters.GetPtr(&trh)))
		return ade_set_error(L, "i", -1);

	if(!trh->IsValid())
		return ade_set_error(L, "i", -1);

	polymodel *pm = trh->Get();

	if(pm == NULL)
		return ade_set_error(L, "i", -1);

	return ade_set_args(L, "i", pm->n_thrusters);
}

ADE_INDEXER(l_Thrusters, "number Index", "Array of all thrusterbanks on this thruster", "thrusterbank", "Handle to the thrusterbank or invalid handle if index is invalid")
{
	thrusters_h *trh = NULL;
	const char* s    = nullptr;
	thrusterbank_h newThr;

	if (!ade_get_args(L, "os|o", l_Thrusters.GetPtr(&trh), &s, l_Thrusterbank.Get(&newThr)))
		return ade_set_error(L, "o", l_Thrusterbank.Set(thrusterbank_h()));

	polymodel *pm = trh->Get();

	if (!trh->IsValid() || s == NULL || pm == NULL)
		return ade_set_error(L, "o", l_Thrusterbank.Set(thrusterbank_h()));

	//Determine index
	int idx = atoi(s) - 1;	//Lua->FS2

	if (idx < 0 || idx >= pm->n_thrusters)
		return ade_set_error(L, "o", l_Thrusterbank.Set(thrusterbank_h()));

	thruster_bank* bank = &pm->thrusters[idx];

	if (ADE_SETTING_VAR && trh != NULL)
	{
		if (newThr.isValid())
		{
			pm->thrusters[idx] = *(newThr.Get());
		}
	}

	return ade_set_args(L, "o", l_Thrusterbank.Set(bank));
}

ADE_FUNC(isValid, l_Thrusters, NULL, "Detects whether handle is valid", "boolean", "true if valid, false if handle is invalid, nil if a syntax/type error occurs")
{
	thrusters_h *trh;
	if(!ade_get_args(L, "o", l_Thrusters.GetPtr(&trh)))
		return ADE_RETURN_NIL;

	return ade_set_args(L, "b", trh->IsValid());
}

//**********HANDLE: thrusterbank
ADE_OBJ(l_Thrusterbank, thrusterbank_h, "thrusterbank", "A model thrusterbank");

thrusterbank_h::thrusterbank_h() {
	bank = NULL;
}
thrusterbank_h::thrusterbank_h(thruster_bank* ba) {
	bank = ba;
}
thruster_bank* thrusterbank_h::Get() {
	if (!isValid())
		return NULL;

	return bank;
}
bool thrusterbank_h::isValid() {
	return bank != NULL;
}

ADE_FUNC(__len, l_Thrusterbank, NULL, "Number of thrusters on this thrusterbank", "number", "Number of thrusters on this bank or 0 if handle is invalid")
{
	thrusterbank_h *tbh = NULL;
	if(!ade_get_args(L, "o", l_Thrusterbank.GetPtr(&tbh)))
		return ade_set_error(L, "i", -1);

	if(!tbh->isValid())
		return ade_set_error(L, "i", -1);

	thruster_bank* bank = tbh->Get();

	return ade_set_args(L, "i", bank->num_points);
}

ADE_INDEXER(l_Thrusterbank, "number Index", "Array of glowpoint", "glowpoint", "Glowpoint, or invalid glowpoint handle on failure")
{
	thrusterbank_h *tbh = NULL;
	const char* s       = nullptr;
	glowpoint_h *glh = NULL;

	if (!ade_get_args(L, "os|o", l_Thrusterbank.GetPtr(&tbh), &s, l_Glowpoint.GetPtr(&glh)))
		return ade_set_error(L, "o", l_Glowpoint.Set(glowpoint_h()));

	if (!tbh->isValid() || s==NULL)
		return ade_set_error(L, "o", l_Glowpoint.Set(glowpoint_h()));

	thruster_bank* bank = tbh->Get();

	//Determine index
	int idx = atoi(s) - 1;	//Lua->FS2

	if (idx < 0 || idx >= bank->num_points)
		return ade_set_error(L, "o", l_Glowpoint.Set(glowpoint_h()));

	glow_point* glp = &bank->points[idx];

	if (ADE_SETTING_VAR && glh != NULL)
	{
		if (glh->isValid())
		{
			bank->points[idx] = *(glh->Get());
		}
	}

	return ade_set_args(L, "o", l_Glowpoint.Set(glp));
}

ADE_FUNC(isValid, l_Thrusterbank, NULL, "Detectes if this handle is valid", "boolean", "true if this handle is valid, false otherwise")
{
	thrusterbank_h* trh;
	if(!ade_get_args(L, "o", l_Thrusterbank.GetPtr(&trh)))
		return ADE_RETURN_FALSE;

	if (!trh->isValid())
		return ADE_RETURN_FALSE;

	return ade_set_args(L, "b", trh->isValid());
}

//**********HANDLE: glowpoint
ADE_OBJ(l_Glowpoint, glowpoint_h, "glowpoint", "A model glowpoint");

glowpoint_h::glowpoint_h() {
}
glowpoint_h::glowpoint_h(glow_point* np) {
	point = np;
}
glow_point* glowpoint_h::Get() {
	if (!isValid())
		return NULL;

	return point;
}
bool glowpoint_h::isValid() {
	return point != NULL;
}

ADE_VIRTVAR(Position, l_Glowpoint, NULL, "The (local) vector to the position of the glowpoint", "vector", "The local vector to the glowpoint or nil invalid")
{
	glowpoint_h *glh = NULL;
	vec3d newVec;

	if(!ade_get_args(L, "o|o", l_Glowpoint.GetPtr(&glh), l_Vector.Get(&newVec)))
		return ADE_RETURN_NIL;

	if (!glh->isValid())
		return ADE_RETURN_NIL;

	vec3d vec = glh->point->pnt;

	if (ADE_SETTING_VAR)
	{
		glh->point->pnt = newVec;
	}

	return ade_set_args(L, "o", l_Vector.Set(vec));
}

ADE_VIRTVAR(Radius, l_Glowpoint, NULL, "The radius of the glowpoint", "number", "The radius of the glowpoint or -1 of invalid")
{
	glowpoint_h* glh = NULL;
	float newVal;

	if(!ade_get_args(L, "o|f", l_Glowpoint.GetPtr(&glh), &newVal))
		return ade_set_error(L, "f", -1.0f);

	if (!glh->isValid())
		return ade_set_error(L, "f", -1.0f);

	float radius = glh->point->radius;

	if (ADE_SETTING_VAR)
	{
		glh->point->radius = newVal;
	}

	return ade_set_args(L, "f", radius);
}

ADE_FUNC(isValid, l_Glowpoint, NULL, "Returns wether this handle is valid or not", "boolean", "True if handle is valid, false otherwise")
{
	glowpoint_h glh = NULL;

	if(!ade_get_args(L, "o", l_Glowpoint.Get(&glh)))
		return ADE_RETURN_FALSE;

	return ade_set_args(L, "b", glh.isValid());
}

//**********HANDLE: dockingbays
ADE_OBJ(l_Dockingbays, dockingbays_h, "dockingbays", "The docking bays of a model");

dockingbays_h::dockingbays_h(polymodel* pm) : model_h(pm){}
dockingbays_h::dockingbays_h() : model_h(){}

ADE_INDEXER(l_Dockingbays, "dockingbay", "Gets a dockingbay handle from this model. If a string is given then a dockingbay with that name is searched.", "dockingbay", "Handle or invalid handle on error")
{
	dockingbays_h *dbhp = NULL;
	int index = -1;
	dockingbay_h *newVal = NULL;

	if (lua_isnumber(L, 2))
	{
		if (!ade_get_args(L, "oi|o", l_Dockingbays.GetPtr(&dbhp), &index, l_Dockingbay.GetPtr(&newVal)))
			return ade_set_error(L, "o", l_Dockingbay.Set(dockingbay_h()));

		if (!dbhp->IsValid())
			return ade_set_error(L, "o", l_Dockingbay.Set(dockingbay_h()));

		index--; // Lua --> C/C++
	}
	else
	{
		const char* name = nullptr;

		if (!ade_get_args(L, "os|o", l_Dockingbays.GetPtr(&dbhp), &name, l_Dockingbay.GetPtr(&newVal)))
		{
			return ade_set_error(L, "o", l_Dockingbay.Set(dockingbay_h()));
		}

		if (!dbhp->IsValid() && name != NULL)
			return ade_set_error(L, "o", l_Dockingbay.Set(dockingbay_h()));

		index = model_find_dock_name_index(dbhp->GetID(), name);
	}

	polymodel *pm = dbhp->Get();

	if (index < 0 || index >= pm->n_docks)
	{
		return ade_set_error(L, "o", l_Dockingbay.Set(dockingbay_h()));
	}

	return ade_set_args(L, "o", l_Dockingbay.Set(dockingbay_h(pm, index)));
}

ADE_FUNC(__len, l_Dockingbays, NULL, "Retrieves the number of dockingbays on this model", "number", "number of docking bays or 0 on error")
{
	dockingbays_h *dbhp = NULL;

	if (!ade_get_args(L, "o", l_Dockingbays.GetPtr(&dbhp)))
		return ade_set_error(L, "i", 0);

	if (!dbhp->IsValid())
		return ade_set_error(L, "i", 0);

	return ade_set_args(L, "i", dbhp->Get()->n_docks);
}

//**********HANDLE: dockingbay
ADE_OBJ(l_Dockingbay, dockingbay_h, "dockingbay", "Handle to a model docking bay");

dockingbay_h::dockingbay_h(polymodel* pm, int dock_idx) : model_h(pm), dock_id(dock_idx) {}
dockingbay_h::dockingbay_h() : model_h(), dock_id(-1){}
bool dockingbay_h::IsValid() {
	if (!model_h::IsValid())
	{
		return false;
	}
	else
	{
		return dock_id >= 0 && dock_id < this->Get()->n_docks;
	}
}
dock_bay* dockingbay_h::getDockingBay() {
	if (!this->IsValid())
	{
		return NULL;
	}

	return &this->Get()->docking_bays[dock_id];
}

ADE_FUNC(__len, l_Dockingbay, NULL, "Gets the number of docking points in this bay", "number", "The number of docking points or 0 on error")
{
	dockingbay_h* dbh = NULL;

	if (!ade_get_args(L, "o", l_Dockingbay.GetPtr(&dbh)))
	{
		return ade_set_error(L, "i", 0);
	}

	if (dbh == NULL || !dbh->IsValid())
	{
		return ade_set_error(L, "i", 0);
	}

	return ade_set_args(L, "i", dbh->getDockingBay()->num_slots);
}

ADE_FUNC(getName, l_Dockingbay, NULL, "Gets the name of this docking bay", "string", "The name or an empty string on error")
{
	dockingbay_h* dbh = NULL;
	if (!ade_get_args(L, "o", l_Dockingbay.GetPtr(&dbh)))
	{
		return ade_set_error(L, "s", "");
	}

	if (dbh == NULL || !dbh->IsValid())
	{
		return ade_set_error(L, "s", "");
	}

	dock_bay* dbp = dbh->getDockingBay();

	return ade_set_args(L, "s", dbp->name);
}

ADE_FUNC(getPoint, l_Dockingbay, "number index", "Gets the location of a docking point in this bay", "vector", "The local location or null vector on error")
{
	dockingbay_h* dbh = NULL;
	int index = -1;

	if (!ade_get_args(L, "oi", l_Dockingbay.GetPtr(&dbh), &index))
	{
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));
	}

	index--; // Lua --> C/C++

	if (dbh == NULL || !dbh->IsValid())
	{
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));
	}

	dock_bay* dbp = dbh->getDockingBay();

	if (index < 0 || index > dbp->num_slots)
	{
		LuaError(L, "Invalid dock bay index %d!", (index + 1));
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));
	}

	return ade_set_args(L, "o", l_Vector.Set(dbp->pnt[index]));
}

ADE_FUNC(getNormal, l_Dockingbay, "number index", "Gets the normal of a docking point in this bay", "vector", "The normal vector or null vector on error")
{
	dockingbay_h* dbh = NULL;
	int index = -1;

	if (!ade_get_args(L, "oi", l_Dockingbay.GetPtr(&dbh), &index))
	{
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));
	}

	index--; // Lua --> C/C++

	if (dbh == NULL || !dbh->IsValid())
	{
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));
	}

	dock_bay* dbp = dbh->getDockingBay();

	if (index < 0 || index > dbp->num_slots)
	{
		LuaError(L, "Invalid dock bay index %d!", (index + 1));
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));
	}

	return ade_set_args(L, "o", l_Vector.Set(dbp->norm[index]));
}

ADE_FUNC(computeDocker, l_Dockingbay, "dockingbay",
         "Computes the final position and orientation of a docker bay that docks with this bay.",
         "vector, orientation",
         "The local location and orientation of the docker vessel in the reference to the vessel of the docking bay "
         "handle, or a nil value on error")
{
	dockingbay_h *dockee_bay_h = NULL, *docker_bay_h = NULL;
	vec3d final_pos;
	matrix final_orient;

	if (!ade_get_args(L, "oo", l_Dockingbay.GetPtr(&dockee_bay_h), l_Dockingbay.GetPtr(&docker_bay_h)))
	{
		return ADE_RETURN_NIL;
	}

	if (!dockee_bay_h->IsValid() || !docker_bay_h->IsValid())
	{
		return ADE_RETURN_NIL;
	}

	dock_bay* dockee_bay = dockee_bay_h->getDockingBay();
	dock_bay* docker_bay = docker_bay_h->getDockingBay();

	// Mostly the same as aicode.cpp: dock_orient_and_approach
	vec3d dockee_dock_pos, docker_dock_pos_local, docker_dock_pos;
	vec3d dock_up_dir;

	// Get the center between the two docking points
	vm_vec_avg(&dockee_dock_pos, &dockee_bay->pnt[0], &dockee_bay->pnt[1]);
	vm_vec_avg(&docker_dock_pos_local, &docker_bay->pnt[0], &docker_bay->pnt[1]);

	// Get the up-vector of the docking bay
	vm_vec_sub(&dock_up_dir, &dockee_bay->pnt[1], &dockee_bay->pnt[0]);

	// Compute the orientation
	vm_vector_2_matrix(&final_orient, &dockee_bay->norm[0], &dock_up_dir, NULL);

	// Rotate the docker position into the right orientation
	vm_vec_unrotate(&docker_dock_pos, &docker_dock_pos_local, &final_orient);

	// The docker vector points into the wrong direction, we need to scale it appropriately
	vm_vec_scale(&docker_dock_pos, -1.0f);

	// Now get the position of the other vessel
	vm_vec_add(&final_pos, &dockee_dock_pos, &docker_dock_pos);

	return ade_set_args(L, "oo", l_Vector.Set(final_pos),l_Matrix.Set(matrix_h(&final_orient)));
}

ADE_FUNC(isValid, l_Dockingbay, NULL, "Detects whether is valid or not", "number", "<i>true</i> if valid, <i>false</i> otherwise")
{
	dockingbay_h* dbh = NULL;

	if (!ade_get_args(L, "o", l_Dockingbay.GetPtr(&dbh)))
	{
		return ade_set_error(L, "i", 0);
	}

	if (dbh == NULL)
	{
		return ADE_RETURN_FALSE;
	}

	return ade_set_args(L, "b", dbh->IsValid());
}

}
}
