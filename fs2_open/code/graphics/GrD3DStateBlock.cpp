
#ifndef NO_DIRECT3D

#include "grd3dsetup.h"
#include <d3d8.h>
#include "graphics/grd3d.h"
#include "graphics/grd3dinternal.h"


static DWORD *state_block = NULL;
static int n_state_blocks = 0;

//hFFFFFFFF
//invalid

static int get_new_state_block(){
	for(int i = 0; i <n_state_blocks; i++)
		if(state_block[i] == 0xFFFFFFFF)return i;

	DWORD *old_states = state_block;
	state_block = (DWORD*)vm_malloc(sizeof(DWORD)*n_state_blocks+1);
	memcpy(state_block, old_states, n_state_blocks);
	if(old_states)vm_free(old_states);

	GlobalD3DVars::lpD3DDevice->CreateStateBlock(D3DSBT_PIXELSTATE, &state_block[n_state_blocks++]);

	return n_state_blocks-1;
}

//start recording a state block, no rendering should occur after this call untill the recording is ended
void gr_d3d_start_state_block(){
	gr_screen.recording_state_block = true;
	GlobalD3DVars::lpD3DDevice->BeginStateBlock();
}

//stops recording of a state block, returns a handle to be used by set_render_state
int gr_d3d_end_state_block(){

	gr_screen.recording_state_block = false;
	int state = get_new_state_block();

	if(FAILED(GlobalD3DVars::lpD3DDevice->EndStateBlock(&state_block[get_new_state_block()]))){
		return -1;//standard error return
	}else{
		return state;
	}

}

//takes a handle to a state_block
void gr_d3d_set_state_block(int handle){
	if(handle == -1)return;
	GlobalD3DVars::lpD3DDevice->ApplyStateBlock(handle);
}

#endif // !NO_DIRECT3D
