

#include "globalincs/pstypes.h"


extern int g_width, g_height;
extern void *g_vBackBuf1, *g_vBackBuf2;

static void dispatchDecoder8(ubyte **pFrame, ubyte codeType, ubyte **pData, int *pDataRemain, int *curXb, int *curYb);

void decodeFrame8(ubyte *pFrame, ubyte *pMap, int mapRemain, ubyte *pData, int dataRemain)
{
	int i, j;
	int xb, yb;

	xb = g_width >> 3;
	yb = g_height >> 3;

	for (j = 0; j < yb; j++) {
		for (i = 0; i < (xb / 2); i++) {
			dispatchDecoder8(&pFrame, (*pMap) & 0xf, &pData, &dataRemain, &i, &j);

			dispatchDecoder8(&pFrame, (*pMap) >> 4, &pData, &dataRemain, &i, &j);

			pMap++;
			mapRemain--;
		}

		pFrame += 7*g_width;
	}
}

static void relClose(int i, int *x, int *y)
{
	int ma, mi;

	ma = i >> 4;
	mi = i & 0xf;

	*x = mi - 8;
	*y = ma - 8;
}

static void relFar(int i, int sign, int *x, int *y)
{
	if (i < 56) {
		*x = sign * (8 + (i % 7));
		*y = sign *      (i / 7);
	} else {
		*x = sign * (-14 + (i - 56) % 29);
		*y = sign *   (8 + (i - 56) / 29);
	}
}

/* copies an 8x8 block from pSrc to pDest.
   pDest and pSrc are both g_width bytes wide */
static void copyFrame(ubyte *pDest, ubyte *pSrc)
{
	int i;

	for (i = 0; i < 8; i++) {
		memcpy(pDest, pSrc, 8);
		pDest += g_width;
		pSrc += g_width;
	}
}

// Fill in the next eight bytes with p[0], p[1], p[2], or p[3],
// depending on the corresponding two-bit value in pat0 and pat1
static void patternRow4Pixels(ubyte *pFrame, ubyte pat0, ubyte pat1, ubyte *p)
{
	ushort mask = 0x0003;
	ushort shift = 0;
	ushort pattern = (pat1 << 8) | pat0;

	while (mask != 0) {
		*pFrame++ = p[(mask & pattern) >> shift];
		mask <<= 2;
		shift += 2;
	}
}

// Fill in the next four 2x2 pixel blocks with p[0], p[1], p[2], or p[3],
// depending on the corresponding two-bit value in pat0.
static void patternRow4Pixels2(ubyte *pFrame, ubyte pat0, ubyte *p)
{
	ubyte mask = 0x03;
	ubyte shift = 0;
	ubyte pel;

	while (mask != 0) {
		pel = p[(mask & pat0) >> shift];
		pFrame[0] = pel;
		pFrame[1] = pel;
		pFrame[g_width + 0] = pel;
		pFrame[g_width + 1] = pel;
		pFrame += 2;
		mask <<= 2;
		shift += 2;
	}
}

// Fill in the next four 2x1 pixel blocks with p[0], p[1], p[2], or p[3],
// depending on the corresponding two-bit value in pat.
static void patternRow4Pixels2x1(ubyte *pFrame, ubyte pat, ubyte *p)
{
	ubyte mask = 0x03;
	ubyte shift = 0;
	ubyte pel;

	while (mask != 0) {
		pel = p[(mask & pat) >> shift];
		pFrame[0] = pel;
		pFrame[1] = pel;
		pFrame += 2;
		mask <<= 2;
		shift += 2;
	}
}

// Fill in the next 4x4 pixel block with p[0], p[1], p[2], or p[3],
// depending on the corresponding two-bit value in pat0, pat1, pat2, and pat3.
static void patternQuadrant4Pixels(ubyte *pFrame, ubyte pat0, ubyte pat1, ubyte pat2, ubyte pat3, ubyte *p)
{
	unsigned long mask = 0x00000003UL;
	int shift=0;
	int i;
	unsigned long pat = (pat3 << 24) | (pat2 << 16) | (pat1 << 8) | pat0;

	for (i = 0; i < 16; i++) {
		pFrame[i&3] = p[(pat & mask) >> shift];

		if ( (i & 3) == 3 )
			pFrame += g_width;

		mask <<= 2;
		shift += 2;
	}
}

// fills the next 8 pixels with either p[0] or p[1], depending on pattern
static void patternRow2Pixels(ubyte *pFrame, ubyte pat, ubyte *p)
{
	ubyte mask=0x01;

	while (mask != 0) {
		*pFrame++ = p[(mask & pat) ? 1 : 0];
		mask <<= 1;
	}
}

// fills the next four 2 x 2 pixel boxes with either p[0] or p[1], depending on pattern
static void patternRow2Pixels2(ubyte *pFrame, ubyte pat, ubyte *p)
{
	ubyte pel;
	ubyte mask=0x1;

	while (mask != 0x10) {
		pel = p[(mask & pat) ? 1 : 0];

		pFrame[0] = pel;              // upper-left
		pFrame[1] = pel;              // upper-right
		pFrame[g_width + 0] = pel;    // lower-left
		pFrame[g_width + 1] = pel;    // lower-right
		pFrame += 2;

		mask <<= 1;
	}
}

// fills pixels in the next 4 x 4 pixel boxes with either p[0] or p[1], depending on pat0 and pat1.
static void patternQuadrant2Pixels(ubyte *pFrame, ubyte pat0, ubyte pat1, ubyte *p)
{
	ubyte pel;
	ushort mask = 0x0001;
	int i, j;
	ushort pat = (pat1 << 8) | pat0;

	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++) {
			pel = p[(pat & mask) ? 1 : 0];

			pFrame[j + i * g_width] = pel;

			mask <<= 1;
		}
	}
}

static void dispatchDecoder8(ubyte **pFrame, ubyte codeType, ubyte **pData, int *pDataRemain, int *curXb, int *curYb)
{
	ubyte p[4];
	ubyte pat[16];
	int i, j, k;
	int x, y;

	switch(codeType) {
		case 0x0:
			copyFrame(*pFrame, *pFrame + ((ubyte *)g_vBackBuf2 - (ubyte *)g_vBackBuf1));
			*pFrame += 8;
			break;

		case 0x1:
			*pFrame += 8;
			break;

		case 0x2:
			relFar(*(*pData)++, 1, &x, &y);
			copyFrame(*pFrame, *pFrame + x + y*g_width);
			*pFrame += 8;
			(*pDataRemain)--;
			break;

		case 0x3:
			relFar(*(*pData)++, -1, &x, &y);
			copyFrame(*pFrame, *pFrame + x + y*g_width);
			*pFrame += 8;
			(*pDataRemain)--;
			break;

		case 0x4:
			relClose(*(*pData)++, &x, &y);
			copyFrame(*pFrame, *pFrame + ((ubyte *)g_vBackBuf2 - (ubyte *)g_vBackBuf1) + x + y*g_width);
			*pFrame += 8;
			(*pDataRemain)--;
			break;

		case 0x5:
			x = (signed char)*(*pData)++;
			y = (signed char)*(*pData)++;
			copyFrame(*pFrame, *pFrame + ((ubyte *)g_vBackBuf2 - (ubyte *)g_vBackBuf1) + x + y*g_width);
			*pFrame += 8;
			*pDataRemain -= 2;
			break;

		case 0x6:
			for (i = 0; i < 2; i++) {
				*pFrame += 16;

				if (++*curXb == (g_width >> 3)) {
					*pFrame += 7*g_width;
					*curXb = 0;

					if (++*curYb == (g_height >> 3))
						return;
				}
			}
			break;

		case 0x7:
			p[0] = *(*pData)++;
			p[1] = *(*pData)++;

			if (p[0] <= p[1]) {
				for (i = 0; i < 8; i++) {
					patternRow2Pixels(*pFrame, *(*pData)++, p);
					*pFrame += g_width;
				}
			} else {
				for (i = 0; i < 2; i++) {
					patternRow2Pixels2(*pFrame, *(*pData) & 0xf, p);
					*pFrame += 2*g_width;

					patternRow2Pixels2(*pFrame, *(*pData)++ >> 4, p);
					*pFrame += 2*g_width;
				}
			}

			*pFrame -= (8*g_width - 8);
			break;

		case 0x8:
			if ( (*pData)[0] <= (*pData)[1] ) {
				// four quadrant case
				for (i = 0; i < 4; i++) {
					p[0] = *(*pData)++;
					p[1] = *(*pData)++;
					pat[0] = *(*pData)++;
					pat[1] = *(*pData)++;
					patternQuadrant2Pixels(*pFrame, pat[0], pat[1], p);

					// alternate between moving down and moving up and right
					if (i & 1)
						*pFrame += 4 - 4*g_width; // up and right
					else
						*pFrame += 4*g_width;     // down
				}
			} else if ( (*pData)[6] <= (*pData)[7] ) {
				// split horizontal
				for (i = 0; i < 4; i++) {
					if ((i & 1) == 0) {
						p[0] = *(*pData)++;
						p[1] = *(*pData)++;
					}

					pat[0] = *(*pData)++;
					pat[1] = *(*pData)++;
					patternQuadrant2Pixels(*pFrame, pat[0], pat[1], p);

					if (i & 1)
						*pFrame -= (4*g_width - 4);
					else
						*pFrame += 4*g_width;
				}
			} else {
				// split vertical
				for (i = 0; i < 8; i++) {
					if ((i & 3) == 0) {
						p[0] = *(*pData)++;
						p[1] = *(*pData)++;
					}

					patternRow2Pixels(*pFrame, *(*pData)++, p);
					*pFrame += g_width;
				}

				*pFrame -= (8*g_width - 8);
			}
			break;

		case 0x9:
			if ( (*pData)[0] <= (*pData)[1] ) {
				if ( (*pData)[2] <= (*pData)[3] ) {
					p[0] = *(*pData)++;
					p[1] = *(*pData)++;
					p[2] = *(*pData)++;
					p[3] = *(*pData)++;

					for (i = 0; i < 8; i++) {
						pat[0] = *(*pData)++;
						pat[1] = *(*pData)++;

						patternRow4Pixels(*pFrame, pat[0], pat[1], p);
						*pFrame += g_width;
					}

					*pFrame -= (8*g_width - 8);
				} else {
					p[0] = *(*pData)++;
					p[1] = *(*pData)++;
					p[2] = *(*pData)++;
					p[3] = *(*pData)++;

					patternRow4Pixels2(*pFrame, *(*pData)++, p);
					*pFrame += 2*g_width;

					patternRow4Pixels2(*pFrame, *(*pData)++, p);
					*pFrame += 2*g_width;

					patternRow4Pixels2(*pFrame, *(*pData)++, p);
					*pFrame += 2*g_width;

					patternRow4Pixels2(*pFrame, *(*pData)++, p);
					*pFrame -= (6*g_width - 8);
				}
			} else {
				if ( (*pData)[2] <= (*pData)[3] ) {
					// draw 2x1 strips
					p[0] = *(*pData)++;
					p[1] = *(*pData)++;
					p[2] = *(*pData)++;
					p[3] = *(*pData)++;

					for (i = 0; i < 8; i++) {
						pat[0] = *(*pData)++;

						patternRow4Pixels2x1(*pFrame, pat[0], p);
						*pFrame += g_width;
					}

					*pFrame -= (8*g_width - 8);
				} else {
					// draw 1x2 strips
					p[0] = *(*pData)++;
					p[1] = *(*pData)++;
					p[2] = *(*pData)++;
					p[3] = *(*pData)++;

					for (i = 0; i < 4; i++) {
						pat[0] = *(*pData)++;
						pat[1] = *(*pData)++;

						patternRow4Pixels(*pFrame, pat[0], pat[1], p);
						*pFrame += g_width;

						patternRow4Pixels(*pFrame, pat[0], pat[1], p);
						*pFrame += g_width;
					}

					*pFrame -= (8*g_width - 8);
				}
			}
			break;

		case 0xa:
			if ( (*pData)[0] <= (*pData)[1] ) {
				for (i = 0; i < 4; i++) {
					p[0] = *(*pData)++;
					p[1] = *(*pData)++;
					p[2] = *(*pData)++;
					p[3] = *(*pData)++;
					pat[0] = *(*pData)++;
					pat[1] = *(*pData)++;
					pat[2] = *(*pData)++;
					pat[3] = *(*pData)++;

					patternQuadrant4Pixels(*pFrame, pat[0], pat[1], pat[2], pat[3], p);

					if (i & 1)
						*pFrame -= (4*g_width - 4);
					else
						*pFrame += 4*g_width;
				}
			} else {
				if ( (*pData)[12] <= (*pData)[13] ) {
					// split vertical
					for (i = 0; i < 4; i++) {
						if ((i&1) == 0) {
							p[0] = *(*pData)++;
							p[1] = *(*pData)++;
							p[2] = *(*pData)++;
							p[3] = *(*pData)++;
						}

						pat[0] = *(*pData)++;
						pat[1] = *(*pData)++;
						pat[2] = *(*pData)++;
						pat[3] = *(*pData)++;

						patternQuadrant4Pixels(*pFrame, pat[0], pat[1], pat[2], pat[3], p);

						if (i & 1)
							*pFrame -= (4*g_width - 4);
						else
							*pFrame += 4*g_width;
					}
				} else {
					// split horizontal
					for (i = 0; i < 8; i++) {
						if ((i&3) == 0) {
							p[0] = *(*pData)++;
							p[1] = *(*pData)++;
							p[2] = *(*pData)++;
							p[3] = *(*pData)++;
						}

						pat[0] = *(*pData)++;
						pat[1] = *(*pData)++;

						patternRow4Pixels(*pFrame, pat[0], pat[1], p);
						*pFrame += g_width;
					}

					*pFrame -= (8*g_width - 8);
				}
			}
			break;

		case 0xb:
			for (i = 0; i < 8; i++) {
				memcpy(*pFrame, *pData, 8);
				*pFrame += g_width;
				*pData += 8;
				*pDataRemain -= 8;
			}

			*pFrame -= (8*g_width - 8);
			break;

		case 0xc:
			for (i = 0; i < 4; i++) {
				for (j = 0; j < 2; j++) {
					for (k = 0; k < 4; k++) {
						(*pFrame)[2*k]   = (*pData)[k];
						(*pFrame)[2*k+1] = (*pData)[k];
					}

					*pFrame += g_width;
				}

				*pData += 4;
				*pDataRemain -= 4;
			}

			*pFrame -= (8*g_width - 8);
			break;

		case 0xd:
			for (i = 0; i < 2; i++) {
				for (j = 0; j < 4; j++) {
					for (k = 0; k < 4; k++) {
						(*pFrame)[k*g_width+j] = (*pData)[0];
						(*pFrame)[k*g_width+j+4] = (*pData)[1];
					}
				}

				*pFrame += 4*g_width;
				*pData += 2;
				*pDataRemain -= 2;
			}

			*pFrame -= (8*g_width - 8);
			break;

		case 0xe:
			for (i = 0; i < 8; i++) {
				memset(*pFrame, **pData, 8);
				*pFrame += g_width;
			}

			(*pData)++;
			(*pDataRemain)--;
			*pFrame -= (8*g_width - 8);
			break;

		case 0xf:
			for (i = 0; i < 8; i++) {
				for (j = 0; j < 8; j++) {
					(*pFrame)[j] = (*pData)[(i+j)&1];
				}

				*pFrame += g_width;
			}

			*pData += 2;
			*pDataRemain -= 2;
			*pFrame -= (8*g_width - 8);
			break;

		default:
			break;
	}
}
