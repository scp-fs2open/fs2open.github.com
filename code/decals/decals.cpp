//sorce file for decals-Bobboau
//and no, I don't have any idea what I'm doing

#include "decals/decals.h"
#include "ship/ship.h"
#include "render/3d.h"
#include "io/timer.h"
#include "math/fvi.h"
#include "lighting/lighting.h"

int decal_intersect_seg_with_cube(vector * poly_point, vector start, vector end, vector cube_point[8], matrix orient, int ints=1);

int decal_intersect_poly_with_cube(vector * poly_point, vector point[3], vector cube_point[8], int ints);

decal_model d_model;

int poly_num = 0;

static vector decal_cube_point[8];
static vector decal_cube_plane[6][2];
static vector decal_hit_point;
static float decal_hit_radius;

static matrix decal_orient;

static vector *	decal_point_list[MAX_DECAL_DEFPOINTS];

static int decal_poly[MAX_DECAL_DEFPOINTS];

static decal * new_decal;

static int nverts;

static float min_rad;

static int back_faceing;

/*
	vector uvec, rvec;
	vector temp;

	temp = *pos;

	vm_vec_sub( &rvec, Eyeposition, &temp );
	vm_vec_normalize( &rvec );	

	vm_vec_crossprod(&uvec,fvec,&rvec);
	vm_vec_normalize(&uvec);

	vm_vec_scale_add( top, &temp, &uvec, w/2.0f );
	vm_vec_scale_add( bot, &temp, &uvec, -w/2.0f );	
*/

int decal_find_next(object *obj){
	ship *shipp = &Ships[obj->instance];
	int oldest = shipp->decals[0].timestamp;
	for(int i = 0; i < MAX_SHIP_DECALS; i++){
		if(shipp->decals[i].is_valid != 1){			//use up all the unused decals first
			return i%MAX_SHIP_DECALS;
		}
	}
	for(i = 1; i < MAX_SHIP_DECALS; i++){
		if(shipp->decals[i].timestamp > oldest){	//then get the oldest one
			oldest = i;
		}
	}
	return oldest%MAX_SHIP_DECALS;
}

int decal_create_simple(object *obj, decal_point *point, int texture){//makes a simple non-clipped decal

	mprintf(("a decal is about to be made at %0.2f, %0.2f, %0.2f\n", point->pnt.xyz.x, point->pnt.xyz.y, point->pnt.xyz.z));
	mprintf(("orient fvec at %0.2f, %0.2f, %0.2f\n", point->orient.vec.fvec.xyz.x, point->orient.vec.fvec.xyz.y, point->orient.vec.fvec.xyz.z));

	if(obj->type != OBJ_SHIP){
		return 0;
	}
//	vertex vert[1][2];
//	vector vec[4];

	vector center = point->pnt;
	float rad = point->radius;
	if(rad <=0) rad = 10;
	mprintf(("radius %f\n",rad));
	vector plain_point[4];
//	mprintf(("orient uvec x %0.2f %0.2f %0.2f\n", point->orient.vec.uvec.xyz.x, point->orient.vec.uvec.xyz.y, point->orient.vec.uvec.xyz.z));
//	mprintf(("orient rvec x %0.2f %0.2f %0.2f\n", point->orient.vec.rvec.xyz.x, point->orient.vec.rvec.xyz.y, point->orient.vec.rvec.xyz.z));
//	mprintf(("orient fvec x %0.2f %0.2f %0.2f\n", point->orient.vec.fvec.xyz.x, point->orient.vec.fvec.xyz.y, point->orient.vec.fvec.xyz.z));
	plain_point[0] = point->orient.vec.uvec;
	plain_point[1] = plain_point[0];
		vm_vec_negate(&plain_point[1]);
	plain_point[2] = point->orient.vec.rvec;
	plain_point[3] = plain_point[2];
		vm_vec_negate(&plain_point[3]);

	mprintf(("\n"));

	int dinx = decal_find_next(obj);

	ship *shipp = &Ships[obj->instance];
	decal *dec = shipp->decals;

		vm_vec_scale_add( &dec[0].vert[0], &center, &plain_point[0], rad );
		vm_vec_scale_add( &dec[0].vert[1], &center, &plain_point[2], rad );
		vm_vec_scale_add( &dec[0].vert[2], &center, &plain_point[1], rad );
		vm_vec_scale_add( &dec[0].vert[3], &center, &plain_point[3], rad );

	dec[dinx].poly[0].point[0] = 0;
	dec[dinx].poly[0].point[1] = 1;
	dec[dinx].poly[0].point[2] = 2;

	dec[dinx].poly[1].point[0] = 0;
	dec[dinx].poly[1].point[1] = 3;
	dec[0].poly[1].point[2] = 2;

	dec[dinx].poly[0].uv[0].u = 0;	dec[0].poly[0].uv[0].v = 0;
	dec[dinx].poly[0].uv[1].u = 0;	dec[0].poly[0].uv[1].v = 1;
	dec[dinx].poly[0].uv[2].u = 1;	dec[0].poly[0].uv[2].v = 1;

	dec[dinx].poly[1].uv[0].u = 0;	dec[0].poly[1].uv[0].v = 0;
	dec[dinx].poly[1].uv[1].u = 1;	dec[0].poly[1].uv[1].v = 0;
	dec[dinx].poly[1].uv[2].u = 1;	dec[0].poly[1].uv[2].v = 1;

	dec[dinx].texture = texture;
	dec[dinx].n_poly = 2;
	dec[dinx].is_valid = 1;
	dec[dinx].timestamp = timestamp();

	mprintf(("a decal should have been made at %0.2f %0.2f %0.2f\n", point->pnt.xyz.x, point->pnt.xyz.y, point->pnt.xyz.z));
//Int3();
	return 1;
}

#define duw(p)	(*((uint *) (p)))
#define dw(p)	(*((int *) (p)))
#define dwp(p)	((int *) (p))
#define dvp(p)	((vector *) (p))
#define dfl(p)	(*((float *) (p)))


//creates a decal, returns decal index on succes, -1 on failure

int decal_create(object * obj, decal_point *point, int subobject, int texture, int backfaced_texture){
	mprintf(("a decal is about to be made at %0.2f, %0.2f, %0.2f\n", point->pnt.xyz.x, point->pnt.xyz.y, point->pnt.xyz.z));

	if(obj->type != OBJ_SHIP){
		return -1;
	}
	if(texture == -1)return -1;
	if(point->radius <= 0)return -1;

	vector center = point->pnt;
	float rad = point->radius;
	if(rad <=0) rad = 1;
	mprintf(("radius %f\n",rad));
	vector plain_point[4];
	vector cube_point[8];
//define the decals dimentions
	plain_point[0] = point->orient.vec.uvec;
	plain_point[1] = plain_point[0];
		vm_vec_negate(&plain_point[1]);
	plain_point[2] = point->orient.vec.rvec;
	plain_point[3] = plain_point[2];
		vm_vec_negate(&plain_point[3]);

	vector topcenter;
	vector bvec = point->orient.vec.fvec;
	vm_vec_negate(&bvec);
	vm_vec_scale_add(&topcenter, &center, &bvec, rad);

	for(int i = 0; i < 4; i++){
		vm_vec_scale_add( &cube_point[i], &topcenter, &plain_point[i], rad );
		vm_vec_scale_add( &cube_point[i+4], &cube_point[i], &point->orient.vec.fvec, rad*2);
	}

	float max_rad = vm_vec_dist_quick(&center, &cube_point[0]);	//the cube points are all equidistant, right?
	min_rad = fl_sqrt((rad * rad)/2);	//the cube points are all equidistant, right?
mprintf(("decal defined\n"));

//define the decals dimentions
	
//set up the decal model
	ship *shipp = &Ships[obj->instance];
	decal *sdec = shipp->decals;

	polymodel *pm = model_get(shipp->modelnum);
	bsp_info * sm;
	sm = &pm->submodel[subobject];
//	decal_model dmod = pm->submodel[subobject].dec_model;// sm->dec_model;
//	decal_model tdec;	//temporary decal model for holding the culled down part of the decal

//setting up the decal cube
	vector test_vec, temp1, temp2;
	for(i = 0; i<8; i++)decal_cube_point[i] = cube_point[i];

	decal_orient = point->orient;

	decal_cube_plane[0][0] = cube_point[5];

	temp1 = decal_orient.vec.uvec;
	temp2 = decal_orient.vec.rvec;

	vm_vec_avg(&test_vec, &temp1, &temp2);
	vm_vec_normalize(&test_vec);

	decal_cube_plane[0][1] = test_vec;

//	mprintf(("plane 0 position is x %0.2f y %0.2f z %0.2f\n", decal_cube_plane[0][0].xyz.x, decal_cube_plane[0][0].xyz.y, decal_cube_plane[0][0].xyz.z));
//	mprintf(("plane 0 normal is x %0.2f y %0.2f z %0.2f\n", decal_cube_plane[0][1].xyz.x, decal_cube_plane[0][1].xyz.y, decal_cube_plane[0][1].xyz.z));



	decal_cube_plane[1][0] = cube_point[0];

	vm_vec_negate(&test_vec);

	decal_cube_plane[1][1] = test_vec;
//	mprintf(("plane 1 position is x %0.2f y %0.2f z %0.2f\n", decal_cube_plane[1][0].xyz.x, decal_cube_plane[1][0].xyz.y, decal_cube_plane[1][0].xyz.z));
//	mprintf(("plane 1 normal is x %0.2f y %0.2f z %0.2f\n", decal_cube_plane[1][1].xyz.x, decal_cube_plane[1][1].xyz.y, decal_cube_plane[1][1].xyz.z));



	decal_cube_plane[2][0] = cube_point[5];

	vm_vec_negate(&temp2);
	vm_vec_avg(&test_vec, &temp1, &temp2);
	vm_vec_normalize(&test_vec);

	decal_cube_plane[2][1] = test_vec;

//	mprintf(("plane 2 position is x %0.2f y %0.2f z %0.2f\n", decal_cube_plane[2][0].xyz.x, decal_cube_plane[2][0].xyz.y, decal_cube_plane[2][0].xyz.z));
//	mprintf(("plane 2 normal is x %0.2f y %0.2f z %0.2f\n", decal_cube_plane[2][1].xyz.x, decal_cube_plane[2][1].xyz.y, decal_cube_plane[2][1].xyz.z));



	decal_cube_plane[3][0] = cube_point[0];

	vm_vec_negate(&test_vec);

	decal_cube_plane[3][1] = test_vec;

//	mprintf(("plane 3 position is x %0.2f y %0.2f z %0.2f\n", decal_cube_plane[3][0].xyz.x, decal_cube_plane[3][0].xyz.y, decal_cube_plane[3][0].xyz.z));
//	mprintf(("plane 3 normal is x %0.2f y %0.2f z %0.2f\n", decal_cube_plane[3][1].xyz.x, decal_cube_plane[3][1].xyz.y, decal_cube_plane[3][1].xyz.z));



	test_vec = decal_orient.vec.fvec;

	decal_cube_plane[4][0] = cube_point[0];

	decal_cube_plane[4][1] = test_vec;

//	mprintf(("plane 4 position is x %0.2f y %0.2f z %0.2f\n", decal_cube_plane[4][0].xyz.x, decal_cube_plane[4][0].xyz.y, decal_cube_plane[4][0].xyz.z));
//	mprintf(("plane 4 normal is x %0.2f y %0.2f z %0.2f\n", decal_cube_plane[4][1].xyz.x, decal_cube_plane[4][1].xyz.y, decal_cube_plane[4][1].xyz.z));



	vm_vec_negate(&test_vec);
	decal_cube_plane[5][0] = cube_point[5];

	decal_cube_plane[5][1] = test_vec;

//	mprintf(("plane 5 position is x %0.2f y %0.2f z %0.2f\n", decal_cube_plane[5][0].xyz.x, decal_cube_plane[5][0].xyz.y, decal_cube_plane[5][0].xyz.z));
//	mprintf(("plane 5 normal is x %0.2f y %0.2f z %0.2f\n", decal_cube_plane[5][1].xyz.x, decal_cube_plane[5][1].xyz.y, decal_cube_plane[5][1].xyz.z));
//setting up the decal cube

mprintf(("cube set up\n"));

	decal_hit_point = point->pnt;
	decal_hit_radius = max_rad;
	poly_num = 0;
	nverts = 0;

//go and find decals for each submodel
//	for(int z = 0; z<pm->n_models;z++){

	int dinx = decal_find_next(obj);
	mprintf(("decal is number %d\n", dinx));

	sdec[dinx].is_valid = 0;

		new_decal = &shipp->decals[dinx];

		if(backfaced_texture != -1){
			back_faceing = 1;
		}else{
			back_faceing = 0;
		}
		decal_create_sub((ubyte *)pm->submodel[subobject].bsp_data);

mprintf(("deacal defined\n"));

	sdec[dinx].texture = texture;
	sdec[dinx].backfaced_texture = backfaced_texture;
	sdec[dinx].is_valid = 1;
	sdec[dinx].timestamp = timestamp();
	sdec[dinx].n_poly = poly_num;
	sdec[dinx].state = 1;
	sdec[dinx].lifetime = 3000;
	sdec[dinx].frames = 1;
	sdec[dinx].importance = 0;


	mprintf(("a decal should have been made with %d polys\n",sdec[0].n_poly));


return dinx;
}
//decal_model d_model;

// Point list
// +0      int         id
// +4      int         size
// +8      int         n_verts
// +12     int         n_norms
// +16     int         offset from start of chunk to vertex data
// +20     n_verts*char    norm_counts
// +offset             vertex data. Each vertex n is a point followed by norm_counts[n] normals.     
void decal_create_defpoints(ubyte * p )
{
//	mprintf(("entering decal decal_create_defpoints\n"));


	int n;
	nverts = dw(p+8);	
	int offset = dw(p+16);	

	mprintf(("%d verts\n",nverts));
	ubyte * normcount = p+20;
	vector *src = dvp(p+offset);
	
	Assert( nverts < 1200 );



	for (n=0; n<nverts; n++ )	{


		decal_point_list[n] = src;
		new_decal->vert[n].xyz = src->xyz;
		src += normcount[n]+1;
//		mprintf(("defpoint %d, x %0.2f, y %0.2f, z %0.2f\n",n,d_model.vert[n].xyz.x,d_model.vert[n].xyz.y,d_model.vert[n].xyz.z));
	} 

}


// Sortnorms
// +0      int         id
// +4      int         size 
// +8      vector      normal
// +20     vector      center
// +32     float       radius
// 36     int     front offset
// 40     int     back offset
// 44     int     prelist offset
// 48     int     postlist offset
// 52     int     online offset

void decal_create_sortnorm(ubyte * p)
{
//	mprintf(("entering decal_create_sortnorm\n"));
	int frontlist = dw(p+36);
	int backlist = dw(p+40);
	int prelist = dw(p+44);
	int postlist = dw(p+48);
	int onlist = dw(p+52);
/*
	if ( Mc_pm->version >= 2000 )	{
		if (!mc_ray_boundingbox( vp(p+56), vp(p+68), &Mc_p0, &Mc_direction, NULL ))	{
			return;
		}
	}
*/
	if (prelist) decal_create_sub(p+prelist);
	if (backlist) decal_create_sub(p+backlist);
	if (onlist) decal_create_sub(p+onlist);
	if (frontlist) decal_create_sub(p+frontlist);
	if (postlist) decal_create_sub(p+postlist);
}

/*
typedef struct model_tmap_vert {
	short vertnum;
	short normnum;
	float u,v;
} model_tmap_vert;
*/
//int poly_num = 0;
// Textured Poly
// +0      int         id
// +4      int         size 
// +8      vector      normal
// +20     vector      normal_point
// +32     int         tmp = 0
// +36     int         nverts
// +40     int         tmap_num
// +44     nverts*(model_tmap_vert) vertlist (n,u,v)
void decal_create_tmappoly(ubyte * p)
{
//	mprintf(("entering decal_create_tmappoly\n"));

	if(poly_num >= MAX_DECAL_POLY){
		mprintf(("bugging out becase there are too many polys in the decal\n"));		
		return;
	}


	int i;
	int nv;
//	uv_pair uvlist[TMAP_MAX_VERTS];
//	vector points[TMAP_MAX_VERTS];
	model_tmap_vert *verts;

	nv = dw(p+36);
	if ( nv < 0 ) return;

//	int tmap_num = dw(p+40);
/*
	if ( (!(Mc->flags & MC_CHECK_INVISIBLE_FACES)) && (Mc_pm->textures[tmap_num] < 0) )	{
		// Don't check invisible polygons.
		return;
	}
*/
	verts = (model_tmap_vert *)(p+44);
	vector temppoly[820];

	for (i=0;i<nv;i++)	{
		decal_poly[i] = (int)verts[i].vertnum;
		temppoly[i].xyz = decal_point_list[decal_poly[i]]->xyz;
	}

	vector pnorm;
	vm_vec_perp(&pnorm, decal_point_list[decal_poly[0]], decal_point_list[decal_poly[1]], decal_point_list[decal_poly[2]]);

	float back = vm_vec_dot(&decal_cube_plane[4][1], &pnorm);

	if( (back_faceing == 0) && (back >= 0) )return;

//poly culling 
	vector pcenter;
	vm_vec_avg_n(&pcenter, nv, temppoly);
	float pradius = 0;
	float dist = 0;
	for (i=0;i<nv;i++)	{
		dist = vm_vec_dist(decal_point_list[decal_poly[i]], &pcenter);
		if(dist>pradius){
			pradius = dist;
		}
	}

	//if it's too far to posably get near the cube get out
	if(vm_vec_dist_quick(&pcenter, &decal_hit_point) >= (pradius + decal_hit_radius)){
//		mprintf(("leaveing becase poly is too far away\n"));
		return;
	}

//this is for faster testing, I'm not sure if it's realy makeing it faster or not
	for(int k = 0; k<6; k++){
		int first_good = -1;
		for(i = 0; ((i< nv) && (first_good == -1)); i++){
			if(fvi_point_dist_plane(&decal_cube_plane[k][0], &decal_cube_plane[k][1], decal_point_list[decal_poly[i]]) > 0){
				first_good = i;
			}
		}
		if(first_good == -1){
//			mprintf(("all points are behind a plane, polygon is not in the cube\n"));
			return;
		}
	}

//poly culling
	int temp_poly[820];

	int skip = 0, numv = 0, valid = 0;

//	mprintf(("starting to test the poly, with %d verts in it\n",nv));
	for(k = 0; k<6; k++){
	//find the first point in front of the plain
		int first_good = -1;
		for(i = 0; ((i< nv) && (first_good == -1)); i++){
//			mprintf(("testing point %d with plane %d\n", i, k));
//			mprintf(("point's position is x %0.2f y %0.2f z %0.2f\n", decal_point_list[decal_poly[i]]->xyz.x, decal_point_list[decal_poly[i]]->xyz.y, decal_point_list[decal_poly[i]]->xyz.z));
//			mprintf(("plane's position is x %0.2f y %0.2f z %0.2f\n", decal_cube_plane[k][0].xyz.x, decal_cube_plane[k][0].xyz.y, decal_cube_plane[k][0].xyz.z));
//			mprintf(("plane's normal is x %0.2f y %0.2f z %0.2f\n", decal_cube_plane[k][1].xyz.x, decal_cube_plane[k][1].xyz.y, decal_cube_plane[k][1].xyz.z));
			if(fvi_point_dist_plane(&decal_cube_plane[k][0], &decal_cube_plane[k][1], decal_point_list[decal_poly[i]]) > 0){
				first_good = i;
//				mprintf(("%d is the first good point\n", i));
			}
		}
		if(first_good == -1){
//			mprintf(("all points are behind a plane, polygon is not in the cube\n"));
			return;
		}else{
			valid = 1;
		}

		//first point in front of the plane is good of corse
	//	temp_poly[0] =  decal_poly[(first_good + nv)%nv];
		numv = 0;
	//go through each point and see if there is an intersection between it the next point and the current plane
	//if there is then make the intersection point the next point in the poly
	//ignor all points untill there is another intersection
	//other wise make the next point the next point in the poly
//		if(k==1)Int3();
		int is_infront = 1, is_infront2 = 1;
		for(i = first_good; i < (nv + first_good); i++){
//							mprintf(("looking for intersections between point %d and point %d, with plane %d\n", i%nv, (i + 1)%nv, k));
//							mprintf((" point %d, is at, x %0.2f y %0.2f z %0.2f and point %d, is at, x %0.2f y %0.2f z %0.2f\n", i%nv, decal_point_list[decal_poly[i]]->xyz.x, decal_point_list[decal_poly[i]]->xyz.y, decal_point_list[decal_poly[i]]->xyz.z,  (i + 1)%nv, decal_point_list[decal_poly[(i + 1)%nv]]->xyz.x, decal_point_list[decal_poly[(i + 1)%nv]]->xyz.y, decal_point_list[decal_poly[(i + 1)%nv]]->xyz.z));
			is_infront2 = is_infront;
			float is_in = fvi_point_dist_plane(&decal_cube_plane[k][0], &decal_cube_plane[k][1], decal_point_list[decal_poly[(i + 1)%nv]]);
			if(is_in > 0){
				is_infront = 1;
			}else{
				is_infront= 0;
			}
			if(is_infront != is_infront2){
				if(fvi_segment_plane(&new_decal->vert[nverts], &decal_cube_plane[k][0], &decal_cube_plane[k][1], decal_point_list[decal_poly[i%nv]], decal_point_list[decal_poly[(i + 1)%nv]], 0)){
//								mprintf(("segment %d %d has an an itersection\n", i%nv, (i + 1)%nv));
					decal_point_list[nverts] = &new_decal->vert[nverts];
//								mprintf(("new decal defpoint has been added as number %d at x %0.2f y %0.2f z %0.2f\n",nverts, new_decal->vert[nverts].xyz.x, new_decal->vert[nverts].xyz.y, new_decal->vert[nverts].xyz.z));
					temp_poly[numv] = nverts++;
//								mprintf(("temp_poly point %d, should have been set to defpoint %d, it was set to %d\n",numv, nverts-1, temp_poly[numv]));
					skip = (skip+1)%2;
//								mprintf(("skip has been set to %d\n",skip));
					numv++;
					if((skip == 0)){
//				mprintf(("adding point, becase it is after an intersection point\n", (i + 1)%nv));
						temp_poly[numv] = decal_poly[(i + 1)%nv];
						numv++;
					}
				}else{
//					mprintf(("there sould have been an intersection but there wasnt"));
				}

			}else{
				if((skip == 0)){//don't copy the first point
//				mprintf(("segment %d %d has no itersection, copying\n", i%nv, (i + 1)%nv));
					temp_poly[numv] = decal_poly[(i + 1)%nv];
						numv++;
				}else{
//					mprintf(("skipping segment %d %d becase it isn't in front of the plane\n", i%nv, (i + 1)%nv));
				}
			}
		}

//		mprintf(("temp poly has %d points\n", numv));
		for(int h = 0; h < numv; h++){
			decal_poly[h] = temp_poly[h];
			mprintf(("seting decal_poly %d to %d\n",h,temp_poly[h]));
		}
		nv = numv;
		numv=0;
//		mprintf(("decal_poly now has has %d points\n", nv));
		if(nv < 3)return;	//if at any time we get fewer than three points this poly is dead
	}


//	mprintf(("copying the poly to the decal\n"));
	//copy a triangulated version of the poly to the new decal
	if((valid == 1) && (nv > 2)){
		for (i=1;i<(nv-1);i++)	{
			if(poly_num >= MAX_DECAL_POLY){
				mprintf(("bugging out becase there are too many polys in the decal\n"));		
				return;
			}
			new_decal->poly[poly_num].point[0] = decal_poly[0];
			new_decal->poly[poly_num].point[1] = decal_poly[i];
			new_decal->poly[poly_num].point[2] = decal_poly[i+1];
			new_decal->poly[poly_num].norm = pnorm;
			if(back > 0){
				new_decal->poly[poly_num].backfaced = 0;
			}else{
				new_decal->poly[poly_num].backfaced = 1;
			}
			for (k=0;k<3;k++)	{
				//calculate the UV coords
				new_decal->poly[poly_num].uv[k].u = (fvi_point_dist_plane(&decal_cube_plane[0][0], &decal_cube_plane[0][1], &new_decal->vert[new_decal->poly[poly_num].point[k]]) / (min_rad*2));
				new_decal->poly[poly_num].uv[k].v = (fvi_point_dist_plane(&decal_cube_plane[2][0], &decal_cube_plane[2][1], &new_decal->vert[new_decal->poly[poly_num].point[k]]) / (min_rad*2));
			}
//			mprintf(("decal poly %d, vert 0 being set to %d, vert 1 %d, and vert 2 %d\n", poly_num, new_decal->poly[poly_num].point[0], new_decal->poly[poly_num].point[1], new_decal->poly[poly_num].point[2]));
			poly_num++;
		}
	}else{
//		mprintf(("no poly, all points are out side of planes"));
	}
	mprintf(("there are now %d polys in the current decal\n", poly_num));

	
}


int decal_create_sub(void *model_ptr )
{

//		mprintf(("plane 0 position is x %0.2f y %0.2f z %0.2f\n", decal_cube_plane[0][0].xyz.x, decal_cube_plane[0][0].xyz.y, decal_cube_plane[0][0].xyz.z));
//		mprintf(("plane 0 normal is x %0.2f y %0.2f z %0.2f\n", decal_cube_plane[0][1].xyz.x, decal_cube_plane[0][1].xyz.y, decal_cube_plane[0][1].xyz.z));

//	mprintf(("entering decal_create_sub\n"));

	ubyte *p = (ubyte *)model_ptr;
	int chunk_type, chunk_size;

	chunk_type = dw(p);
	chunk_size = dw(p+4);

//		mprintf(("plane 0 position is x %0.2f y %0.2f z %0.2f\n", decal_cube_plane[0][0].xyz.x, decal_cube_plane[0][0].xyz.y, decal_cube_plane[0][0].xyz.z));
//		mprintf(("plane 0 normal is x %0.2f y %0.2f z %0.2f\n", decal_cube_plane[0][1].xyz.x, decal_cube_plane[0][1].xyz.y, decal_cube_plane[0][1].xyz.z));


	while (chunk_type != DP_EOF)	{

//		mprintf(( "Processing chunk type %d, len=%d\n", chunk_type, chunk_size ));
//			mprintf(("plane 0 position is x %0.2f y %0.2f z %0.2f\n", decal_cube_plane[0][0].xyz.x, decal_cube_plane[0][0].xyz.y, decal_cube_plane[0][0].xyz.z));
//			mprintf(("plane 0 normal is x %0.2f y %0.2f z %0.2f\n", decal_cube_plane[0][1].xyz.x, decal_cube_plane[0][1].xyz.y, decal_cube_plane[0][1].xyz.z));


		switch (chunk_type) {
		case DP_EOF: return 1;
		case DP_DEFPOINTS:		decal_create_defpoints(p); break;
		case DP_FLATPOLY:		break;// I don't want any of these anyway-Bobboau decal_create_flatpoly(p); break;
		case DP_TMAPPOLY:		decal_create_tmappoly(p); break;
		case DP_SORTNORM:		decal_create_sortnorm(p); break;
		case DP_BOUNDBOX:	
/*			if (!mc_ray_boundingbox( vp(p+8), vp(p+20), &Mc_p0, &Mc_direction, NULL ))	{
				return 1;
			}*/
			break;
		default:
			mprintf(( "Bad chunk type %d, len=%d in decal_create_sub\n", chunk_type, chunk_size ));
			Int3();		// Bad chunk type!
			return 0;
		}
//				mprintf(( "current chunk offset %d\n", (int)p ));

		p += chunk_size;
		chunk_type = dw(p);
		chunk_size = dw(p+4);
	}


	return 1;
}


int decal_make_model(polymodel * pm){

	return 1;
}
/*
void decal_render_all(object * obj){

	if(obj->type != OBJ_SHIP){
		return;
	}

	vertex vecs[3];
	vertex *vlist[3] = { &vecs[0], &vecs[1], &vecs[2] };
	
//		mprintf(("about to render all decals\n"));

	ship *shipp = &Ships[obj->instance];

	for(int h = 0; h < MAX_SHIP_DECALS; h++){
		decal *dec = &shipp->decals[h];
		if(dec->is_valid){
			mprintf(("decal %d is valid, and has %d polys\n",h,dec->n_poly));
			for(int i = 0; i<dec->n_poly; i++){

mprintf(("drawing decal poly %d, with bitmap %d\n", i, dec->texture));
					gr_set_bitmap(dec->texture, 0, GR_BITBLT_MODE_NORMAL, 1.0f );
					if((dec->poly[i].backfaced == 1) && (dec->backfaced_texture)){
						gr_set_bitmap(dec->backfaced_texture, 0, GR_BITBLT_MODE_NORMAL, 1.0f );
					}

					for( int j = 0; j < 3; j++){
//						vecs[j] = dec->vert[i][j];
//						vector pos;
//						vm_vert2vec(&vecs[j], &pos);
					//	vm_vec_add2(&pos, &obj->pos);
						g3_rotate_vertex(&vecs[j], &dec->vert[dec->poly[i].point[j]]);
						g3_project_vertex(&vecs[j]);
						vecs[j].u = dec->poly[i].uv[j].u;
						vecs[j].v = dec->poly[i].uv[j].v;
mprintf(("vert %d at x %0.2f y %0.2f z %0.2f u %0.2f v %0.2f\n", j, vecs[j].x, vecs[j].y, vecs[j].z, vecs[j].u, vecs[j].v));

					}

					gr_set_cull(0);
					g3_draw_poly(3, vlist, TMAP_FLAG_TEXTURED | TMAP_FLAG_CORRECT );
					gr_set_cull(1);
			}
		}
	}
//	mprintf(("decals rendered\n"));

}
*/


void decal_render_all(object * obj){

	if(obj->type != OBJ_SHIP){
		return;
	}

	vertex vecs[3];
	vertex *vlist[3] = { &vecs[0], &vecs[1], &vecs[2] };
	
//		mprintf(("about to render all decals\n"));

	ship *shipp = &Ships[obj->instance];
	polymodel *pm = model_get( shipp->modelnum );

	for(int h = 0; h < MAX_SHIP_DECALS; h++){
		decal *dec = &shipp->decals[h];
		if(dec->is_valid){
			mprintf(("decal %d is valid, and has %d polys\n",h,dec->n_poly));
			for(int i = 0; i<dec->n_poly; i++){

			//	if((pm->submodel[dec->submodel_parent].blown_off != 0) && (dec->submodel_parent > 0))continue;

					for( int j = 0; j < 3; j++){
//						vecs[j] = dec->vert[i][j];
//						vector pos;
//						vm_vert2vec(&vecs[j], &pos);
					//	vm_vec_add2(&pos, &obj->pos);
						vector pnt = dec->vert[dec->poly[i].point[j]];
					if(dec->submodel_parent != pm->detail[0]){
							matrix m;
							angles angs = pm->submodel[dec->submodel_parent].angs;
							angs.b = PI2 - angs.b;
							angs.p = PI2 - angs.p;
							angs.h = PI2 - angs.h;
							mprintf(("decal is child of a submodel, who's angles are b %0.2f p %0.2f h %0.2f, it should be rotateing with it\n", angs.b, angs.p, angs.h));

							vector offset = pm->submodel[dec->submodel_parent].offset;
							vm_angles_2_matrix(&m, &angs);

							vector p = pnt;
							vm_vec_rotate(&pnt, &p, &m);

							vm_vec_add2(&pnt, &offset);
						}

						g3_rotate_vertex(&vecs[j], &pnt);
						g3_project_vertex(&vecs[j]);
						vecs[j].u = dec->poly[i].uv[j].u;
						vecs[j].v = dec->poly[i].uv[j].v;
mprintf(("vert %d at x %0.2f y %0.2f z %0.2f u %0.2f v %0.2f\n", j, vecs[j].x, vecs[j].y, vecs[j].z, vecs[j].u, vecs[j].v));
/*
						if ( D3D_enabled )	{
							light_apply_rgb( &vlist[j]->r, &vlist[j]->g, &vlist[j]->b, &pnt, &dec->poly[i].norm, 0 );
						} else {
							vlist[j]->b = light_apply( &pnt, &dec->poly[i].norm, 0 );
						}
*/
					}

					gr_set_bitmap(dec->texture, 0, GR_BITBLT_MODE_NORMAL, 1.0f );
					if((dec->poly[i].backfaced == 1) && (dec->backfaced_texture > -1)){
						gr_set_bitmap(dec->backfaced_texture, 0, GR_BITBLT_MODE_NORMAL, 1.0f );
						mprintf(("drawing decal poly %d, with bitmap %d\n", i, dec->backfaced_texture));
					}else{
						mprintf(("drawing decal poly %d, with bitmap %d\n", i, dec->texture));
					}

//					gr_set_cull(0);
					g3_draw_poly(3, vlist, TMAP_FLAG_TEXTURED | TMAP_FLAG_CORRECT | TMAP_FLAG_RGB | TMAP_FLAG_GOURAUD );
//					gr_set_cull(1);
			}
		}
	}
//	mprintf(("decals rendered\n"));

}
