/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#ifndef __CREDITS_H__
#define __CREDITS_H__

void credits_init();
void credits_do_frame(float frametime);
void credits_close();

void credits_stop_music(bool fade);

#endif /* __CREDITS_H__ */
