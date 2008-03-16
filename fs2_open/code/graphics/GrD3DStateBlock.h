
#ifndef _GRD3DSTATEBLOCK_H
#define _GRD3DSTATEBLOCK_H

#ifndef NO_DIRECT3D

/*
state blocks are expected to be made early on, or dureing a loading period
then live for the duration of the program
*/

void gr_d3d_start_state_block();
int gr_d3d_end_state_block();
void gr_d3d_set_state_block(int);

#endif // !NO_DIRECT3D

#endif // _GRD3DSTATEBLOCK_H
