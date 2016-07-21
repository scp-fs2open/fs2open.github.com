/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#include "Management.h"

int	create_wing();
void	remove_wing(int wing_num);
void	remove_ship_from_wing(int ship, int min = 1);
void	mark_wing(int wing);
int	delete_wing(int wing = cur_wing, int bypass = 0);
