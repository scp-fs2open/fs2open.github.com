/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#ifndef _LINE_H
#define _LINE_H

//void gr8_line(int x1,int y1,int x2,int y2, bool b);
//void gr8_aaline(vertex *v1, vertex *v2);


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
