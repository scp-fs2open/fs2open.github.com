/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
 */ 

/*
 * $Logfile$
 * $Revision: 1.3 $
 * $Date: 2005-07-13 03:25:58 $
 * $Author: Goober5000 $
 *
 * Header file for PXO-substitute (FS2OX -- "fs2_open exchange") screen
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.2  2004/08/11 05:06:29  Kazan
 * added preprocdefines.h to prevent what happened with fred -- make sure to make all fred2 headers include this file as the _first_ include -- i have already modified fs2 files to do this
 *
 * Revision 1.1  2002/07/29 22:24:26  penguin
 * First attempt at "fs2_open exchange" (PXO substitute) screen
 *
 * $NoKeywords: $
 */

#ifndef FS2OX_H
#define FS2OX_H

void fs2ox_close();
void fs2ox_init();
void fs2ox_do_frame();

#endif
