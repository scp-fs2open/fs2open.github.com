/*
 * Created by Ian "Goober5000" Warfield for the FreeSpace2 Source Code Project.
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */ 

/*
 * $Logfile: /Freespace2/code/Model/ModelDiff.cpp $
 * $Revision: 2.2 $
 * $Date: 2007-02-26 04:47:52 $
 * $Author: Goober5000 $
 *
 * Stuff.
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.1  2007/02/26 04:30:48  Goober5000
 * add model compare capability (currently only via debug console)
 *
 */

#include <stdarg.h>
#include "model/model.h"
#include "ship/ship.h"


void (*print_string_global)(char *);

template <class T>
bool equals(T array1[], T array2[], int size1, int size2);

template <class T>
void diff(char *name, T array1[], T array2[], int size1, int size2);


// print one or two columns of stuff to our custom function -------------------

void xprintf(char *format, ...)
{
	Assert(print_string_global != NULL);


	// if we have two columns, divide up the text into left and right;
	// otherwise just display and exit

	int temp;
	char left[2048];
	char right[2048];
	va_list args;

	// get our complete string
	va_start(args, format);
	vsprintf(left, format, args);
	va_end(args);

	// check if we have one or two columns
	char *p = strstr(left, "||");
	if (p == NULL)
	{
		// one column: add a \n if none exists
		temp = strlen(left);
		if (left[temp-1] != '\n');
		{
			left[temp] = '\n';
			left[temp+1] = '\0';
		}
	}
	else
	{
		// two columns: chop off a \n if one exists, because we'll add it later
		temp = strlen(left);
		if (left[temp-1] == '\n')
			left[temp-1] = '\0';
	}


	// if one column, just print it and leave
	if (p == NULL)
	{
		print_string_global(left);
		return;
	}


	// divide the text
	strcpy(right, p+2);
	*p = '\0';
	

	// now we have the left and right columns...

	char buf[80];
	char *leftp = left;
	char *rightp = right;

	// display them in rows of 35 characters each
	while (leftp && rightp)
	{
		memset(buf, 0, 80*sizeof(char));

		// this part of the left column
		if (strlen(leftp) > 35)
		{
			strncat(buf, leftp, 35);
			leftp += 35;
		}
		else
		{
			strcat(buf, leftp);
			leftp = '\0';
		}

		// add spaces
		int idx = strlen(buf);
		while (idx < 40)
		{
			buf[idx] = ' ';
			idx++;
		}

		// this part of the right column
		if (strlen(rightp) > 35)
		{
			strncat(buf, rightp, 35);
			rightp += 35;
		}
		else
		{
			strcat(buf, rightp);
			rightp = '\0';
		}

		buf[strlen(buf)] = '\n';
		print_string_global(buf);
	}
}

// ----------------------------------------------------------------------------


// equals functions, needed for diffs below -----------------------------------

bool equals(char *text1, char *text2)
{
	if (!text1 && !text2)
		return true;
	else if (!text1 || !text2)
		return false;

	return !stricmp(text1, text2);
}

bool equals(int *num1, int *num2)
{
	if (!num1 && !num2)
		return true;
	else if (!num1 || !num2)
		return false;

	return *num1 == *num2;
}

bool equals(float *num1, float *num2)
{
	if (!num1 && !num2)
		return true;
	else if (!num1 || !num2)
		return false;

	return *num1 == *num2;
}

bool equals(vec3d *vec1, vec3d *vec2)
{
	if (!vec1 && !vec2)
		return true;
	else if (!vec1 || !vec2)
		return false;

	return (vm_vec_same(vec1, vec2) != 0) ? true : false;
}

bool equals(matrix *m1, matrix *m2)
{
	if (!m1 && !m2)
		return true;
	else if (!m1 || !m2)
		return false;

	return (vm_matrix_same(m1, m2) != 0) ? true : false;
}

bool equals(bsp_light *light1, bsp_light *light2)
{
	if (!light1 && !light2)
		return true;
	else if (!light1 || !light2)
		return false;

	return equals(&light1->pos, &light2->pos)
		&& light1->type == light2->type
		&& light1->value == light2->value;
}

bool equals(eye *eye1, eye *eye2)
{
	if (!eye1 && !eye2)
		return true;
	else if (!eye1 || !eye2)
		return false;

	return eye1->parent == eye2->parent
		&& equals(&eye1->pnt, &eye2->pnt)
		&& equals(&eye1->norm, &eye2->norm);
}

bool equals(w_bank *bank1, w_bank *bank2)
{
	if (!bank1 && !bank2)
		return true;
	else if (!bank1 || !bank2)
		return false;

	if (!equals(bank1->pnt, bank2->pnt, bank1->num_slots, bank2->num_slots))
		return false;
	if (!equals(bank1->norm, bank2->norm, bank1->num_slots, bank2->num_slots))
		return false;
	if (!equals(bank1->radius, bank2->radius, bank1->num_slots, bank2->num_slots))
		return false;

	return true;
}

bool equals(dock_bay *bay1, dock_bay *bay2)
{
	if (!bay1 && !bay2)
		return true;
	else if (!bay1 || !bay2)
		return false;

	if (!equals(bay1->name, bay2->name))
		return false;
	if (bay1->type_flags != bay2->type_flags)
		return false;

	if (!equals(bay1->splines, bay2->splines, bay1->num_spline_paths, bay2->num_spline_paths))
		return false;

	if (!equals(bay1->pnt, bay2->pnt, bay1->num_slots, bay2->num_slots))
		return false;
	if (!equals(bay1->norm, bay2->norm, bay1->num_slots, bay2->num_slots))
		return false;

	return true;
}

bool equals(engine_wash_info *info1, engine_wash_info *info2)
{
	if (!info1 && !info2)
		return true;
	else if (!info1 || !info2)
		return false;

	return equals(info1->name, info2->name)
		&& info1->angle == info2->angle
		&& info1->radius_mult == info2->radius_mult
		&& info1->length == info2->length
		&& info1->intensity == info2->intensity;
}

bool equals(glow_point *gp1, glow_point *gp2)
{
	if (!gp1 && !gp2)
		return true;
	else if (!gp1 || !gp2)
		return false;

	return equals(&gp1->pnt, &gp2->pnt)
		&& equals(&gp1->norm, &gp2->norm)
		&& gp1->radius == gp2->radius;
}

bool equals(thruster_bank *bank1, thruster_bank *bank2)
{
	if (!bank1 && !bank2)
		return true;
	else if (!bank1 || !bank2)
		return false;

	if (!equals(bank1->points, bank2->points, bank1->num_points, bank2->num_points))
		return false;
	if (!equals(bank1->wash_info_pointer, bank2->wash_info_pointer))
		return false;
	if (bank1->obj_num != bank2->obj_num)
		return false;

	return true;
}

bool equals(mp_vert *v1, mp_vert *v2)
{
	if (!v1 && !v2)
		return true;
	else if (!v1 || !v2)
		return false;

	if (!equals(&v1->pos, &v2->pos))
		return false;
	if (!equals(v1->turret_ids, v2->turret_ids, v1->nturrets, v2->nturrets))
		return false;
	if (v1->radius != v2->radius)
		return false;

	return true;
}

bool equals(model_path *path1, model_path *path2)
{
	if (!path1 && !path2)
		return true;
	else if (!path1 || !path2)
		return false;

	if (!equals(path1->name, path2->name))
		return false;
	if (!equals(path1->parent_name, path2->parent_name))
		return false;
	if (path1->parent_submodel != path2->parent_submodel)
		return false;
	if (!equals(path1->verts, path2->verts, path1->nverts, path2->nverts))
		return false;
	if (path1->goal != path2->goal)
		return false;
	if (path1->type != path2->type)
		return false;
	if (path1->value != path2->value)
		return false;

	return true;
}

bool equals(cross_section *cs1, cross_section *cs2)
{
	if (!cs1 && !cs2)
		return true;
	else if (!cs1 || !cs2)
		return false;

	return cs1->radius == cs2->radius
		&& cs1->z == cs2->z;
}

bool equals(glow_point_bank *bank1, glow_point_bank *bank2)
{
	if (!bank1 && !bank2)
		return true;
	else if (!bank1 || !bank2)
		return false;

	return bank1->type == bank2->type
		&& bank1->on_time == bank2->on_time
		&& bank1->off_time == bank2->off_time
		&& bank1->disp_time == bank2->disp_time
		&& bank1->submodel_parent == bank2->submodel_parent
		&& bank1->LOD == bank2->LOD
		&& equals(bank1->points, bank2->points, bank1->num_points, bank2->num_points);
}

// ----------------------------------------------------------------------------


// simple diff functions ------------------------------------------------------

void diff(char *name, char *text1, char *text2)
{
	if (equals(text1, text2))
		return;

	if (name)
		xprintf(name);

	if (text1 && text2)
		xprintf("%s||%s", text1, text2);
	else if (text1)
		xprintf("%s||", text1);
	else if (text2)
		xprintf("||%s", text2);
}

void diff(char *name, int *num1, int *num2)
{
	if (equals(num1, num2))
		return;

	if (name)
		xprintf(name);

	if (num1 && num2)
		xprintf("%d||%d", *num1, *num2);
	else if (num1)
		xprintf("%d||", *num1);
	else if (num2)
		xprintf("||%d", *num2);
}

void diff(char *name, float *num1, float *num2)
{
	if (equals(num1, num2))
		return;

	if (name)
		xprintf(name);

	if (num1 && num2)
		xprintf("%f||%f", *num1, *num2);
	else if (num1)
		xprintf("%f||", *num1);
	else if (num2)
		xprintf("||%f", *num2);
}

void diff(char *name, vec3d *vec1, vec3d *vec2)
{
	if (equals(vec1, vec2))
		return;

	if (name)
		xprintf(name);

	if (vec1 && vec2)
		xprintf("%f %f %f||%f %f %f", vec1->xyz.x, vec1->xyz.y, vec1->xyz.z, vec2->xyz.x, vec2->xyz.y, vec2->xyz.z); 
	else if (vec1)
		xprintf("%f %f %f||", vec1->xyz.x, vec1->xyz.y, vec1->xyz.z);
	else if (vec2)
		xprintf("||%f %f %f", vec2->xyz.x, vec2->xyz.y, vec2->xyz.z); 
}

// ----------------------------------------------------------------------------


// compound diff functions ----------------------------------------------------

void diff(char *name, matrix *m1, matrix *m2)
{
	if (equals(m1, m2))
		return;

	if (name)
		xprintf(name);

	if (m1 && m2)
	{
		diff(NULL, &m1->vec.rvec, &m2->vec.rvec);
		diff(NULL, &m1->vec.uvec, &m2->vec.uvec);
		diff(NULL, &m1->vec.fvec, &m2->vec.fvec);
	}
	else if (m1)
	{
		diff(NULL, &m1->vec.rvec, NULL);
		diff(NULL, &m1->vec.uvec, NULL);
		diff(NULL, &m1->vec.fvec, NULL);
	}
	else if (m2)
	{
		diff(NULL, NULL, &m2->vec.rvec);
		diff(NULL, NULL, &m2->vec.uvec);
		diff(NULL, NULL, &m2->vec.fvec);
	}
}

void diff(char *name, bsp_light *light1, bsp_light *light2)
{
	if (equals(light1, light2))
		return;

	if (name)
		xprintf(name);

	if (light1 && light2)
	{
		diff(NULL, &light1->pos, &light2->pos);
		diff(NULL, &light1->type, &light2->type);
		diff(NULL, &light1->value, &light2->value);
	}
	else if (light1)
	{
		diff(NULL, &light1->pos, NULL);
		diff(NULL, &light1->type, NULL);
		diff(NULL, &light1->value, NULL);
	}
	else if (light2)
	{
		diff(NULL, NULL, &light2->pos);
		diff(NULL, NULL, &light2->type);
		diff(NULL, NULL, &light2->value);
	}
}

void diff(char *name, eye *eye1, eye *eye2)
{
	if (equals(eye1, eye2))
		return;

	if (name)
		xprintf(name);

	if (eye1 && eye2)
	{
		diff(NULL, &eye1->parent, &eye2->parent);
		diff(NULL, &eye1->pnt, &eye2->pnt);
		diff(NULL, &eye1->norm, &eye2->norm);
	}
	else if (eye1)
	{
		diff(NULL, &eye1->parent, NULL);
		diff(NULL, &eye1->pnt, NULL);
		diff(NULL, &eye1->norm, NULL);
	}
	else if (eye2)
	{
		diff(NULL, NULL, &eye2->parent);
		diff(NULL, NULL, &eye2->pnt);
		diff(NULL, NULL, &eye2->norm);
	}
}

void diff(char *name, ship_bay *bay1, ship_bay *bay2)
{
	if (bay1 && bay2)
		diff(name, bay1->path_indexes, bay2->path_indexes, bay1->num_paths, bay2->num_paths);
	else if (bay1)
		diff(name, bay1->path_indexes, bay1->path_indexes, bay1->num_paths, 0);
	else if (bay2)
		diff(name, bay2->path_indexes, bay2->path_indexes, 0, bay2->num_paths);
}

void diff(char *name, w_bank *bank1, w_bank *bank2)
{
	if (equals(bank1, bank2))
		return;

	if (name)
		xprintf(name);

	if (bank1 && bank2)
	{
		diff(NULL, bank1->pnt, bank2->pnt, bank1->num_slots, bank2->num_slots);
		diff(NULL, bank1->norm, bank2->norm, bank1->num_slots, bank2->num_slots);
		diff(NULL, bank1->radius, bank2->radius, bank1->num_slots, bank2->num_slots);
	}
	else if (bank1)
	{
		diff(NULL, bank1->pnt, bank1->pnt, bank1->num_slots, 0);
		diff(NULL, bank1->norm, bank1->norm, bank1->num_slots, 0);
		diff(NULL, bank1->radius, bank1->radius, bank1->num_slots, 0);
	}
	else if (bank2)
	{
		diff(NULL, bank2->pnt, bank2->pnt, 0, bank2->num_slots);
		diff(NULL, bank2->norm, bank2->norm, 0, bank2->num_slots);
		diff(NULL, bank2->radius, bank2->radius, 0, bank2->num_slots);
	}
}

void diff(char *name, dock_bay *bay1, dock_bay *bay2)
{
	if (equals(bay1, bay2))
		return;

	if (name)
		xprintf(name);

	if (bay1 && bay2)
	{
		diff(NULL, bay1->name, bay2->name);
		diff(NULL, &bay1->type_flags, &bay2->type_flags);

		diff(NULL, bay1->splines, bay2->splines, bay1->num_spline_paths, bay2->num_spline_paths);

		diff(NULL, bay1->pnt, bay2->pnt, bay1->num_slots, bay2->num_slots);
		diff(NULL, bay1->norm, bay2->norm, bay1->num_slots, bay2->num_slots);
	}
	else if (bay1)
	{
		diff(NULL, bay1->name, NULL);
		diff(NULL, &bay1->type_flags, NULL);

		diff(NULL, bay1->splines, bay1->splines, bay1->num_spline_paths, 0);

		diff(NULL, bay1->pnt, bay1->pnt, bay1->num_slots, 0);
		diff(NULL, bay1->norm, bay1->norm, bay1->num_slots, 0);
	}
	else if (bay2)
	{
		diff(NULL, NULL, bay2->name);
		diff(NULL, NULL, &bay2->type_flags);

		diff(NULL, bay2->splines, bay2->splines, 0, bay2->num_spline_paths);

		diff(NULL, bay2->pnt, bay2->pnt, 0, bay2->num_slots);
		diff(NULL, bay2->norm, bay2->norm, 0, bay2->num_slots);
	}
}

void diff(char *name, engine_wash_info *info1, engine_wash_info *info2)
{
	if (equals(info1, info2))
		return;

	if (name)
		xprintf(name);

	if (info1 && info2)
	{
		diff(NULL, info1->name, info2->name);
		diff(NULL, &info1->angle, &info2->angle);
		diff(NULL, &info1->radius_mult, &info2->radius_mult);
		diff(NULL, &info1->length, &info2->length);
		diff(NULL, &info1->intensity, &info2->intensity);
	}
	else if (info1)
	{
		diff(NULL, info1->name, NULL);
		diff(NULL, &info1->angle, NULL);
		diff(NULL, &info1->radius_mult, NULL);
		diff(NULL, &info1->length, NULL);
		diff(NULL, &info1->intensity, NULL);
	}
	else if (info2)
	{
		diff(NULL, NULL, info2->name);
		diff(NULL, NULL, &info2->angle);
		diff(NULL, NULL, &info2->radius_mult);
		diff(NULL, NULL, &info2->length);
		diff(NULL, NULL, &info2->intensity);
	}
}

void diff(char *name, glow_point *gp1, glow_point *gp2)
{
	if (equals(gp1, gp2))
		return;

	if (name)
		xprintf(name);

	if (gp1 && gp2)
	{
		diff(NULL, &gp1->pnt, &gp2->pnt);
		diff(NULL, &gp1->norm, &gp2->norm);
		diff(NULL, &gp1->radius, &gp2->radius);
	}
	else if (gp1)
	{
		diff(NULL, &gp1->pnt, NULL);
		diff(NULL, &gp1->norm, NULL);
		diff(NULL, &gp1->radius, NULL);
	}
	else if (gp2)
	{
		diff(NULL, NULL, &gp2->pnt);
		diff(NULL, NULL, &gp2->norm);
		diff(NULL, NULL, &gp2->radius);
	}
}

void diff(char *name, thruster_bank *bank1, thruster_bank *bank2)
{
	if (equals(bank1, bank2))
		return;

	if (name)
		xprintf(name);

	if (bank1 && bank2)
	{
		diff(NULL, bank1->points, bank2->points, bank1->num_points, bank2->num_points);
		diff(NULL, bank1->wash_info_pointer, bank2->wash_info_pointer);
		diff(NULL, &bank1->obj_num, &bank2->obj_num);
	}
	else if (bank1)
	{
		diff(NULL, bank1->points, bank1->points, bank1->num_points, 0);
		diff(NULL, bank1->wash_info_pointer, NULL);
		diff(NULL, &bank1->obj_num, NULL);
	}
	else if (bank2)
	{
		diff(NULL, bank2->points, bank2->points, 0, bank2->num_points);
		diff(NULL, NULL, bank2->wash_info_pointer);
		diff(NULL, NULL, &bank2->obj_num);
	}
}

void diff(char *name, mp_vert *v1, mp_vert *v2)
{
	if (equals(v1, v2))
		return;

	if (name)
		xprintf(name);

	if (v1 && v2)
	{
		diff(NULL, &v1->pos, &v2->pos);
		diff(NULL, v1->turret_ids, v2->turret_ids, v1->nturrets, v2->nturrets);
		diff(NULL, &v1->radius, &v2->radius);
	}
	else if (v1)
	{
		diff(NULL, &v1->pos, NULL);
		diff(NULL, v1->turret_ids, v1->turret_ids, v1->nturrets, 0);
		diff(NULL, &v1->radius, NULL);
	}
	else if (v2)
	{
		diff(NULL, NULL, &v2->pos);
		diff(NULL, v2->turret_ids, v2->turret_ids, 0, v2->nturrets);
		diff(NULL, NULL, &v2->radius);
	}
}

void diff(char *name, model_path *path1, model_path *path2)
{
	if (equals(path1, path2))
		return;

	if (name)
		xprintf(name);

	if (path1 && path2)
	{
		diff(NULL, path1->name, path2->name);
		diff(NULL, path1->parent_name, path2->parent_name);
		diff(NULL, &path1->parent_submodel, &path2->parent_submodel);
		diff(NULL, path1->verts, path2->verts, path1->nverts, path2->nverts);
		diff(NULL, &path1->goal, &path2->goal);
		diff(NULL, &path1->type, &path2->type);
		diff(NULL, &path1->value, &path2->value);
	}
	else if (path1)
	{
		diff(NULL, path1->name, NULL);
		diff(NULL, path1->parent_name, NULL);
		diff(NULL, &path1->parent_submodel, NULL);
		diff(NULL, path1->verts, path1->verts, path1->nverts, 0);
		diff(NULL, &path1->goal, NULL);
		diff(NULL, &path1->type, NULL);
		diff(NULL, &path1->value, NULL);
	}
	else if (path2)
	{
		diff(NULL, NULL, path2->name);
		diff(NULL, NULL, path2->parent_name);
		diff(NULL, NULL, &path2->parent_submodel);
		diff(NULL, path2->verts, path2->verts, 0, path2->nverts);
		diff(NULL, NULL, &path2->goal);
		diff(NULL, NULL, &path2->type);
		diff(NULL, NULL, &path2->value);
	}
}

void diff(char *name, cross_section *cs1, cross_section *cs2)
{
	if (equals(cs1, cs2))
		return;

	if (name)
		xprintf(name);

	if (cs1 && cs2)
	{
		diff(NULL, &cs1->radius, &cs2->radius);
		diff(NULL, &cs1->z, &cs2->z);
	}
	else if (cs1)
	{
		diff(NULL, &cs1->radius, NULL);
		diff(NULL, &cs1->z, NULL);
	}
	else if (cs2)
	{
		diff(NULL, NULL, &cs2->radius);
		diff(NULL, NULL, &cs2->z);
	}
}

void diff(char *name, glow_point_bank *bank1, glow_point_bank *bank2)
{
	if (equals(bank1, bank2))
		return;

	if (name)
		xprintf(name);

	if (bank1 && bank2)
	{
		diff(NULL, &bank1->type, &bank2->type);
		diff(NULL, &bank1->on_time, &bank2->on_time);
		diff(NULL, &bank1->off_time, &bank2->off_time);
		diff(NULL, &bank1->disp_time, &bank2->disp_time);
		diff(NULL, &bank1->submodel_parent, &bank2->submodel_parent);
		diff(NULL, &bank1->LOD, &bank2->LOD);
		diff(NULL, bank1->points, bank2->points, bank1->num_points, bank2->num_points);
	}
	else if (bank1)
	{
		diff(NULL, &bank1->type, NULL);
		diff(NULL, &bank1->on_time, NULL);
		diff(NULL, &bank1->off_time, NULL);
		diff(NULL, &bank1->disp_time, NULL);
		diff(NULL, &bank1->submodel_parent, NULL);
		diff(NULL, &bank1->LOD, NULL);
		diff(NULL, bank1->points, bank1->points, bank1->num_points, 0);
	}
	else if (bank2)
	{
		diff(NULL, NULL, &bank2->type);
		diff(NULL, NULL, &bank2->on_time);
		diff(NULL, NULL, &bank2->off_time);
		diff(NULL, NULL, &bank2->disp_time);
		diff(NULL, NULL, &bank2->submodel_parent);
		diff(NULL, NULL, &bank2->LOD);
		diff(NULL, bank2->points, bank2->points, 0, bank2->num_points);
	}
}


// ----------------------------------------------------------------------------


// array equals, capable of operating on different types of arrays ------------

template <class T>
bool equals(T array1[], T array2[], int size1, int size2)
{
	int i;

	if (size1 != size2)
		return false;

	for (i = 0; i < size1; i++)
		if (!equals(&array1[i], &array2[i]))
			return false;

	return true;
}

// ----------------------------------------------------------------------------


// array diff, capable of operating on different types of arrays --------------

template <class T>
void diff(char *name, T array1[], T array2[], int size1, int size2)
{
	int i;
	int max = (size1 > size2) ? size1 : size2;

	if (equals(array1, array2, size1, size2))
		return;

	if (name)
		xprintf(name);

	xprintf("size %d||size %d", size1, size2);
	for (i = 0; i < max; i++)
	{
		if (i < size1 && i < size2)
		{
			if (!equals(&array1[i], &array2[i]))
			{
				xprintf("%d:", i);
				diff(NULL, &array1[i], &array2[i]);
			}
		}
		else if (i < size1)
		{
			xprintf("%d:", i);
			diff(NULL, &array1[i], NULL);
		}
		else if (i < size2)
		{
			xprintf("%d:", i);
			diff(NULL, NULL, &array2[i]);
		}
	}
}

// ----------------------------------------------------------------------------


// the main model diffing function --------------------------------------------

void model_diff(int modelnum1, int modelnum2, void (*print_string)(char *))
{
	print_string_global = print_string;

	if (modelnum1 < 0 || modelnum2 < 0)
	{
		Int3();
		return;
	}

	polymodel *pm1 = model_get(modelnum1);
	polymodel *pm2 = model_get(modelnum2);


	diff("File", pm1->filename, pm2->filename);
	diff("Version", &pm1->version, &pm2->version);
	diff("Flags", (int *) &pm1->flags, (int *) &pm2->flags);

	// don't compare ID

	diff("Detail", pm1->detail, pm2->detail, pm1->n_detail_levels, pm2->n_detail_levels);
	diff("Detail Depth", pm1->detail_depth, pm2->detail_depth, pm1->n_detail_levels, pm2->n_detail_levels);

	diff("Debris Objects", pm1->debris_objects, pm2->debris_objects, pm1->num_debris_objects, pm2->num_debris_objects);

	diff("Mins", &pm1->mins, &pm2->mins);
	diff("Maxs", &pm1->maxs, &pm2->maxs);
	diff("Bounding Box", pm1->bounding_box, pm2->bounding_box, 8, 8);

	diff("Lights", pm1->lights, pm2->lights, pm1->num_lights, pm2->num_lights);

	diff("View Positions", pm1->view_positions, pm2->view_positions, pm1->n_view_positions, pm2->n_view_positions);

	diff("Autocenter", &pm1->autocenter, &pm2->autocenter);

	diff("Radius", &pm1->rad, &pm2->rad);
	diff("Core Radius", &pm1->core_radius, &pm2->core_radius);
	
	// don't compare texture maps

	// don't compare submodel info

	diff("Gun Banks", pm1->gun_banks, pm2->gun_banks, pm1->n_guns, pm2->n_guns);
	diff("Missile Banks", pm1->missile_banks, pm2->missile_banks, pm1->n_missiles, pm2->n_missiles);
	diff("Docking Bays", pm1->docking_bays, pm2->docking_bays, pm1->n_docks, pm2->n_docks);
	diff("Thrusters", pm1->thrusters, pm2->thrusters, pm1->n_thrusters, pm2->n_thrusters);

	diff("Fighter Bays", pm1->ship_bay, pm2->ship_bay);

	// don't compare shield info

	diff("Paths", pm1->paths, pm2->paths, pm1->n_paths, pm2->n_paths);

	diff("Mass", &pm1->mass, &pm2->mass);
	diff("Center of Mass", &pm1->center_of_mass, &pm2->center_of_mass);
	diff("Moment of Inertia", &pm1->moment_of_inertia, &pm2->moment_of_inertia);

	// don't compare model octants

	diff("Cross Sections", pm1->xc, pm2->xc, pm1->num_xc, pm2->num_xc);

	diff("Split Panes", pm1->split_plane, pm2->split_plane, pm1->num_split_plane, pm2->num_split_plane);

	// don't compare insigniae

	// don't compare used-this-mission

	diff("Glow Point Banks", pm1->glow_point_banks, pm2->glow_point_banks, pm1->n_glow_point_banks, pm2->n_glow_point_banks);

	diff("Gun Submodel Rotation", &pm1->gun_submodel_rotation, &pm2->gun_submodel_rotation);
}

