//sorce file for decals-Bobboau
//and no, I don't have any idea what I'm doing

/*
DECAL AND POLYGON MANAGEMENT:

High Level Decal Management
the decals are organised by texture into lists, they are held in the ship structure by a
dynamic array of decal_list_controle items wich, as the name sudgests, controle a decal list.
the decal_list_controle has the first element of a linked list of decals and the last element 
for easy adding to the end. it also keeps trak of the texture(s) to be used wich vertex buffer
is assosiated with the decals under it's controle, and manages decal book keeping (adding/trimming).
each ship has a set number of decals can be within each list, and once that number is 
exceeded it begins recycleing the oldest decal in the list. allocation of a new decal is 
provided by the 'decal_find_next' function, you tell it wich texture and wich ship the new 
decal is going to use and it find/allocate/does whatever it has to to get you a decal.

Low Level Decal Management
origonaly I made use of dynamic memory, this was a bad idea, slow as all hell. so after 
some thinking, I devised a static memory allocation system for both decals and the polys
within them. for each of them there is a staticly allocated arry 'big_ol_decal_array' and
'big_ol_decal_poly_array' respectively. the function 'get_open_decal_list' has the job of 
looking through the big_ol_decal_array and finding a decal that has not yet been claimed 
(or has been freed from it's former tyrantical master (a decal) ), it then returns a pointer 
to this elementwich is added onto the end of a decal list. there are MAX_GLOBAL_DECALS decals 
available, at the time of this documentation MAX_GLOBAL_DECALS is defined as 5000, I don't 
think it would ever use that many in it's current form though as the most decals any capship 
would have would probly be 700, and there's rarely more than two or three at a time, fighters
would only rack up about 10-20 before dieing, and the medium sized ship would fall some were 
between them (100-300). so the lookup of decals from the static memory block is a simple for 
loop that checks a 'used' flag, and the first one it finds it returns. 
there is also a secondary list for burn maps, this is a second pass that renders a glowing burn texture over fresh decals
in each decal there is a list of polygons (triangles) grabed from a similar global array, 
they are held in a linked list within each decal (you may be confused by the use of linked 
lists within an array the array simply serves as a giant block of allocated memory, outside 
of the management functions it is never used directly)
polygon mamagement is on the other hand signifigant enough to warent it's own section.

Low Level Polygon Management
initaly I thought of useing a similar simple lookup system as thedecal management, but the 
lookup of polygons is both more frequent (about a dozen times per created decal) and more 
intinsive (there are MAX_GLOBAL_DECAL_POLYS to sort through, at the time of documentation 
this was defined as _11000_ !). I had to devise something a bit more clever. the big_ol_decal_poly_array
is subdivided into (QUICKFIND_SUBDIVISION == 100) segments, each segment has a length 
determined to be (MAX_GLOBAL_DECAL_POLYS/QUICKFIND_SUBDIVISION) aka QUICKFIND_SEGMENT_LENGTH.
wich is 110 for anyone who may care I made a paralel array for each of these segments called 
'quick_find_array' looking in the elements of this array I can determine weather I should 
bother looking at any of the elements of big_ol_decal_poly_array within the segment that 
quick_find_array represents, this turns what would be a potentaly 11000 element search 
into no more than 210 (QUICKFIND_SUBDIVISION + QUICKFIND_SEGMENT_LENGTH), and even that 
would be rare (if all but one 11000 polys were taken). when a new polygon is needed 
get_open_poly is called, it will search through the quick look array untill it finds 
a segment that has an empty member in it (if any element in the segment represented 
by the quick look array is available then that element of the quicklook array will 
be false) then it will look through the found segment, and only that segment, untill it 
finds the empty poly that _is_ there. a great deal of time was spent ensureing that the 
quicklook array was in sync with the big_ol_decal_poly_array. once a poly is found it is 
marked as used and, if no more polys in the segment are free, that segment is marked (with 
true) as being totaly occupied

Debugging Functions
there are sevral debugging functions, they check the consistency of the lists, they only work 
in debug and they are _REALY >>_SLOW_<<_ wich is why they are all disabled, I don't 
even know if they still all/any work. enable at your own risk.



DECAL CREATION:
High Level Decal Creation Interface
for anyone wanting to add a decal to a ship, it is very simple, you simply fill out a 
decal_point structure and pass it along with what texture(s) you want the decal to use 
and the object (as ship) that you want it to be applyed to to the aptly named decal_create 
(currently subobjects are not suported so just ignore that paraeter for now).keep in mind 
that decals are organised by there primary texture, make sure you always send the same 
bunch of textures along with the primary texture (this is an issue I need to work on).

High Level Decal Creation
the mentioned decal_create takes the decal_point you give it and constructs a decal_cube, 
(see setup_decal_cube for the monotanus code) a rectangular volume defined by 6 planes 
(defined by a point and a normal each). if a backfaced texture is requested a second 
decal will be made, the model of the ship is then aquired and the innards of the BSP 
parseing take care of generateing polygons.

Low Level Decal Creation (the BSP parseing)
Parseing of the BSP is done through a serese of recersive functions, the first in the line 
is decal_create_sub, it is passed a pointer to the BSP data and sends it off to the proper 
sub functions:
decal_create_defpoints - 
this function parses the defpoints portion of the BSP data, it allocates and set's up 
the decal_point_list and decal__poly via the decal_realc_pointlist function. it then stuffs
decal_point_list with pointers to all of the vec3d defpoints within the defpoint chunck, at 
the time of documantation it ignores normals.
decal_create_sortnorm -
this function takes care of the sortnorm chunck, it actualy ignores the sort norms them selves, 
but it uses the bounding boxes to speed up generation of the decal, as well as improve the 
quality of the final decal. it finds wich sections of the BSP data should be ignored and 
wich ones should be procesed further, those wich should be procesed further are sent back 
through decal_create_sub wich then recersivelty searches through the rest of the sortnorms 
untill it finds a tmappoly chunk.
decal_create_tmappoly - 
wich brings us the the heart of the decal creation code, decal_create_tmappoly. this function 
works on the actual polygons of the model. there have bee sevral culling methods used to speed 
things up, but they are fairly irrelevent. what this function realy does is it calculates the 
distance of each of the points on the polygon from two of the decal planes, it then takes that
result and generates UV coords, if all the points are outside of a 0.0 to 1.0 range the poly is 
discarded, additional testing is done on a third plane to see if the polygon is too far along
the decal's z axis to be used.


DECAL RENDERING AND VERTEX BUFFER MANAGEMENT
Vertex Buffers
as mentioned in the decal management section, decals are organised by texture, this is becase
they are all rendere at the same time via a vertex buffer, and each decal list has a texture
and vertex buffer assosiated with it. any time a new decal is added to a list it's vertex 
buffer needs to be rebuilt, this is done in a manner similar to that used by the model rendering 
system, a poly list is used and allocated with the proper sized buffers for as many polys as 
it will be sending to the API level VB code, then it is filled with the vertex data of all 
decals. then it's sent off to be made into a vertex buffer and the returned index is stored 
by the decal list controle.

Rendering
after the vertex buffer has been made, the decal lists are gone through one at a time, the 
texture for the list is set, then a call is made to render the vertex buffer.

  recent additions hav also taken advantage of the ability to rnder part of a VB. 
  enableing dynamic rnedering of decals, this is currently being worked on, and
  is not covered in this documentation.

*/

#include "object/object.h"
#include "globalincs/linklist.h"
#include "decals/decals.h"
#include "render/3d.h"
#include "io/timer.h"
#include "math/fvi.h"
#include "lighting/lighting.h"
#include "debugconsole/timerbar.h"
#include "ship/ship.h"



int decal_total_decals_made = 0;
int total_polys_used = 0;
int highest_poly_index = 0;
int highest_decal_index = 0;

bool quick_find_array[QUICKFIND_SUBDIVISION] = {false};
//this subdivides the lower array into smaller chunks so that an empy slot can be found quicker
//if any of these are false then that means that there is atleast one empty slot within the range it represents
decal_poly big_ol_decal_poly_array[MAX_GLOBAL_DECAL_POLYS];

decal_item big_ol_decal_array[MAX_GLOBAL_DECALS];

/****BSP Defpoint Buffer Start****/
vec3d **decal_point_list = NULL;
int *decal__poly = NULL;
//those two are used deep within the bowels of the BSP parseing code

int decal_points_alocated = 0;

//model data is held in dynamic memory, it's only realocated when 

//the current buffers are too small, this free's the buffers atexit (see end of init_decals)
void free_decal_stuff(){
	if(decal_points_alocated){
		//if either one of these arn't NULL than someone allocated something to them
		if(decal_point_list)free(decal_point_list);
		if(decal__poly)free(decal__poly);
	}
}

//will realocate the lists (_without_ saveing what was in them!) 
//if the passed size is bigger than what is currently allocated
void decal_realc_pointlist(int n){
	if(decal_points_alocated < n){
		//if they've been allocated already free what they have
		if(decal_point_list)free(decal_point_list);
		if(decal__poly)free(decal__poly);
		//then reallocate with the new point count
		decal_point_list = (vec3d**) malloc(sizeof(vec3d *)*n); 
		decal__poly = (int*) malloc(sizeof(int)*n); 
		decal_points_alocated = n;
	}
}
/****BSP Defpoint Buffer End****/

/****init stuff****/
bool decals_once_init = false;
void init_decals(){
	int i;

	//quick find is set to be false everyware because there are no decals, and thus no polys
	for(i = 0; i<QUICKFIND_SUBDIVISION; i++)
		quick_find_array[i] = false;

	//every poly has a pointer to it's parent (used for error checking)
	//and the next poly in the list (and for debugging perposes it's index)
	//as these decals are all free at the begining these pointers are all NULL
	for( i = 0; i<MAX_GLOBAL_DECAL_POLYS; i++){
		big_ol_decal_poly_array[i].parent = NULL;
		big_ol_decal_poly_array[i].next = NULL;
//#ifndef NDEBUG
		big_ol_decal_poly_array[i].idx = i;
//#endif
	}

	//no decals are used, they don't have any polys, 
	//there are none next in the list of decals (becase they are free and not in a list)
	//and they have no poly list
	for( i = 0; i<MAX_GLOBAL_DECALS; i++){
		big_ol_decal_array[i].used = false;
		big_ol_decal_array[i].dec.dec.n_poly = 0;
		big_ol_decal_array[i].dec.next = NULL;
//		big_ol_decal_array[i].dec.next_burn = NULL;
		big_ol_decal_array[i].dec.dec.poly = NULL;
	}

	if(!decals_once_init){
		//see I told you it was hear
		atexit(free_decal_stuff);
		decals_once_init = true;
	}
}

/****begin ugly debugging functions****/
//I'm not documanting this becase there all disabled, and overly redundent and stupid
//alright, some debugging functions

//#define DECAL_DEBUGGING
//looks at all the decals in all the ships and makes sure there ok
void check_ship_decals(){
#ifndef DECAL_DEBUGGING	
	return;
#else
	for(int i = 0; i<MAX_SHIPS; i++){
		ship *shipp = &Ships[i];

		for(int j = 0; j<shipp->ship_decal_system.n_decal_textures; j++){
			decal_list_controle* list= shipp->ship_decal_system.decals;

			if(!list->n_decals)Assert(!list->d_list); //if there are no decals there should be no list 
			if(!list->d_list)Assert(!list->n_decals); //if there are no list there should be no decals
			if(list->n_decals)Assert(list->d_list); //if there are decals there should be a list 
			if(list->d_list)Assert(list->n_decals); //if there are list there should be some decals

			decal_list *decl = list->d_list;
			for(int d = 0; d<list->n_decals; d++){
				decal *dec = &decl->dec;
				dec->n_poly;

				decal_poly* p = dec->poly;
				if(!dec->n_poly)Assert(dec->poly == NULL); //if there are no polys there should be no polys
				int n = 0;
				while(p != NULL){
					n++;
					Assert(n <= dec->n_poly);
					p = p->next;
				}
				Assert(n == dec->n_poly);

				decl = decl->next;
			}
		}
	}
#endif // DECAL_DEBUGGING
}

// make sure this poly is withing the list, and while you'r at it, make sure there are the corect number of polys
bool check_decal(decal* dec, decal_poly* poly){
#ifndef DECAL_DEBUGGING	
	return true;
#else
#ifndef NDEBUG
	if(dec == NULL)return (poly->next==NULL)?true:false;
	decal_poly* p = dec->poly;
	if(p == NULL)return (dec == NULL)?true:false;
	int n = 0;
	while(p != NULL){
		n++;
		Assert(n <= dec->n_poly);
		if(p == poly)return true;
		p = p->next;
	}
	Assert(n == dec->n_poly);
	return false;
#endif
	return true;
#endif // DECAL_DEBUGGING
}

bool decal_is_in_proper_list(decal* dec){
#ifndef NDEBUG
	if(dec == NULL){
		Error(LOCATION, "decal is null!");
		return false;
	}
	decal_list_controle *list = dec->parent;
	if(list == NULL){
		if(dec->n_poly || dec->poly)Error(LOCATION, "decal list is null");
		return false;
	}
	decal_list *decl = list->d_list;
	if(decl == NULL){
		Error(LOCATION, "list controle has no list");
		return false;
	}
	while(decl != NULL){
		if(dec == &decl->dec)return true;
		decl = decl->next;
	}
	return false;
#endif
	return true;
}

//looks at all decals and makes sure there all ok
void check_list_decals(){
#ifndef DECAL_DEBUGGING	
	return;
#else
#ifndef NDEBUG
	for(int i = 0; i<MAX_GLOBAL_DECALS; i++){
		decal_item* d = &big_ol_decal_array[i];
		decal_list* dec = &d->dec;
		decal_is_in_proper_list(&dec->dec);
		if(d->used){
			decal_poly* p = dec->dec.poly;
		//	if(!dec->dec.n_poly)Assert(dec->dec.poly == NULL); //if there are no polys there should be no polys
			int n = 0;
			while(p != NULL){
				n++;

				Assert(n <= dec->dec.n_poly);
				p = p->next;
			}
			Assert(n == dec->dec.n_poly);
			Assert(p == NULL);
		}
	}
#endif
#endif // DECAL_DEBUGGING
}

//looks at all pollys, and the quicklook table, makes sure everything is cosuer
void check_poly_decals(){
#ifndef DECAL_DEBUGGING
	return;
#else
#ifndef NDEBUG
	int i;

	for(int q = 0; q<QUICKFIND_SUBDIVISION; q++){
		//look in each quicklook segment
		for(i = q*QUICKFIND_SEGMENT_LENGTH; i<(q+1)*QUICKFIND_SEGMENT_LENGTH; i++){
			//in each poly within the segment make sure everything is as it should be
			if(!quick_find_array[q]){
				//make sure there is at least one free
				if(big_ol_decal_poly_array[i].parent == NULL)break;
			}else{
				//make sure there all full in here
				Assert(big_ol_decal_poly_array[i].parent != NULL);
				//this quicklook segment should be totaly occupied
			}
		}
		if(!quick_find_array[q])Assert(i != (q+1)*QUICKFIND_SEGMENT_LENGTH);
		//the quicklook should have an empy but it doesn't!!!
	}

	for(i = 0; i<MAX_GLOBAL_DECAL_POLYS; i++){
		Assert(check_decal(big_ol_decal_poly_array[i].parent, &big_ol_decal_poly_array[i]));
	}
#endif
#endif // DECAL_DEBUGGING
}
/****end ugly debugging functions****/
//I feel dirty...


/*****************************/
/*BEGING DECAL POLY MANAGEMENT*/
/*****************************/

//finds an available polygon (triangle)
decal_poly* get_open_poly(){

	int i, j;

	check_list_decals();
		SAFEPOINT("entering get_open_poly");
		check_poly_decals();
		check_ship_decals();

	//look through the quicklook array
	for(i = 0; i<QUICKFIND_SUBDIVISION; i++)
		if(!quick_find_array[i]) break;
		//untill you find a segment that had an empty slot (false)

	int s = i;	//s = segment, it's were you are in the quicklook
	int end = ((i+1)*QUICKFIND_SEGMENT_LENGTH);	//start of the next segment
	//were we are going to stop looking in the big_ol_decal_poly_array

	//i*QUICKFIND_SEGMENT_LENGTH == the start of segment i
	//so go from the start of the segment we know is empty to the end of it
	for(i = i*QUICKFIND_SEGMENT_LENGTH; i<end; i++){
			check_poly_decals();

		//within this segment look at each member and see if it is assigned to anyone
		if(big_ol_decal_poly_array[i].parent == NULL){
			//if it isn't then that's the one we want
			if(i == end)quick_find_array[s]=true;
			//if this is the last element then this segment is now full
			Assert(big_ol_decal_poly_array[i].next == NULL);
			big_ol_decal_poly_array[i].next = NULL;
			//just to make sure

			for(j = i+1; j<end; j++){
				//see if there are any empties left in this segment 
				//if there aren't mark this segment as full
				if(big_ol_decal_poly_array[j].parent == NULL)break;
			}
			if(j == end)quick_find_array[s]=true;
			//if it couldn't find any empties, then this segment is full
			else quick_find_array[s]=false;
			//otherwise it's still got some space left in it
			
				SAFEPOINT("leaveing get_open_poly");
			//and were done :D
				if(i > highest_poly_index){
					highest_poly_index = i;
					mprintf(("highest poly used: %d; highest decal used: %d", highest_poly_index, highest_decal_index));
				}
			return &big_ol_decal_poly_array[i];
		}
	}
	//bad bad bad bad bad... 
	//couldn't find an empty within the segment that it was told was empty
	//bad BAD _BAD_ !!!!!11!!!1!
	//_realy freaking bad_

		check_ship_decals();
		check_list_decals();
		check_poly_decals();
	Warning(LOCATION, "get_open_poly failed\n");
	//it's so bad were gona just stop everything and tell you about it
		SAFEPOINT("leaveing get_open_poly");
	return NULL;	
}

//mark this poly as free also tell the quicklook array that there is something open here
void free_poly(decal_poly* poly, bool error_check = true){
	if(error_check){
	check_list_decals();
	check_poly_decals();
	}
		SAFEPOINT("entering free_poly");
	Assert(poly != NULL || poly->parent != NULL);

	int idx = int(poly - big_ol_decal_poly_array);//tricky :D, 
	//get's the index by pointer subtraction, was suprized this worked

	int i = idx/QUICKFIND_SEGMENT_LENGTH;
	//get the quicklook index
	quick_find_array[i] = false;
	//set it to false
	poly->parent = NULL;
	poly->next = NULL;
	//clear out andything that it has in relation to it's old master
#ifndef NDEBUG
	//this is some stuff to make sure that I got the right index
	Assert(i*QUICKFIND_SEGMENT_LENGTH <= poly->idx);
	Assert((i+1)*QUICKFIND_SEGMENT_LENGTH > poly->idx);
	Assert(idx == poly->idx);
#endif
		SAFEPOINT("leaveing free_poly");
	//done
	if(error_check){
	check_list_decals();
	check_poly_decals();
	}
}

decal_poly* decal::add_poly(){
		SAFEPOINT("entering decal::add_poly");
	check_list_decals();
	check_poly_decals();

	//get the last poly on the poly list for this decal
	decal_poly* ppoly = get_last_poly();

	//if you couldn't get any, that means there were none to get
	//this is the first poly in the list
	if(ppoly == NULL){ 
		Assert(n_poly == 0);// make sure this decal know's that it's empty
		//allocate a new poly
		poly = get_open_poly();
		//set it's parent to this
		poly->parent = this;
		//put it into this decal's poly list as the first
		ppoly = poly; //we can use this without getting a read/write to NULL error :)
	}else{
		//get a new poly and place it at the end of the list
		ppoly->next = get_open_poly();//add one to it
		//move the end poly back one so we can use the new poly we just got a little easier
		ppoly = ppoly->next;
	}
	if(ppoly == NULL){
		//ok, something when cataclismicly wrong :(
		//get_open_poly probly failed
		SAFEPOINT("leaveing decal::add_poly NULL");
		return NULL;
	}
	//ppoly is the end of the list, there should be no other polys after it
	ppoly->next = NULL;
	//well we've got one more poly than we did when we came in here
	n_poly++;
	//this is important to make sure it has the parent set corectly
	ppoly->parent = this;
#ifndef NDEBUG
	//hmm... don't ask...
	Assert(ppoly->parent == this);
#endif
	check_poly_decals();
	check_list_decals();
		SAFEPOINT("leaveing decal::add_poly Vailid");
	return ppoly;	//and return the new poly we got
}

void decal::remove_last_poly(){
	check_list_decals();
		SAFEPOINT("entering decal::remove_last_poly");
	check_poly_decals();

	decal_poly* p = poly;	//get the start of the list
	decal_poly *last = NULL;
	if(p==NULL){	// if it's NULL than there are no  polys in this list, return NULL
		Assert(!n_poly);
		SAFEPOINT("leaveing decal::remove_last_poly NULL");
		return ; //the first time
	}
	last = p;
	while(p->next != NULL){// untill we find one that doesn't have anymore afer it
		Assert(p->parent == this);//check for consistency
		last = p;
		p = p->next;//find the end of the list, move on down the line
	}
	last->next = NULL; //this is now the end of the list
	n_poly--;
	free_poly(p, false);	//I'm afraid you've outlived your usefulness as a polygon in this decal
	if(!n_poly)poly = NULL;
	//if there are no polys in the list after were done, kill off the list
	Assert(p->parent == NULL);
	check_poly_decals();
	check_list_decals();
		SAFEPOINT("leaveing decal::remove_last_poly valid");
}

//retuns the last polly of the decal
decal_poly *decal::get_last_poly(){
		SAFEPOINT("entering decal::get_last_poly");
	check_list_decals();
	check_poly_decals();

	decal_poly* p = poly;	//get the start of the list
	if(p==NULL){	// if it's NULL than there are no  polys in this list, return NULL
		SAFEPOINT("leaveing decal::get_last_poly NULL");
	check_list_decals();
	check_poly_decals();
		return p; //the first time
	}
	while(p->next != NULL){// untill we find one that doesn't have anymore afer it
		Assert(p->parent == this);//check for consistency
		p = p->next;//find the end of the list, move on down the line
	}
	//we got it
	Assert(p->parent == this);
	check_poly_decals();
		SAFEPOINT("leaveing decal::get_last_poly valid");
	//it's p
	check_list_decals();
	check_poly_decals();
	return p;
}

//frees all polys from a decal
void free_all_polys(decal* dec){
	if(!dec)return;
	check_list_decals();
	check_poly_decals();
	decal_poly* poly = dec->poly;
	while(poly){
		decal_poly* next_poly = poly->next;
		dec->n_poly--;
		free_poly(poly, false);
		poly = next_poly;
	}
	Assert(dec->n_poly == 0);
	//this decal is now empty make sure it is marked as such
	dec->poly = NULL;
	dec->n_poly = 0;
	check_list_decals();
	check_poly_decals();
}

/***************************/
/*END DECAL POLY MANAGEMENT*/
/***************************/




/************************/
/*BEGIN DECAL MANAGEMENT*/
/************************/

//finds the first free decal
//allocates it, be sure to free with free_decal_list
decal_list* get_open_decal_list(){

		SAFEPOINT("entering get_open_decal_list");
	check_poly_decals();
	check_list_decals();

	for(int i = 0; i<MAX_GLOBAL_DECALS; i++){
		if(!big_ol_decal_array[i].used){
			//look untill you find one that isn't used
			big_ol_decal_array[i].used = true;
			//mark it as used

				check_poly_decals();
				SAFEPOINT("leaveing get_open_decal_list");
				if(i > highest_decal_index){
					highest_decal_index = i;
					mprintf(("highest poly used: %d; highest decal used: %d", highest_poly_index, highest_decal_index));
				}

			return &big_ol_decal_array[i].dec;
			//and return it
		}
	}

	Warning(LOCATION, "get_open_decal_list failed\n");
		SAFEPOINT("leaveing get_open_decal_list");
	check_list_decals();
	check_poly_decals();
	return NULL;
}

//makes the low level decal management see this as free
void free_decal_list(decal_list* dec){
SAFEPOINT("entering free_decal_list");
	check_list_decals();
	check_poly_decals();
	for(int i = 0; i<MAX_GLOBAL_DECALS; i++){
		//find the decal in the array
		if(&big_ol_decal_array[i].dec == dec){
			//free it's pollys
			free_all_polys(&dec->dec);;
			//then mark it as not being used
#ifndef NDEBUG
			dec->dec.parent = NULL;
#endif
			big_ol_decal_array[i].used = false;
		}
	}
	check_list_decals();
	check_poly_decals();
SAFEPOINT("leaveing free_decal_list");
}

//add's a new decal to the list
decal_list *decal_list_controle::add_new(){
SAFEPOINT("entering decal_list_controle::add_new");
	check_list_decals();
	check_poly_decals();
	check_ship_decals();
	//if the end decal is empty just use that
	if(n_decals && !end->dec.n_poly){
//		n_decals++;
		return end;
	}
	
	//if d_list is empty, then this is the first decal, 
	//put the new decal at the start of the list (d_list)
	//otherwise use the decal after the last one in the list
	decal_list** dl = (d_list == NULL)?&d_list:&end->next;
	*dl = get_open_decal_list(); 
	if(dl == NULL){
		//if we couldn't get an open decal quit
		return NULL;
	}

	//if dl == d_list then this is the first decal
	//it's also the last decal, mark it acordingly
	if(&d_list == dl){
		end = d_list;
	}else{
	//otherwise move the end down one
		end = end->next;
	}
	//now that end has been set to the corect position, there should be no more decals after it
	//next should already be NULL due to the freeing function, but just to make sure
	end->next = NULL;

	//make sure this decal is empty, though it should be anyway
	decal_poly *poly = (*dl)->dec.poly;
	for(int i = 0; i<(*dl)->dec.n_poly; i++){
		//maybe an error mesage should go in here, becase this realy should never be getting called
		decal_poly *next = poly->next;
		free_poly(poly);
		poly = next;
	}
	Assert(poly == NULL);

	//another make-absolutely-sure-the-data-leaveing-this-function-is-corect thingy
	(*dl)->dec.n_poly = 0;
	(*dl)->dec.poly = NULL;

	//we now have one more decal in this list than we did before
	n_decals++;

	modifyed = true; //mark this decal list as haveing been modifyed
check_ship_decals();
check_list_decals();
check_poly_decals();
SAFEPOINT("leaveing decal_list_controle::add_new");
#ifndef NDEBUG
	(*dl)->dec.parent = this;
#endif
	check_list_decals();

	check_ship_decals();

	return *dl;
};

//removes the first item from the list and puts the emptied decal on the end
void decal_list_controle::trim(){
SAFEPOINT("entering decal_list_controle::trim");

	check_ship_decals();
	check_list_decals();
	check_poly_decals();
	//get the poly list
//hack fix
	if(!d_list)Error(LOCATION, "decal list is null in trim");//return;
	decal_poly *poly = d_list->dec.poly;
	if(poly)free_all_polys(&d_list->dec);
/*	for(int i = 0; i<d_list->dec.n_poly; i++){
		if(!poly)Error(LOCATION, "poly list is corupt in trim");//break;
		//grab wich poly is to be next
		decal_poly *next = poly->next;
		//then kill the poly
		free_poly(poly);
		//move to the next poly
		poly = next;
	}*/
	else
		if(d_list->dec.n_poly)Error(LOCATION, "poly list is missing in trim");
#ifndef NDEBUG
	d_list->dec.parent = this;//heh, becase it's still part of this decal list,, duh...
#endif
SAFEPOINT("decal_list_controle::trim; polys cleared");
	//the poly being pointed to here is no longer valid, 
	//also marking this as null means that there are no polys in it
	if(!d_list)Error(LOCATION, "decal list is null in trim 1");//return;
	d_list->dec.poly = NULL;
	//poly should have made it to the end of the list, make sure that it did
	if(!d_list)Error(LOCATION, "decal list is null in trim 2");//return;
//	Assert(poly == NULL);
	d_list->dec.n_poly = 0;
	if(!d_list)Error(LOCATION, "decal list is null in trim 3");//return;
	//the end is the first
	end->next = d_list;
	if(!d_list)Error(LOCATION, "decal list is null in trim 4");//return;
	//the end is pushed back
	end = end->next;
	if(!d_list)Error(LOCATION, "decal list is null in trim 5");//return;
	//the first is the second
	d_list = d_list->next;
	//the end is NULLed off
	if(!d_list)Error(LOCATION, "decal list is null in trim 6");//return;
	end->next = NULL;
	if(!d_list)Error(LOCATION, "decal list is null in trim 7");//return;
	check_poly_decals();
	modifyed = true; //mark this decal list as haveing been modifyed
	if(!d_list)Error(LOCATION, "decal list is null at the end of trim");//return;
SAFEPOINT("leaveing decal_list_controle::trim");

	check_ship_decals();
	check_list_decals();
	check_poly_decals();
//no decals were killed in the recycleing of this decal
};

void decal_list_controle::clear(){
SAFEPOINT("entering deecal_list_controle::clear");
check_ship_decals();
	check_list_decals();
	check_poly_decals();

	decal_list *n = d_list;
	decal_list *c = d_list;
	decal_list *list = d_list;

	while(n != NULL){
		n = list->next;
		free_decal_list(list);
		c = list; //this is to make sure it's getting to the end of the list
		list = n;
		n_decals--;
	}	
	Assert(c == end);//see it got to the end, stop worrying
	Assert(!n_decals);
	d_list = NULL;

	modifyed = true; //mark this decal list as haveing been modifyed, might as well even though it probly won't getting used
	check_poly_decals();
	check_list_decals();
	check_ship_decals();

SAFEPOINT("leaveing deecal_list_controle::clear");
};

decal_list_controle::decal_list_controle():n_decals(0),buffer(-1),texture(-1),modifyed(false),d_list(NULL),end(NULL){
}

decal_list_controle::~decal_list_controle(){
	clear();
};

/****high level decal management****/

//find a decal list with the texture we want from an array of decal lists
decal_list_controle *find_list(decal_list_controle* list, int n_textures, int texture){
	for(int i = 0; i < n_textures; i++){
		//if this decal list is useing our text return it
		if(list[i].texture == texture)return &list[i];
	}
	//couldn't find any
	return NULL;
}

decal *decal_find_next(decal_system* system, int texture, int glow_texture = -1, int burn_texture = -1){
check_list_decals();
check_poly_decals();
check_ship_decals();
SAFEPOINT("entering decal_find_next");

	//see if you can find a decal list already in exsistence that uses the texture we want
	decal_list_controle* list = find_list( system->decals, system->n_decal_textures, texture);
	if(list == NULL){	
		//if it couldn't find the texture, this means we need to 
		//realocate the array and add one
		decal_list_controle* old_list = system->decals;
		//save the current list
		system->decals = (decal_list_controle *) malloc(sizeof(decal_list_controle)*++system->n_decal_textures); 

		if(!system->decals)Error(LOCATION, "unable to malloc new decal list");
		//reallocate

		memcpy(system->decals, old_list, sizeof(decal_list_controle)*(system->n_decal_textures-1));
		//copy the contents of the old list into the new one

		free(old_list);
		//free the old list

		list = &system->decals[system->n_decal_textures-1];
		list->buffer = -1;
		list->d_list = NULL;
		list->end = NULL;
		list->texture = texture;
		list->burn_texture = burn_texture;
		list->glow_texture = glow_texture;
		list->n_decals = 0;

		for(int i = 0; i<system->n_decal_textures -1; i++){
			decal_list *decl = system->decals[i].d_list;
			if(decl == NULL){
				Error(LOCATION, "list controle has no list");
				break;
			}
			while(decl != NULL){
				decl->dec.parent = &system->decals[i];
				decl = decl->next;
			}
		}

		//set up the innital values for a new list, 
		//nulling out the first members setting the textures ect...
check_ship_decals();
	}
	if(!system->decals)Error(LOCATION, "decal list lost");
	//ok so now we've got the list controle
	//lets get the actual linked list of decals
	decal_list* l = list->d_list;
bool trimcalled = false;
	if(!system->max_decals)system->max_decals = 10;//this should never happen, but it is and I'm to lazy to fix it properly
	if(list->n_decals >= system->max_decals){
		list->trim();
		trimcalled = true;
	}
		//if we have the max number of decals kill the oldest one. 
		//this will no longer allocate any memory, it just keeps recycleling what it has

	if(list->n_decals == 0){
		l = list->add_new();
		// no decals in this list, so we need to make one
	}else if(list->end->dec.n_poly != 0){
		l = list->add_new();	
		//if the last decal already has polys in it get a new one
	}
	if(l == NULL){
		//umm, something went realy wrong, not sure what happens if this happens
		SAFEPOINT("leaveing decal_find_next NULL");
		return NULL;
	}
	// so now we have a good (hopefully empty) decal at the end of our list
SAFEPOINT("leaveing decal_find_next");
check_poly_decals();
check_ship_decals();
check_list_decals();
	if(!system->decals)Error(LOCATION, "decal list lost");

	list->modifyed = true;//it should aslways read as modified after this function
	return &list->end->dec;
	//return a pointer to the decal at the end of the list
}

void clear_decals(decal_system	*system){
#ifdef DECALS_ENABLED
	int i;
	if(!system->n_decal_textures)return;
	for( i = 0; i<system->n_decal_textures; i++)system->decals[i].clear();
	for( i = 0; i<system->n_decal_textures; i++)Assert(!system->decals[i].n_decals);
	system->n_decal_textures = 0;
	free(system->decals);
	system->decals = NULL;
#endif
}
/**********************/
/*END DECAL MANAGEMENT*/
/**********************/



static vec3d decal_cube_point[8];
static vec3d decal_cube_plane[6][2];
static vec3d decal_hit_point;
static vec3d decal_hit_vect;
static float decal_hit_radius;
static float decal_min_hit_radius;

static matrix decal_orient;
//the orientation of the decal

static decal * new_decal;
static decal * new_decal_backface;

static int nverts;

static float min_rad;
float max_rad;	
// the minimum radius of the decal anything within this distance _is_ in the decal

static int back_faceing;	//global boolian type thing telling the low level stuff it it's going to care about back faced polys

//early simple decal creation code, I don't even know if it works, 
//saveing it becase I may have a use for this type off effect later
int decal_create_simple(object *obj, decal_point *point, int texture){//makes a simple non-clipped decal
#ifdef DECALS_ENABLED
	mprintf(("a decal is about to be made at %0.2f, %0.2f, %0.2f\n", point->pnt.xyz.x, point->pnt.xyz.y, point->pnt.xyz.z));
	mprintf(("orient fvec at %0.2f, %0.2f, %0.2f\n", point->orient.vec.fvec.xyz.x, point->orient.vec.fvec.xyz.y, point->orient.vec.fvec.xyz.z));

	if(obj->type != OBJ_SHIP){
		return 0;
	}
//	vertex vert[1][2];
//	vec3d vec[4];

	vec3d center = point->pnt;
	float rad = point->radius;
	if(rad <=0) rad = 10;
	mprintf(("radius %f\n",rad));
	vec3d plain_point[4];
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
	ship *shipp = &Ships[obj->instance];

	decal *dec = decal_find_next(&shipp->ship_decal_system, texture, -1, -1);
check_ship_decals();
	if(!dec)return 0;

	vertex vert[4];
	vec3d _vert[4];

	for(int i = 0; i<4; i++){
		vm_vec_scale_add( &_vert[i], &center, &plain_point[i], rad );
		vm_vec2vert(&_vert[i], &vert[i]);
	}

	
	decal_poly *poly = dec->add_poly();;

	poly->point[0] = vert[0];
	poly->point[1] = vert[1];
	poly->point[2] = vert[2];

	poly->point[0].u = 0;	poly->point[0].v = 0;
	poly->point[1].u = 0;	poly->point[1].v = 1;
	poly->point[2].u = 1;	poly->point[2].v = 1;

	poly = dec->add_poly();

	poly->point[0] = vert[0];
	poly->point[1] = vert[3];
	poly->point[2] = vert[2];

	poly->point[0].u = 0;	poly->point[0].v = 0;
	poly->point[1].u = 1;	poly->point[1].v = 0;
	poly->point[2].u = 1;	poly->point[2].v = 1;

//	dec->texture = texture;
	dec->n_poly = 2;

	shipp->ship_decal_system.decals_modified = true;

	mprintf(("a decal should have been made at %0.2f %0.2f %0.2f\n", point->pnt.xyz.x, point->pnt.xyz.y, point->pnt.xyz.z));
//Int3();
#endif
	return 1;
}

//BSP parseing macros
#define duw(p)	(*((uint *) (p)))
#define dw(p)	(*((int *) (p)))
#define dwp(p)	((int *) (p))
#define dvp(p)	((vec3d *) (p))
#define dfl(p)	(*((float *) (p)))


//creates a decal, returns decal index on succes, -1 on failure
void setup_decal_cube(matrix m, vec3d cube_point[8]){

//setting up the decal cube
	vec3d test_vec, temp1, temp2;
	for(int i = 0; i<8; i++)decal_cube_point[i] = cube_point[i];

	decal_orient = m;

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
}

/**************/
/*DECAL CREATE*/
/**************/

decal_poly *LAST_POLY;
int decal_create(object * obj, decal_point *point, int subobject, int texture, int backfaced_texture, int glow_texture, int burn_texture, int burn_time){
	if(!Cmdline_decals)return -1;
//	return -1;
#ifdef DECALS_ENABLED
SAFEPOINT("entering create_decal");
	mprintf(("a decal is about to be made at %0.2f, %0.2f, %0.2f\n", point->pnt.xyz.x, point->pnt.xyz.y, point->pnt.xyz.z));

	if(obj->type != OBJ_SHIP){
		return -1;
	}
	if(texture < 0)return -1;
	if(point->radius <= 0)return -1;
//TIMERBAR_PUSH(TIMERBAR_GREY);

	vec3d center = point->pnt;
	float rad = point->radius;
	if(rad <=0) rad = 1;
	mprintf(("radius %f\n",rad));
	vec3d plain_point[4];
	vec3d cube_point[8];
//define the decals dimentions
	plain_point[0] = point->orient.vec.uvec;
	plain_point[1] = plain_point[0];
		vm_vec_negate(&plain_point[1]);
	plain_point[2] = point->orient.vec.rvec;
	plain_point[3] = plain_point[2];
		vm_vec_negate(&plain_point[3]);

	vec3d topcenter;
	vec3d bvec = point->orient.vec.fvec;
	vm_vec_negate(&bvec);
	vm_vec_scale_add(&topcenter, &center, &bvec, rad);

	for(int i = 0; i < 4; i++){
		vm_vec_scale_add( &cube_point[i], &topcenter, &plain_point[i], rad );
		vm_vec_scale_add( &cube_point[i+4], &cube_point[i], &point->orient.vec.fvec, rad*2);
	}

	max_rad = vm_vec_dist_quick(&center, &cube_point[0]);	//the cube points are all equidistant, right?
	min_rad = fl_sqrt((rad * rad)/2);	//the cube points are all equidistant, right?
mprintf(("decal defined\n"));

//define the decals dimentions
	
//set up the decal model
	ship *shipp = &Ships[obj->instance];
//	decal *sdec = shipp->decals;

	polymodel *pm = model_get(shipp->modelnum);
	bool hit_main_hull = (pm->detail[0] == subobject);
	bsp_info * sm;
	sm = &pm->submodel[subobject];

	model_subsystem* psub = NULL;
	//subsys_list
	ship_subsys *sys = GET_FIRST(&shipp->subsys_list);;
	while(sys != END_OF_LIST(&shipp->subsys_list)){
		if(sys->system_info->subobj_num == subobject){
			psub = sys->system_info;
			break;
		}
		sys = GET_NEXT(sys);
	}

	if(!hit_main_hull && !psub)return -1;

	decal_system* system = (hit_main_hull)?&shipp->ship_decal_system:&psub->model_decal_system;
//	decal_model dmod = pm->submodel[subobject].dec_model;// sm->dec_model;
//	decal_model tdec;	//temporary decal model for holding the culled down part of the decal
	
	setup_decal_cube(point->orient, cube_point);

	decal_hit_point = point->pnt;
	decal_hit_radius = max_rad;
	
//	decal_min_hit_radius = min_rad;
	decal_min_hit_radius = rad;
	decal_hit_vect = point->orient.vec.fvec;
	nverts = 0;

//go and find decals for each submodel
//	for(int z = 0; z<pm->n_models;z++){

	if(backfaced_texture != -1)
		new_decal_backface = decal_find_next(system, backfaced_texture,-1,-1);
	else
		new_decal_backface = NULL;

//check_poly_decals();
	decal *sdec = decal_find_next(system, texture, glow_texture, burn_texture);
//check_poly_decals();
	Assert(sdec != NULL);
//	mprintf(("decal is number %d\n", dinx));

//	sdec->is_valid = 0;

		new_decal = sdec;

		if(backfaced_texture != -1){
			back_faceing = 1;
		}else{
			back_faceing = 0;
		}
		//no decal should have any polys before it's been created
		Assert(new_decal->n_poly == 0);
		if(backfaced_texture != -1)
			Assert(new_decal_backface->n_poly == 0);
SAFEPOINT("");
		LAST_POLY = new_decal->add_poly();
SAFEPOINT("");
		decal_create_sub((ubyte *)pm->submodel[subobject].bsp_data);
		new_decal->remove_last_poly();
		//there's always one too many polygons made, becase one is always needed to be worked with

mprintf(("deacal defined\n"));
	if(new_decal->n_poly){
		new_decal->timestamp = timestamp();
		new_decal->burn_time = burn_time;
	}

/*	sdec->texture = texture;
	sdec->backfaced_texture = backfaced_texture;
	sdec->is_valid = 1;
	sdec->timestamp = timestamp();
	sdec->state = 1;
	sdec->lifetime = 3000;
	sdec->frames = 1;
	sdec->importance = 0;
	sdec->radius = max_rad;
	sdec->position = point->pnt;
*/
	
	decal_total_decals_made++;
	total_polys_used += sdec[0].n_poly;

	mprintf(("a decal should have been made with %d polys,\n %d decals have been made with an average of %0.3f polys eachf\n",sdec[0].n_poly, decal_total_decals_made, float(total_polys_used)/float(decal_total_decals_made)));

	system->decals_modified = true;
check_poly_decals();

//TIMERBAR_POP();
return 1;

#else
return -1;
#endif
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

	decal_realc_pointlist(nverts);

	mprintf(("%d verts\n",nverts));
	ubyte * normcount = p+20;
	vec3d *src = dvp(p+offset);
	
//>	Assert( nverts < 1200 );



	for (n=0; n<nverts; n++ )	{

		Assert(decal_points_alocated >= n);

		decal_point_list[n] = src;
//		new_decal->vert[n].xyz = src->xyz;
		src += normcount[n]+1;
//		mprintf(("defpoint %d, x %0.2f, y %0.2f, z %0.2f\n",n,d_model.vert[n].xyz.x,d_model.vert[n].xyz.y,d_model.vert[n].xyz.z));
	} 

}


// Sortnorms
// +0      int         id
// +4      int         size 
// +8      vec3d      normal
// +20     vec3d      center
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

	vec3d *normal, *point;
	normal = dvp(p+8);
	point = dvp(p+20);

	vec3d min, max, h;
	min.xyz = dvp(p+56)->xyz;
	max.xyz = dvp(p+68)->xyz;
	min.xyz.x -= decal_hit_radius;
	min.xyz.y -= decal_hit_radius;
	min.xyz.z -= decal_hit_radius;
	max.xyz.x += decal_hit_radius;
	max.xyz.y += decal_hit_radius;
	max.xyz.z += decal_hit_radius;

	if (!fvi_ray_boundingbox( &min, &max, &decal_hit_point, &decal_hit_vect, &h ))	{
		mprintf(("leaveing from sortnorm bbox\n"));
		return;
	}

	float rad = dfl(p+32);
	float dist = vm_vec_dist( &decal_hit_point, point);
	if(dist <= (rad + max_rad)){
		mprintf(("leaveing from sortnorm rad\n"));
		return;
	}

	float decal_plane_dist = fvi_point_dist_plane(point, normal, &decal_hit_point);
	bool within = fl_abs(decal_plane_dist) < max_rad;

	if (prelist) decal_create_sub(p+prelist);

	if(within)
		if (onlist) decal_create_sub(p+onlist);

	if(decal_plane_dist > 0.0f){
		if (frontlist) decal_create_sub(p+frontlist);
		if (backlist) decal_create_sub(p+backlist);
	}else{
		if (backlist) decal_create_sub(p+backlist);
		if (frontlist) decal_create_sub(p+frontlist);
	}

	if(!within)
		if (onlist) decal_create_sub(p+onlist);

	if (postlist) decal_create_sub(p+postlist);
	return;
}

/*
typedef struct model_tmap_vert {
	short vertnum;
	short normnum;
	float u,v;
} model_tmap_vert;
*/

//if all the UVs are out of range this poly can be discarded
bool check_uv(decal_poly *poly){
	if(poly->point[0].u < 0.0f 
		&&
		poly->point[1].u < 0.0f 
		&&
		poly->point[2].u  < 0.0f 
		)
		return false;
	if(poly->point[0].v < 0.0f 
		&&
		poly->point[1].v < 0.0f 
		&&
		poly->point[2].v  < 0.0f 
		)
		return false;
	if(poly->point[0].u > 1.0f 
		&&
		poly->point[1].u > 1.0f 
		&&
		poly->point[2].u  > 1.0f 
		)
		return false;
	if(poly->point[0].v > 1.0f 
		&&
		poly->point[1].v > 1.0f 
		&&
		poly->point[2].v  > 1.0f 
		)
		return false;

	return true;
}

//checks the distance from the capping planes
bool check_ab(float a[3], float b[3]){
	if(a[0] > 0.0f 
		&&
		a[1] > 0.0f 
		&&
		a[2]  > 0.0f 
		)
		return false;
	if(b[0] > 0.0f 
		&&
		b[1] > 0.0f 
		&&
		b[2]  > 0.0f 
		)
		return false;
	/*
	if(a[0] < 0.0f 
		&&
		a[1] < 0.0f 
		&&
		a[2]  < 0.0f 
		)
		return false;
	if(b[0] < 0.0f 
		&&
		b[1] < 0.0f 
		&&
		b[2]  < 0.0f 
		)
		return false;
	if(a[0] > 1.0f 
		&&
		a[1] > 1.0f 
		&&
		a[2]  > 1.0f 
		)
		return false;
	if(b[0] > 1.0f 
		&&
		b[1] > 1.0f 
		&&
		b[2]  > 1.0f 
		)
		return false;
*/
	return true;
}

//tells weather a sphere colides with a triangle
bool shpere_tri_edge(vec3d* s_point, float rad, vertex vert[3]){
	vec3d n;
	float d;
	for(int i = 0; i< 3; i++){
		vec3d point_one;
		vm_vert2vec(&vert[i], &point_one);
		vec3d point_two;
		vm_vert2vec(&vert[(i+1)%3], &point_two);
		switch(vm_vec_dist_to_line(s_point, &point_one, &point_two, &n, NULL)){
				// behind the line, so use the start pos
			case -1:
				d = vm_vec_dist(s_point,&point_one);
				break;
		
				// use the closest point
			case 0:
				d = vm_vec_dist(s_point,&n);
				break;

				// past the line, so use the shot pos
			case 1:
				d = vm_vec_dist(s_point,&point_two);
				break;
			default:
				d = 0.0f;
				Int3();
				break;
			}

		Assert(d > 0);
		if(d < rad)return true;
	}
	return false;
}

bool decal_tmap_bbox(vec3d *test, float rad){
	if(fl_abs(test->xyz.x) > rad)return false;
	if(fl_abs(test->xyz.y) > rad)return false;
	if(fl_abs(test->xyz.z) > rad)return false;
	return true;
}

void decal_clip_line(vertex *out,vertex* p1, vertex* p2, float test, bool U){ 
	//test is wich part of the UV plane were testing 0.0 or 1.0
	//U is if were testing the u of the uv pair, if false then were testing v
	float iterp = 0.0;
	if(U)iterp = (p1->u - test)/(p1->u - p2->u);
	else iterp = (p1->v - test)/(p1->v - p2->v);
	out->x = ((p2->x-p1->x)*iterp)+p1->x*iterp;
	out->y = ((p2->y-p1->y)*iterp)+p1->y*iterp;
	out->z = ((p2->z-p1->z)*iterp)+p1->z*iterp;
	if(U){
		out->u = ((p2->u-p1->u)*iterp)+p1->u;
		out->v = p1->v;;
	}else{
		out->u = p1->u;
		out->v = ((p2->v-p1->v)*iterp)+p1->v;
	}
}

int decal_clip_tri(vertex in[3], vertex out[20]){
	vertex work[20];
	memcpy(work, in, sizeof(vertex)*3);
	int n_verts = 0;
	int old_n_verts = 3;
	int i;

	for(i = 0; i<old_n_verts; i++){
		//for each edge, in[i], in[(i+1)%3]
		if((work[i].u <= 1.0f)){
			//if the starting point is ok, copy it
			out[n_verts++] = work[i];

			if(!(work[(i+1)%old_n_verts].u <= 1.0f)){
				//if the end point is bad, clip between them
				decal_clip_line(&out[n_verts++], &work[i], &work[(i+1)%old_n_verts], 1.0f, true);
				//don't clip between two bad points
			}
		}else if((work[(i+1)%old_n_verts].u <= 1.0f)){
			//if the start point is bad, but the end point is good clip it
			decal_clip_line(&out[n_verts++], &work[i], &work[(i+1)%old_n_verts], 1.0f, true);
			//don't clip between two bad points
		}

	}
	old_n_verts = n_verts;
	n_verts = 0;
	memcpy(work, out, sizeof(vertex)*20);
	for( i = 0; i<old_n_verts; i++){
		//for each edge, in[i], in[(i+1)%3]
		if((work[i].u >= 0.0f)){
			//if the starting point is ok, copy it
			out[n_verts++] = work[i];

			if(!(work[(i+1)%old_n_verts].u >= 0.0f)){
				//if the end point is bad, clip between them
				decal_clip_line(&out[n_verts++], &work[i], &work[(i+1)%old_n_verts], 0.0f, true);
				//don't clip between two bad points
			}
		}else if((work[(i+1)%old_n_verts].u >= 0.0f) ){
			//if the start point is bad, but the end point is good clip it
			decal_clip_line(&out[n_verts++], &work[i], &work[(i+1)%old_n_verts], 0.0f, true);
			//don't clip between two bad points
		}

	}

	old_n_verts = n_verts;
	n_verts = 0;
	memcpy(work, out, sizeof(vertex)*20);
	for( i = 0; i<old_n_verts; i++){
		//for each edge, in[i], in[(i+1)%3]
		if((work[i].v <= 1.0f)){
			//if the starting point is ok, copy it
			out[n_verts++] = work[i];

			if(!(work[(i+1)%old_n_verts].v <= 1.0f)){
				//if the end point is bad, clip between them
				decal_clip_line(&out[n_verts++], &work[i], &work[(i+1)%old_n_verts], 1.0f, false);
				//don't clip between two bad points
			}
		}else if((work[(i+1)%old_n_verts].v <= 1.0f) ){
			//if the start point is bad, but the end point is good clip it
			decal_clip_line(&out[n_verts++], &work[i], &work[(i+1)%old_n_verts], 1.0f, false);
			//don't clip between two bad points
		}

	}

	old_n_verts = n_verts;
	n_verts = 0;
	memcpy(work, out, sizeof(vertex)*20);
	for( i = 0; i<old_n_verts; i++){
		//for each edge, in[i], in[(i+1)%3]
		if((work[i].v >= 0.0f)){
			//if the starting point is ok, copy it
			out[n_verts++] = work[i];

			if(!(work[(i+1)%old_n_verts].v >= 0.0f)){
				//if the end point is bad, clip between them
				decal_clip_line(&out[n_verts++], &work[i], &work[(i+1)%old_n_verts], 0.0f, false);
				//don't clip between two bad points
			}
		}else if((work[(i+1)%old_n_verts].v >= 0.0f) ){
			//if the start point is bad, but the end point is good clip it
			decal_clip_line(&out[n_verts++], &work[i], &work[(i+1)%old_n_verts], 0.0f, false);
			//don't clip between two bad points
		}

	}
	return n_verts;
}

float fvi_point_dist_plane(vec3d* pl_pnt, vec3d *pl_norm, vertex *vert){
	vec3d vec;
	vm_vert2vec(vert, &vec);
	return fvi_point_dist_plane(pl_pnt, pl_norm, &vec);
}

int decal_plane_clip_tri(vertex in[3], vertex out[20]){
	//decal_cube_plane[point][norm]
	vertex work[20];
	memcpy(work, in, sizeof(vertex)*3);
	int n_verts = 0;
	int old_n_verts = 3;
	vec3d temp, temp2, temp3;

	for(int p = 0; p<6; p++){
		for(int i = 0; i<old_n_verts; i++){
			//for each edge, in[i], in[(i+1)%3]
			if(fvi_point_dist_plane(&decal_cube_plane[p][0], &decal_cube_plane[p][1], &work[i]) > 0.0){
				//if the starting point is ok, copy it
				out[n_verts++] = work[i];
	
				if(!(fvi_point_dist_plane(&decal_cube_plane[p][0], &decal_cube_plane[p][1], &work[(i+1)%old_n_verts]) > 0.0)){
					//if the end point is bad, clip between them
					vm_vert2vec(&work[(i+1)%old_n_verts], &temp2);
					vm_vert2vec(&work[i], &temp3);
					fvi_segment_plane(&temp, &decal_cube_plane[p][0], &decal_cube_plane[p][1], &temp3, &temp2, 0.0f);
					vm_vec2vert(&temp, &out[n_verts++]);
					//don't clip between two bad points
				}
			}else if(fvi_point_dist_plane(&decal_cube_plane[p][0], &decal_cube_plane[p][1], &work[(i+1)%old_n_verts]) > 0.0){
				//if the start point is bad, but the end point is good clip it
				vm_vert2vec(&work[i], &temp3);
				vm_vert2vec(&work[(i+1)%old_n_verts], &temp2);
				fvi_segment_plane(&temp, &decal_cube_plane[p][0], &decal_cube_plane[p][1], &temp3, &temp2, 0.0f);
				vm_vec2vert(&temp, &out[n_verts++]);
				//don't clip between two bad points
			}
	
		}
		old_n_verts = n_verts;
		n_verts = 0;
		memcpy(work, out, sizeof(vertex)*20);
	}
	return old_n_verts;
}

// Textured Poly
// +0      int         id
// +4      int         size 
// +8      vec3d      normal
// +20     vec3d      normal_point
// +32     int         radius
// +36     int         nverts
// +40     int         tmap_num
// +44     nverts*(model_tmap_vert) vertlist (n,u,v)
void decal_create_tmappoly(ubyte * p)
{

//	mprintf(("entering decal_create_tmappoly\n"));



	bool clip = true;
	int i;
	int nv;
//	uv_pair uvlist[TMAP_MAX_VERTS];
	vec3d *points[TMAP_MAX_VERTS];
	model_tmap_vert *verts;

	nv = dw(p+36);
	if ( nv < 0 ) return;
//TIMERBAR_PUSH(TIMERBAR_WHITE);
	verts = (model_tmap_vert *)(p+44);

	for (i=0;i<nv;i++)	{
		decal__poly[i] = (int)verts[i].vertnum;
		points[i] = decal_point_list[decal__poly[i]];
	}

	vec3d *pnorm = dvp(p+8);
	vec3d norm = *pnorm;
	vm_vec_normalize(&norm);

	//float back = 1.0;

//	if( (back_faceing == 0) && (back < 0) )return;

//poly culling 
	vec3d *pcenter = (vec3d*)(p+20);
	float pradius = *(float*)(p+32);
	vec3d dir_to_dec;
	vm_vec_sub(&dir_to_dec, pcenter, &decal_hit_point);
	vm_vec_normalize(&dir_to_dec);
	float dot = vm_vec_dot(&decal_hit_vect, &norm);

	if(dot > -0.005){
//		TIMERBAR_POP();
		return;
	}
/*	float dist = 0;
	for (i=0;i<nv;i++)	{
		dist = vm_vec_dist(decal_point_list[decal__poly[i]], &pcenter);
		if(dist>pradius){
			pradius = dist;
		}
	}
*/
	float decal_plane_dist = fl_abs(fvi_point_dist_plane(pcenter, pnorm, &decal_hit_point));
	if(decal_plane_dist > max_rad*1.5f){
//		TIMERBAR_POP();
		return;
	}

	vec3d closest;
	vm_project_point_onto_plane(&closest, &decal_hit_point, &norm, pcenter);

	if(clip && !fvi_point_face(&closest, nv, points, &norm, NULL, NULL, NULL)){
		// if the point isn't on the face, find the line segment it is closest to 
		//and make that point the closest
		float distance = FLT_MAX, test_dist = FLT_MAX;
		vec3d nearest = closest, test_nearest;
		for(i = 0; i<nv; i++){
			switch (vm_vec_dist_to_line(&closest,decal_point_list[decal__poly[i]],decal_point_list[decal__poly[(i+1)%nv]],&test_nearest, &test_dist)){
				case -1: 
					if(test_dist < distance){
						nearest = *decal_point_list[decal__poly[i]];
						distance = test_dist;
					}
					break;
				case 0: 
					if(test_dist < distance){
							nearest = test_nearest;
							distance = test_dist;
					}
					break;
				case 1:
					if(test_dist < distance){
						nearest = *decal_point_list[decal__poly[(i+1)%nv]];
						distance = test_dist;
					}
						break;
				}
		}
		closest = nearest;
	}

	vec3d local_closest;
	if(clip){
	vm_vec_sub(&local_closest, &closest, &decal_hit_point);
	vec3d ctemp = local_closest;
	vm_vec_rotate(&local_closest,&ctemp, &decal_orient);
	}

	bool defenately = !clip || (decal_tmap_bbox(&local_closest, decal_min_hit_radius));

	if(!defenately && max_rad*1.5 < vm_vec_dist(&closest, &decal_hit_point)){
		mprintf(("bugging out becase closest is out of max_rad\n"));		
//		TIMERBAR_POP();
		return;
	}
	decal* set_decal = new_decal;
	if(set_decal->n_poly >= MAX_DECAL_POLY-1){
		mprintf(("bugging out becase there are too many polys in the decal\n"));		
//		TIMERBAR_POP();
		return;
	}
	vertex work_poly[20];
	if(nv > 2){
	//	decal_poly *poly = set_decal->get_last_poly();
		if(clip){
			//cliped decals
			for (i=1;i<(nv-1);i++)	{
				if(set_decal->n_poly >= MAX_DECAL_POLY-1){
					mprintf(("bugging out becase there are too many polys in the decal\n"));		
//					TIMERBAR_POP();
					return;
				}
			
				Assert(decal_points_alocated >= i%nv);
				Assert(decal_points_alocated >= (i+1)%nv);
				Assert(decal_points_alocated >= decal__poly[0]);
				Assert(decal_points_alocated >= decal__poly[i%nv]);
				Assert(decal_points_alocated >= decal__poly[(i+1)%nv]);
	
				vm_vec2vert(decal_point_list[decal__poly[0]], &LAST_POLY->point[0]);
				vm_vec2vert(decal_point_list[decal__poly[i%nv]], &LAST_POLY->point[1]);
				vm_vec2vert(decal_point_list[decal__poly[(i+1)%nv]], &LAST_POLY->point[2]);
				LAST_POLY->norm[0] = norm;
				LAST_POLY->norm[1] = norm;
				LAST_POLY->norm[2] = norm;
//				set_decal->poly[set_decal->n_poly].norm = norm;
//				set_decal->n_poly++;
				int k;
				float a[3], b[3];
				for (k=0;k<3;k++)	{
					vec3d temp;
					vm_vert2vec(&LAST_POLY->point[k], &temp);
					b[k] = (fvi_point_dist_plane(&decal_cube_plane[4][0], &decal_cube_plane[4][1], &temp) / (max_rad*3));
					a[k] = (fvi_point_dist_plane(&decal_cube_plane[5][0], &decal_cube_plane[5][1], &temp) / (max_rad*3));
				}
				for ( k=0;k<3;k++)	{
					//calculate the UV coords
					vec3d temp;
					vm_vert2vec(&LAST_POLY->point[k], &temp);
					LAST_POLY->point[k].u = (fvi_point_dist_plane(&decal_cube_plane[0][0], &decal_cube_plane[0][1], &temp) / (min_rad*2));
					LAST_POLY->point[k].v = (fvi_point_dist_plane(&decal_cube_plane[2][0], &decal_cube_plane[2][1], &temp) / (min_rad*2));
				}
//				bool uv_passed = check_uv(LAST_POLY);
//				mprintf(("decal poly %d, vert 0 being set to %d, vert 1 %d, and vert 2 %d\n", set_decal->n_poly, new_decal->poly[set_decal->n_poly].point[0], new_decal->poly[set_decal->n_poly].point[1], new_decal->poly[set_decal->n_poly].point[2]));
				int work_n_verts = 0;;
				/*if(uv_passed)*/{
					if(pradius > max_rad){
						work_n_verts = decal_plane_clip_tri(LAST_POLY->point, work_poly);
					}else{
						memcpy(work_poly, LAST_POLY->point, sizeof(vertex)*3);
						work_n_verts = 3;
					}
					for( int j = 1; j < work_n_verts-1; j++){
	
						LAST_POLY->point[0] = work_poly[0];
						LAST_POLY->point[1] = work_poly[j];
						LAST_POLY->point[2] = work_poly[(j+1)%work_n_verts];
	
						for ( k=0;k<3;k++)	{
							//calculate the UV coords
							vec3d temp;
							vm_vert2vec(&LAST_POLY->point[k], &temp);
							LAST_POLY->point[k].u = (fvi_point_dist_plane(&decal_cube_plane[0][0], &decal_cube_plane[0][1], &temp) / (min_rad*2));
							LAST_POLY->point[k].v = (fvi_point_dist_plane(&decal_cube_plane[2][0], &decal_cube_plane[2][1], &temp) / (min_rad*2));
						}
	
						bool uv_passed = check_uv(LAST_POLY);
						if(defenately){
							LAST_POLY = set_decal->add_poly();
							mprintf(("there are now %d polys in the current decal\n", set_decal->n_poly));
						}else
						if(uv_passed && IS_VEC_NULL(&local_closest)){
							//if local_closest is null than the poly is coplaner to the poly that was hit, 
							//only if it's UVs are out of range would it not count
							LAST_POLY = set_decal->add_poly();
							mprintf(("there are now %d polys in the current decal\n", set_decal->n_poly));
						}else 
						if(uv_passed && check_ab(a,b)){
							LAST_POLY = set_decal->add_poly();
							mprintf(("there are now %d polys in the current decal\n", set_decal->n_poly));
						}/*else 
						if(shpere_tri_edge(&decal_hit_point, decal_min_hit_radius, LAST_POLY->point)){
							LAST_POLY = set_decal->add_poly();
							mprintf(("there are now %d polys in the current decal, sphere colide\n", set_decal->n_poly));
						}else
						if(shpere_tri_edge(&decal_hit_point, decal_hit_radius, LAST_POLY->point)){
							LAST_POLY = set_decal->add_poly();
							mprintf(("there are now %d polys in the current decal, big sphere colide\n", set_decal->n_poly));
						}*/
					}
				}
			}
		}else{
			for (i=1;i<(nv-1);i++)	{
		//		for (int k=0;k<3;k++)	{
					//calculate the UV coords
					vm_vec2vert(decal_point_list[decal__poly[0]], &LAST_POLY->point[0]);
					vec3d temp;
					vm_vert2vec(&LAST_POLY->point[0], &temp);
					LAST_POLY->point[0].u = (fvi_point_dist_plane(&decal_cube_plane[0][0], &decal_cube_plane[0][1], &temp) / (min_rad*2));
					LAST_POLY->point[0].v = (fvi_point_dist_plane(&decal_cube_plane[2][0], &decal_cube_plane[2][1], &temp) / (min_rad*2));

					vm_vec2vert(decal_point_list[decal__poly[i]], &LAST_POLY->point[1]);
					vm_vert2vec(&LAST_POLY->point[1], &temp);
					LAST_POLY->point[1].u = (fvi_point_dist_plane(&decal_cube_plane[0][0], &decal_cube_plane[0][1], &temp) / (min_rad*2));
					LAST_POLY->point[1].v = (fvi_point_dist_plane(&decal_cube_plane[2][0], &decal_cube_plane[2][1], &temp) / (min_rad*2));
										
					vm_vec2vert(decal_point_list[decal__poly[(i+1)%nv]], &LAST_POLY->point[2]);
					vm_vert2vec(&LAST_POLY->point[2], &temp);
					LAST_POLY->point[2].u = (fvi_point_dist_plane(&decal_cube_plane[0][0], &decal_cube_plane[0][1], &temp) / (min_rad*2));
					LAST_POLY->point[2].v = (fvi_point_dist_plane(&decal_cube_plane[2][0], &decal_cube_plane[2][1], &temp) / (min_rad*2));

		//		}
				bool uv_passed = check_uv(LAST_POLY);
				if(defenately){
					LAST_POLY = set_decal->add_poly();
					mprintf(("there are now %d polys in the current decal\n", set_decal->n_poly));
				}else
				if(uv_passed && IS_VEC_NULL(&local_closest)){
					//if local_closest is null than the poly is coplaner to the poly that was hit, 
					//only if it's UVs are out of range would it not count
					LAST_POLY = set_decal->add_poly();
					mprintf(("there are now %d polys in the current decal\n", set_decal->n_poly));
				}
			}
		}
	}else{
//		mprintf(("no poly, all points are out side of planes"));
	}

//TIMERBAR_POP();	
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

poly_list dec_list;

//finds the tri count of the current decal list
int find_decal_tricounts(decal_system* system, int i){
	int dec_tri_count = 0;
	decal_list_controle* list = &system->decals[i];
	decal_list* decal = list->d_list;
	decal_list* d;
#ifndef NDEBUG
	int n = list->n_decals;
#endif
	while(decal != NULL){
		Assert(n--);
		Assert((decal->dec.n_poly < MAX_DECAL_POLY) && (decal->dec.n_poly > -1));
		dec_tri_count += decal->dec.n_poly;
		d = decal;
		decal = decal->next;
	}
	Assert(d == list->end);
	return dec_tri_count;
}

//decal *temp_decals[MAX_SHIP_DECAL_TEXTURES][MAX_SHIP_DECALS];	//the decals of the ship
/*
int find_decal_texture_index(int t, int texture[MAX_SHIP_DECAL_TEXTURES]){
	for(int i = 0; i<MAX_SHIP_DECAL_TEXTURES; i++)if(texture[i] == t)return i;
	return-1;
}
*/
void rebuild_decal_buffer(decal_system* system){
	SAFEPOINT("entering rebuild_decal_buffer");

	for(int i = 0; i<system->n_decal_textures; i++){
check_ship_decals();
check_list_decals();
check_poly_decals();
		decal_list_controle* list = &system->decals[i];
		if(!list)continue;

		if(!list->modifyed){
			continue; //if this list hasn't been modifyed don't bother
		}

		decal_list* decl = list->d_list;
		dec_list.allocate(find_decal_tricounts(system, i)*3);

		decal_list* d;
		dec_list.n_verts = 0;
		list->n_poly = 0;
		while(decl != NULL){
			decal *dec = &decl->dec;
			Assert((dec->n_poly < MAX_DECAL_POLY) && (dec->n_poly > -1));
			if(dec->n_poly == MAX_DECAL_POLY-1)
				mprintf(( "decal maxed out, decal creation probly isn't working right\n"));
			if(dec->n_poly == 0)
				mprintf(( "decal empty, decal creation failed\n"));

			decal_poly* poly = dec->poly;
			dec->vertex_buffer_start = dec_list.n_verts;
			for(int j = 0; j < dec->n_poly; j++){
				dec_list.vert[dec_list.n_verts] = poly->point[0];
				dec_list.norm[dec_list.n_verts] = poly->norm[0];
	
				dec_list.vert[dec_list.n_verts + 1] = poly->point[1];
				dec_list.norm[dec_list.n_verts + 1] = poly->norm[1];

				dec_list.vert[dec_list.n_verts + 2] = poly->point[2];
				dec_list.norm[dec_list.n_verts + 2] = poly->norm[2];

				poly = poly->next;

				dec_list.n_verts += 3;
				list->n_poly++;
			}
			d = decl;
			decl = decl->next;
		}
		Assert(d == list->end);

		Assert(list->end->dec.vertex_buffer_start + (list->end->dec.n_poly*3) <= dec_list.n_verts);

		if(list->buffer > -1)gr_destroy_buffer(list->buffer);
		if(dec_list.n_verts)list->buffer = gr_make_buffer(&dec_list, VERTEX_FLAG_POSITION | VERTEX_FLAG_NORMAL | VERTEX_FLAG_UV1);
		else {
//n			Warning(LOCATION, "no polys in the poly list!");
			list->buffer = -1;
		}

		list->modifyed = false; //becase now the vertex buffer is in sync with what the decal list has
	}

	system->decals_modified = false;
check_ship_decals();
check_list_decals();
check_poly_decals();
	
		SAFEPOINT("leaveing rebuild_decal_buffer");
}

/*
void decal_render_all(object * obj){
#ifdef DECALS_ENABLED
	if(obj->type != OBJ_SHIP){
		return;
	}

	ship *shipp = &Ships[obj->instance];
	if(shipp->decals_modified)rebuild_decal_buffer(obj);
	gr_set_texture_addressing(TMAP_ADDRESS_CLAMP);
	gr_zbias(3);
	for(int i = 0; i<shipp->n_decal_textures; i++){
		gr_set_bitmap( shipp->decals[i].texture, GR_ALPHABLEND_NONE, GR_BITBLT_MODE_NORMAL, 1.0 );
		if(shipp->decals[i].buffer != -1)gr_render_buffer(shipp->decals[i].buffer);
		
	}
	gr_zbias(0);
	gr_set_texture_addressing(TMAP_ADDRESS_WRAP);

#endif
}
*/

extern int GLOWMAP;
void decal_render_system(decal_system* system){
//return;
	int i;

	if(system->decals_modified)rebuild_decal_buffer(system);
	gr_zbias(4);
	for(i = 0; i<system->n_decal_textures; i++){
		GLOWMAP = system->decals[i].glow_texture;
		gr_set_bitmap( system->decals[i].texture, GR_ALPHABLEND_NONE, GR_BITBLT_MODE_NORMAL, 1.0 );
		if(system->decals[i].buffer != -1){
			gr_set_buffer(system->decals[i].buffer);
			gr_render_buffer(0, system->decals[i].n_poly, NULL);
		}
	}
	GLOWMAP = -1;
	SAFEPOINT("begining decal burn pass");
	if(!Cmdline_nohtl)gr_set_lighting(false,true);
	gr_zbias(5);
	for( i = 0; i<system->n_decal_textures; i++){
		decal_list *list = system->decals[i].d_list;
		int verts_used = 0;
		if(system->decals[i].buffer != -1){
			gr_set_buffer(system->decals[i].buffer);
			while(list != NULL){
				// if it's too old prune it
				if(list->dec.timestamp + list->dec.burn_time > timestamp()){
					//if it's still alive render it
					Assert(list->dec.n_poly > 0 && list->dec.vertex_buffer_start > -1);
					Assert(list->dec.vertex_buffer_start == 3*(list->dec.vertex_buffer_start/3));
					if(!list->dec.n_poly)continue;
					if(list->dec.vertex_buffer_start < 0)continue;
					float a = 1.0f - ( (float(timestamp()) - float(list->dec.timestamp)) / float(list->dec.burn_time) );
					gr_set_bitmap( system->decals[i].burn_texture, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, a );
					gr_render_buffer(verts_used, list->dec.n_poly, NULL);
				}
				verts_used += list->dec.n_poly * 3;
				list = list->next;
			}
		}
	}
	gr_zbias(0);
}

int decal_submodel_instance_depth = 0;
void set_submodel_instance(int model_num, int sub_model_num){
	Assert(!decal_submodel_instance_depth);
	polymodel *pm = model_get(model_num);

	angles orient[8];//probly only need 2 or 3, but what the hell
	vec3d offset[8];

	while ((sub_model_num >= 0) && (pm->submodel[sub_model_num].parent >= 0) ) {

		orient[decal_submodel_instance_depth] = pm->submodel[sub_model_num].angs;
		offset[decal_submodel_instance_depth] = pm->submodel[sub_model_num].offset;
		decal_submodel_instance_depth++;
		
		sub_model_num = pm->submodel[sub_model_num].parent;
		if(decal_submodel_instance_depth == 7)break;
	}
	for(int i = 0; i < decal_submodel_instance_depth; i++){
		g3_start_instance_angles(&offset[i], &orient[i]);
	}
}

void end_submodel_instance(){
	while(decal_submodel_instance_depth > 0){
		decal_submodel_instance_depth--;
		g3_done_instance(true);
	}
	Assert(!decal_submodel_instance_depth);
}

void decal_render_all(object * obj){
	if(!Cmdline_decals)return;
//	return;
#ifdef DECALS_ENABLED
//	TIMERBAR_PUSH(TIMERBAR_DARKGREEN);
/*check_ship_decals();
check_list_decals();
check_poly_decals();*/
	SAFEPOINT("entering decal render");
	if(obj->type != OBJ_SHIP){
		return;
	}
	gr_set_texture_addressing(TMAP_ADDRESS_CLAMP);
	gr_zbuffer_set(GR_ZBUFF_READ);

	ship *shipp = &Ships[obj->instance];
	decal_system* system = &shipp->ship_decal_system;

	decal_render_system(system);

	ship_subsys *sys = GET_FIRST(&shipp->subsys_list);;

//	ship_model_start(obj); //were allready in

//		TIMERBAR_PUSH(TIMERBAR_LIGHTGREY);
	while(sys != END_OF_LIST(&shipp->subsys_list)){
		model_subsystem* psub = sys->system_info;
		if(!psub){
			sys = GET_NEXT(sys);
			continue;
		}
		set_submodel_instance(psub->model_num, psub->subobj_num);
		decal_render_system(&psub->model_decal_system);
		end_submodel_instance();

		sys = GET_NEXT(sys);
	}
//		TIMERBAR_POP();

//	ship_model_stop(obj);

	gr_set_texture_addressing(TMAP_ADDRESS_WRAP);

	SAFEPOINT("leaveing decal render");
//	TIMERBAR_POP();
/*check_ship_decals();
check_list_decals();
check_poly_decals();
*/
#endif
}

