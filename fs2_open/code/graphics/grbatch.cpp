/*
 * Code created by Thomas Whittaker (RT) for a Freespace 2 source code project
 *
 * You may not sell or otherwise commercially exploit the source or things you 
 * created based on the source.
 *
*/ 

#include "globalincs/pstypes.h"
#include "graphics/grbatch.h"
#include "graphics/2d.h"
#include "cmdline/cmdline.h"

typedef struct {
	int start, length;
	int flags;
	int bitmap;
} BatchNode;

const int BATCH_MAX_VERTEX = 500;
const int BATCH_MAX = 50;

vertex *Batch_vertex_array	= NULL;
BatchNode *Batch_array		= NULL;
bool Batch_in_process = false;
int  Batch_current = 0;
int  Batch_vertex_current = 0;

bool batch_init()
{
	if(!Cmdline_batch_3dunlit) return true;

	Batch_vertex_array	= (vertex *)	malloc(sizeof(vertex) * BATCH_MAX_VERTEX);
	Batch_array			= (BatchNode *) malloc(sizeof(vertex) * BATCH_MAX);

	return (Batch_array != 0); 
}

void batch_deinit()
{
	if(!Cmdline_batch_3dunlit) return;

	if(Batch_array)
		free(Batch_array);
}

void batch_start()
{
	Batch_in_process = true;
	Batch_current = 0;
	Batch_vertex_current = 0;
}

void batch_end()
{
	Batch_in_process = false;
}

void batch_render()
{
	if(!Cmdline_batch_3dunlit) return;
	if(Batch_current == 0) return;

	// Sort
	// Batch up again
	// Send to renderer
	for(int i = 0; i < Batch_current; i++)
	{
		vertex *vlist = &Batch_vertex_array[Batch_array[i].start];

		gr_set_bitmap(Batch_array[i].bitmap);

		gr_tmapper_batch_3d_unlit(Batch_array[i].length, vlist, Batch_array[i].flags);  
	}

	Batch_current = 0;
	Batch_vertex_current = 0;		  
}

vertex *batch_get_block(int num_verts, int flags)
{
	if(!Cmdline_batch_3dunlit) return NULL;

	if(Batch_vertex_array == NULL || Batch_array == NULL) {
		Assert(0);
		return NULL;
	}

	//int vcurrent = 0;

	// We've run out of vertex slots!
	if(num_verts >= (BATCH_MAX_VERTEX - Batch_vertex_current))
	{
		batch_render();
	}

	// Not the first batch
	if(Batch_current > 0)
	{
		int last_batch = Batch_current - 1;

		// Can add to current batch
		if(	Batch_array[last_batch].flags  == flags && 
			Batch_array[last_batch].bitmap == gr_screen.current_bitmap)
		{
			int pos = Batch_array[last_batch].start + Batch_array[last_batch].length;
			vertex *block = &Batch_vertex_array[pos];
			Batch_array[last_batch].length += num_verts;
			Batch_vertex_current += num_verts;
			return block; 
		}
	}	

	// We've run out of batch slots!
	if(Batch_current >= BATCH_MAX)
	{
		batch_render();
	}

	// Make a new batch
	Batch_array[Batch_current].flags  = flags;
	Batch_array[Batch_current].start  = Batch_vertex_current;
	Batch_array[Batch_current].length = num_verts;
	Batch_array[Batch_current].bitmap = gr_screen.current_bitmap;
	vertex *block = &Batch_vertex_array[Batch_array[Batch_current].start];
	Batch_current++;

	Batch_vertex_current += num_verts;
	return block;
}