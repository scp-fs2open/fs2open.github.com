/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Mission/MissionTraining.h $
 * $Revision: 2.2 $
 * $Date: 2005-05-12 03:50:10 $
 * $Author: Goober5000 $
 *
 * Special code for training missions.  Stuff like displaying training messages in
 * the special training window, listing the training objectives, etc.
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.1  2004/08/11 05:06:28  Kazan
 * added preprocdefines.h to prevent what happened with fred -- make sure to make all fred2 headers include this file as the _first_ include -- i have already modified fs2 files to do this
 *
 * Revision 2.0  2002/06/03 04:02:25  penguin
 * Warpcore CVS sync
 *
 * Revision 1.1  2002/05/02 18:03:10  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:49a Dave
 * 
 * 7     4/16/98 4:33p Hoffoss
 * Added support for detecting instructor terminating training due to
 * player shooting at him.
 * 
 * 6     4/15/98 5:25p Lawrance
 * extern Training_message_visible
 * 
 * 5     1/05/98 4:04p Hoffoss
 * Changed training-message sexp operator to allow it to control the length of
 * time a message is displayed for.
 * 
 * 4     10/17/97 6:39p Hoffoss
 * Added delayability to key-pressed operator and training-message operator.
 * 
 * 3     10/10/97 6:15p Hoffoss
 * Implemented a training objective list display.
 * 
 * 2     10/09/97 4:44p Hoffoss
 * Dimmed training window glass and made it less transparent, added flags
 * to events, set he stage for detecting current events.
 * 
 * 1     10/09/97 2:41p Hoffoss
 *
 * $NoKeywords: $
 */

#include "PreProcDefines.h"
extern int Training_message_method;
extern int Training_num_lines;
extern int Training_message_visible;
extern int Training_failure;

void training_mission_init();
void training_mission_shutdown();
void training_check_objectives();
void message_training_queue(char *text, int timestamp, int length = -1);
void message_training_setup(int num, int length = -1);
void message_training_display();
void message_translate_tokens(char *buf, char *text);
void training_fail();
