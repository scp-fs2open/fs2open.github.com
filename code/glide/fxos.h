/*
** Copyright (c) 1995, 3Dfx Interactive, Inc.
** All Rights Reserved.
**
** $ Revision: $
** $ Date: $
**
*/


#ifndef _FXOS_H_
#define _FXOS_H_

#include <stdio.h>

#   ifdef __cplusplus
extern "C" {
#   endif

#   ifdef WIN32
void sleep(int secs);
#define gethostname fxGethostname

int gethostname(char *name, int namelen);

#   endif

float fxTime(void);
float timer(int flag);

FILE *fxFopenPath(const char *filename, const char *mode,
					const char *path, const char **pprefix);

#   ifdef __cplusplus
}
#   endif

#endif
