/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Graphics/Line.h $
 * $Revision: 2.3 $
 * $Date: 2005-07-13 03:15:51 $
 * $Author: Goober5000 $
 *
 * Header file for line.cpp
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.2  2004/08/11 05:06:24  Kazan
 * added preprocdefines.h to prevent what happened with fred -- make sure to make all fred2 headers include this file as the _first_ include -- i have already modified fs2 files to do this
 *
 * Revision 2.1  2004/03/08 15:03:27  Kazan
 * *crosses fingers*
 *
 * Revision 2.0  2002/06/03 04:02:23  penguin
 * Warpcore CVS sync
 *
 * Revision 1.1  2002/05/02 18:03:07  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:49a Dave
 * 
 * 10    5/06/98 5:30p John
 * Removed unused cfilearchiver.  Removed/replaced some unused/little used
 * graphics functions, namely gradient_h and _v and pixel_sp.   Put in new
 * DirectX header files and libs that fixed the Direct3D alpha blending
 * problems.
 * 
 * 9     3/10/98 4:18p John
 * Cleaned up graphics lib.  Took out most unused gr functions.   Made D3D
 * & Glide have popups and print screen.  Took out all >8bpp software
 * support.  Made Fred zbuffer.  Made zbuffer allocate dynamically to
 * support Fred.  Made zbuffering key off of functions rather than one
 * global variable.
 * 
 * 8     10/03/97 9:10a John
 * added better antialiased line drawer
 * 
 * 7     6/13/97 5:35p John
 * added some antialiased bitmaps and lines
 * 
 * 6     11/26/96 6:50p John
 * Added some more hicolor primitives.  Made windowed mode run as current
 * bpp, if bpp is 8,16,or 32.
 * 
 * 5     10/26/96 2:56p John
 * Got gradient code working.
 * 
 * 4     10/26/96 1:40p John
 * Added some now primitives to the 2d library and
 * cleaned up some old ones.
 *
 * $NoKeywords: $
 */

#ifndef _LINE_H
#define _LINE_H

void gr8_line(int x1,int y1,int x2,int y2, bool b);
void gr8_aaline(vertex *v1, vertex *v2);
											

#define INT_EXCHG(a,b) do {                                              \
    int __temp__ = (a);                                                 \
    (a) = (b);                                                          \
    (b) = __temp__;                                                     \
} while(0)

//#define INT_SCALE(var,arg,num,den) ((var) = ((arg) * (num)) / (den))
#define INT_SCALE(var,arg,num,den) ((var) = MulDiv(arg, num, den))

#define INT_CLIPLINE(x1,y1,x2,y2,XMIN,YMIN,XMAX,YMAX,WHEN_OUTSIDE,WHEN_CLIPPED,WHEN_SWAPPED) do {                                    \
    int temp;                                                  \
                                                                        \
    if(y1 > y2)                                                         \
        { INT_EXCHG(y1,y2); INT_EXCHG(x1,x2); WHEN_SWAPPED; }                                 \
    if((y2 < YMIN) || (y1 > YMAX))                    \
        { WHEN_OUTSIDE; }                                               \
    if(x1 < x2) {                                                       \
        if((x2 < XMIN) || (x1 > XMAX)) {              \
            WHEN_OUTSIDE;                                               \
        }                                                               \
        if(x1 < XMIN) {                                        \
			INT_SCALE(temp,(y2 - y1),(XMIN - x1),(x2 - x1));      \
            if((y1 += temp) > YMAX) { WHEN_OUTSIDE; }          \
            x1 = XMIN;                                         \
            WHEN_CLIPPED;                                               \
        }                                                               \
        if(x2 > XMAX) {                                        \
			INT_SCALE(temp,(y2 - y1),(x2 - XMAX),(x2 - x1));      \
            if((y2 -= temp) < YMIN) { WHEN_OUTSIDE; }          \
            x2 = XMAX;                                         \
            WHEN_CLIPPED;                                               \
        }                                                               \
        if(y1 < YMIN) {                                        \
			INT_SCALE(temp,(x2 - x1),(YMIN - y1),(y2 - y1));      \
            x1 += temp;                                                 \
            y1 = YMIN;                                         \
            WHEN_CLIPPED;                                               \
        }                                                               \
        if(y2 > YMAX) {                                        \
			INT_SCALE(temp,(x2 - x1),(y2 - YMAX),(y2 - y1));      \
            x2 -= temp;                                                 \
            y2 = YMAX;                                         \
            WHEN_CLIPPED;                                               \
        }                                                               \
    }                                                                   \
    else {                                                              \
        if((x1 < XMIN) || (x2 > XMAX)) {              \
            WHEN_OUTSIDE;                                               \
        }                                                               \
        if(x1 > XMAX) {                                        \
			INT_SCALE(temp,(y2 - y1),(x1 - XMAX),(x1 - x2));      \
            if((y1 += temp) > YMAX) { WHEN_OUTSIDE; }          \
            x1 = XMAX;                                         \
            WHEN_CLIPPED;                                               \
        }                                                               \
        if(x2 < XMIN) {                                        \
			INT_SCALE(temp,(y2 - y1),(XMIN - x2),(x1 - x2));      \
            if((y2 -= temp) < YMIN) { WHEN_OUTSIDE; }          \
            x2 = XMIN;                                         \
            WHEN_CLIPPED;                                               \
        }                                                               \
        if(y1 < YMIN) {                                        \
			INT_SCALE(temp,(x1 - x2),(YMIN - y1),(y2 - y1));      \
            x1 -= temp;                                                 \
            y1 = YMIN;                                         \
            WHEN_CLIPPED;                                               \
        }                                                               \
        if(y2 > YMAX) {                                        \
			INT_SCALE(temp,(x1 - x2),(y2 - YMAX),(y2 - y1));      \
            x2 += temp;                                                 \
            y2 = YMAX;                                         \
            WHEN_CLIPPED;                                               \
        }                                                               \
    }                                                                   \
} while(0)

#define FL_EXCHG(a,b) do {                                                 \
    float __temp__ = (a);                                                 \
    (a) = (b);                                                          \
    (b) = __temp__;                                                     \
} while(0)

#define FL_SCALE(var,arg,num,den) ((var) = ((arg) * (num)) / (den))

#define FL_CLIPLINE(x1,y1,x2,y2,XMIN,YMIN,XMAX,YMAX,WHEN_OUTSIDE,WHEN_CLIPPED,WHEN_SWAPPED) do {                                    \
    float temp;                                                  \
                                                                        \
    if(y1 > y2)                                                         \
        { FL_EXCHG(y1,y2); FL_EXCHG(x1,x2); WHEN_SWAPPED; }                                 \
    if((y2 < YMIN) || (y1 > YMAX))                    \
        { WHEN_OUTSIDE; }                                               \
    if(x1 < x2) {                                                       \
        if((x2 < XMIN) || (x1 > XMAX)) {              \
            WHEN_OUTSIDE;                                               \
        }                                                               \
        if(x1 < XMIN) {                                        \
			FL_SCALE(temp,(y2 - y1),(XMIN - x1),(x2 - x1));      \
            if((y1 += temp) > YMAX) { WHEN_OUTSIDE; }          \
            x1 = XMIN;                                         \
            WHEN_CLIPPED;                                               \
        }                                                               \
        if(x2 > XMAX) {                                        \
			FL_SCALE(temp,(y2 - y1),(x2 - XMAX),(x2 - x1));      \
            if((y2 -= temp) < YMIN) { WHEN_OUTSIDE; }          \
            x2 = XMAX;                                         \
            WHEN_CLIPPED;                                               \
        }                                                               \
        if(y1 < YMIN) {                                        \
			FL_SCALE(temp,(x2 - x1),(YMIN - y1),(y2 - y1));      \
            x1 += temp;                                                 \
            y1 = YMIN;                                         \
            WHEN_CLIPPED;                                               \
        }                                                               \
        if(y2 > YMAX) {                                        \
			FL_SCALE(temp,(x2 - x1),(y2 - YMAX),(y2 - y1));      \
            x2 -= temp;                                                 \
            y2 = YMAX;                                         \
            WHEN_CLIPPED;                                               \
        }                                                               \
    }                                                                   \
    else {                                                              \
        if((x1 < XMIN) || (x2 > XMAX)) {              \
            WHEN_OUTSIDE;                                               \
        }                                                               \
        if(x1 > XMAX) {                                        \
			FL_SCALE(temp,(y2 - y1),(x1 - XMAX),(x1 - x2));      \
            if((y1 += temp) > YMAX) { WHEN_OUTSIDE; }          \
            x1 = XMAX;                                         \
            WHEN_CLIPPED;                                               \
        }                                                               \
        if(x2 < XMIN) {                                        \
			FL_SCALE(temp,(y2 - y1),(XMIN - x2),(x1 - x2));      \
            if((y2 -= temp) < YMIN) { WHEN_OUTSIDE; }          \
            x2 = XMIN;                                         \
            WHEN_CLIPPED;                                               \
        }                                                               \
        if(y1 < YMIN) {                                        \
			FL_SCALE(temp,(x1 - x2),(YMIN - y1),(y2 - y1));      \
            x1 -= temp;                                                 \
            y1 = YMIN;                                         \
            WHEN_CLIPPED;                                               \
        }                                                               \
        if(y2 > YMAX) {                                        \
			FL_SCALE(temp,(x1 - x2),(y2 - YMAX),(y2 - y1));      \
            x2 += temp;                                                 \
            y2 = YMAX;                                         \
            WHEN_CLIPPED;                                               \
        }                                                               \
    }                                                                   \
} while(0)

#endif

