

#include "globalincs/pstypes.h"

extern int g_width, g_height;
extern void *g_vBackBuf1, *g_vBackBuf2;

/* 16 bit decoding routines */

static ushort *backBuf1, *backBuf2;
static int lookup_initialized;

static void dispatchDecoder16(ushort **pFrame, ubyte codeType, ubyte **pData, ubyte **pOffData, int *pDataRemain, int *curXb, int *curYb);
static void genLoopkupTable();

void decodeFrame16(ubyte *pFrame, ubyte *pMap, int mapRemain, unsigned char *pData, int dataRemain)
{
	ubyte *pOrig, *pOffData, *pEnd, op;
	ushort offset;
	int length = 0, i, j, xb, yb;

	if (!lookup_initialized) {
		genLoopkupTable();
	}

	backBuf1 = (ushort *)g_vBackBuf1;
	backBuf2 = (ushort *)g_vBackBuf2;

	xb = g_width >> 3;
	yb = g_height >> 3;

	offset = pData[0]|(pData[1]<<8);

	pOffData = pData + offset;
	pEnd = pData + offset;

	pData += 2;

	pOrig = pData;
	length = offset - 2; /*dataRemain-2;*/

	for (j=0; j<yb; j++) {
		for (i=0; i<xb/2; i++) {
			op = (*pMap) & 0xf;
			dispatchDecoder16((ushort **)&pFrame, op, &pData, &pOffData, &dataRemain, &i, &j);

			op = ((*pMap) >> 4) & 0xf;
			dispatchDecoder16((ushort **)&pFrame, op, &pData, &pOffData, &dataRemain, &i, &j);

			pMap++;
			mapRemain--;
		}

		pFrame += 7*g_width*2;
	}

	if ((length-(pData-pOrig)) != 0) {
		nprintf(("MVE", "DEBUG: junk left over: %d,%d,%d\n", (pData-pOrig), length, (length-(pData-pOrig))));
	}
}

static ushort GETPIXEL(unsigned char **buf, int off)
{
	ushort val = (*buf)[0+off] | ((*buf)[1+off] << 8);
	return val;
}

static ushort GETPIXELI(unsigned char **buf, int off)
{
	ushort val = (*buf)[0+off] | ((*buf)[1+off] << 8);
	(*buf) += 2;
	return val;
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
		*y = sign *	  (i / 7);
	} else {
		*x = sign * (-14 + (i - 56) % 29);
		*y = sign *   (8 + (i - 56) / 29);
	}
}

static int close_table[512];
static int far_p_table[512];
static int far_n_table[512];

static void genLoopkupTable()
{
	int i;
	int x, y;

	for (i = 0; i < 256; i++) {
		relClose(i, &x, &y);

		close_table[i*2+0] = x;
		close_table[i*2+1] = y;

		relFar(i, 1, &x, &y);

		far_p_table[i*2+0] = x;
		far_p_table[i*2+1] = y;

		relFar(i, -1, &x, &y);

		far_n_table[i*2+0] = x;
		far_n_table[i*2+1] = y;
	}

	lookup_initialized = 1;
}

static void copyFrame(ushort *pDest, ushort *pSrc)
{
	int i;

	for (i=0; i<8; i++) {
		memcpy(pDest, pSrc, 16);
		pDest += g_width;
		pSrc += g_width;
	}
}

static void patternRow4Pixels(ushort *pFrame, unsigned char pat0, unsigned char pat1, ushort *p)
{
	ushort mask=0x0003;
	ushort shift=0;
	ushort pattern = (pat1 << 8) | pat0;

	while (mask != 0) {
		*pFrame++ = p[(mask & pattern) >> shift];
		mask <<= 2;
		shift += 2;
	}
}

static void patternRow4Pixels2(ushort *pFrame, unsigned char pat0, ushort *p)
{
	unsigned char mask=0x03;
	unsigned char shift=0;
	ushort pel;

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

static void patternRow4Pixels2x1(ushort *pFrame, unsigned char pat, ushort *p)
{
	unsigned char mask=0x03;
	unsigned char shift=0;
	ushort pel;

	while (mask != 0) {
		pel = p[(mask & pat) >> shift];
		pFrame[0] = pel;
		pFrame[1] = pel;
		pFrame += 2;
		mask <<= 2;
		shift += 2;
	}
}

static void patternQuadrant4Pixels(ushort *pFrame, unsigned char pat0, unsigned char pat1, unsigned char pat2, unsigned char pat3, ushort *p)
{
	unsigned long mask = 0x00000003UL;
	int shift=0;
	int i;
	unsigned long pat = (pat3 << 24) | (pat2 << 16) | (pat1 << 8) | pat0;

	for (i=0; i<16; i++) {
		pFrame[i&3] = p[(pat & mask) >> shift];
		if ((i&3) == 3) {
			pFrame += g_width;
		}
		mask <<= 2;
		shift += 2;
	}
}

static void patternRow2Pixels(ushort *pFrame, unsigned char pat, ushort *p)
{
	unsigned char mask=0x01;

	while (mask != 0) {
		*pFrame++ = p[(mask & pat) ? 1 : 0];
		mask <<= 1;
	}
}

static void patternRow2Pixels2(ushort *pFrame, unsigned char pat, ushort *p)
{
	ushort pel;
	unsigned char mask=0x1;

	while (mask != 0x10) {
		pel = p[(mask & pat) ? 1 : 0];

		pFrame[0] = pel;
		pFrame[1] = pel;
		pFrame[g_width + 0] = pel;
		pFrame[g_width + 1] = pel;
		pFrame += 2;

		mask <<= 1;
	}
}

static void patternQuadrant2Pixels(ushort *pFrame, unsigned char pat0, unsigned char pat1, ushort *p)
{
	ushort mask = 0x0001;
	int i;
	ushort pat = (pat1 << 8) | pat0;

	for (i=0; i<16; i++) {
		pFrame[i&3] = p[(pat & mask) ? 1 : 0];
		if ((i&3) == 3) {
			pFrame += g_width;
		}
		mask <<= 1;
	}

}

static void dispatchDecoder16(ushort **pFrame, unsigned char codeType, unsigned char **pData, unsigned char **pOffData, int *pDataRemain, int *curXb, int *curYb)
{
	ushort p[4];
	unsigned char pat[16];
	int i, j, k;
	int x, y;
	ushort *pDstBak;

	pDstBak = *pFrame;

	switch(codeType) {
		case 0x0:
			copyFrame(*pFrame, *pFrame + (backBuf2 - backBuf1));
		break; // ADDED

		case 0x1:
		break;

		case 0x2:
			k = *(*pOffData)++;
			x = far_p_table[k*2+0];
			y = far_p_table[k*2+1];
			copyFrame(*pFrame, *pFrame + x + y*g_width);
			(*pDataRemain)--;
		break;

		case 0x3:
			k = *(*pOffData)++;
			x = far_n_table[k*2+0];
			y = far_n_table[k*2+1];
			copyFrame(*pFrame, *pFrame + x + y*g_width);
			(*pDataRemain)--;
		break;

		case 0x4:
			k = *(*pOffData)++;
			x = close_table[k*2+0];
			y = close_table[k*2+1];
			copyFrame(*pFrame, *pFrame + (backBuf2 - backBuf1) + x + y*g_width);
			(*pDataRemain)--;
		break;

		case 0x5:
			x = (char)*(*pData)++;
			y = (char)*(*pData)++;
			copyFrame(*pFrame, *pFrame + (backBuf2 - backBuf1) + x + y*g_width);
			*pDataRemain -= 2;
		break;

		case 0x6:
			nprintf(("MVE", "STUB: encoding 6 not tested\n"));
			for (i=0; i<2; i++) {
				*pFrame += 16;
				if (++*curXb == (g_width >> 3)) {
					*pFrame += 7*g_width;
					*curXb = 0;
					if (++*curYb == (g_height >> 3)) {
						return;
					}
				}
			}
		break;

		case 0x7:
			p[0] = GETPIXELI(pData, 0);
			p[1] = GETPIXELI(pData, 0);
			if (!((p[0]/*|p[1]*/)&0x8000)) {
				for (i=0; i<8; i++) {
					patternRow2Pixels(*pFrame, *(*pData), p);
					(*pData)++;
					*pFrame += g_width;
				}
			} else {
				for (i=0; i<2; i++) {
					patternRow2Pixels2(*pFrame, *(*pData) & 0xf, p);
					*pFrame += 2*g_width;
					patternRow2Pixels2(*pFrame, *(*pData) >> 4, p);
					(*pData)++;
					*pFrame += 2*g_width;
				}
			}
		break;

		case 0x8:
			p[0] = GETPIXEL(pData, 0);
			if (!(p[0] & 0x8000)) {
				for (i=0; i<4; i++) {
					p[0] = GETPIXELI(pData, 0);
					p[1] = GETPIXELI(pData, 0);
					pat[0] = (*pData)[0];
					pat[1] = (*pData)[1];
					(*pData) += 2;
					patternQuadrant2Pixels(*pFrame, pat[0], pat[1], p);
					if (i & 1) {
						*pFrame -= (4*g_width - 4);
					} else {
						*pFrame += 4*g_width;
					}
				}
			} else {
				p[2] = GETPIXEL(pData, 8);
				if (!(p[2]&0x8000)) {
					for (i=0; i<4; i++) {
						if ((i & 1) == 0) {
							p[0] = GETPIXELI(pData, 0);
							p[1] = GETPIXELI(pData, 0);
						}
						pat[0] = *(*pData)++;
						pat[1] = *(*pData)++;
						patternQuadrant2Pixels(*pFrame, pat[0], pat[1], p);
						if (i & 1) {
							*pFrame -= (4*g_width - 4);
						} else {
							*pFrame += 4*g_width;
						}
					}
				} else {
					for (i=0; i<8; i++) {
						if ((i & 3) == 0) {
							p[0] = GETPIXELI(pData, 0);
							p[1] = GETPIXELI(pData, 0);
						}
						patternRow2Pixels(*pFrame, *(*pData), p);
						(*pData)++;
						*pFrame += g_width;
					}
				}
			}
		break;

		case 0x9:
			p[0] = GETPIXELI(pData, 0);
			p[1] = GETPIXELI(pData, 0);
			p[2] = GETPIXELI(pData, 0);
			p[3] = GETPIXELI(pData, 0);
			*pDataRemain -= 8;
			if (!(p[0] & 0x8000)) {
				if (!(p[2] & 0x8000)) {
					for (i=0; i<8; i++) {
						pat[0] = (*pData)[0];
						pat[1] = (*pData)[1];
						(*pData) += 2;
						patternRow4Pixels(*pFrame, pat[0], pat[1], p);
						*pFrame += g_width;
					}
					*pDataRemain -= 16;
				} else {
					patternRow4Pixels2(*pFrame, (*pData)[0], p);
					*pFrame += 2*g_width;
					patternRow4Pixels2(*pFrame, (*pData)[1], p);
					*pFrame += 2*g_width;
					patternRow4Pixels2(*pFrame, (*pData)[2], p);
					*pFrame += 2*g_width;
					patternRow4Pixels2(*pFrame, (*pData)[3], p);
					(*pData) += 4;
					*pDataRemain -= 4;
				}
			} else {
				if (!(p[2] & 0x8000))  {
					for (i=0; i<8; i++) {
						pat[0] = (*pData)[0];
						(*pData) += 1;
						patternRow4Pixels2x1(*pFrame, pat[0], p);
						*pFrame += g_width;
					}
					*pDataRemain -= 8;
				} else {
					for (i=0; i<4; i++) {
						pat[0] = (*pData)[0];
						pat[1] = (*pData)[1];
						(*pData) += 2;
						patternRow4Pixels(*pFrame, pat[0], pat[1], p);
						*pFrame += g_width;
						patternRow4Pixels(*pFrame, pat[0], pat[1], p);
						*pFrame += g_width;
					}
					*pDataRemain -= 8;
				}
			}
		break;

		case 0xa:
			p[0] = GETPIXEL(pData, 0);
			if (!(p[0] & 0x8000)) {
				for (i=0; i<4; i++) {
					p[0] = GETPIXELI(pData, 0);
					p[1] = GETPIXELI(pData, 0);
					p[2] = GETPIXELI(pData, 0);
					p[3] = GETPIXELI(pData, 0);
					pat[0] = (*pData)[0];
					pat[1] = (*pData)[1];
					pat[2] = (*pData)[2];
					pat[3] = (*pData)[3];
					(*pData) += 4;
					patternQuadrant4Pixels(*pFrame, pat[0], pat[1], pat[2], pat[3], p);
					if (i & 1) {
						*pFrame -= (4*g_width - 4);
					} else {
						*pFrame += 4*g_width;
					}
				}
			} else {
				p[0] = GETPIXEL(pData, 16);
				if (!(p[0] & 0x8000)) {
					for (i=0; i<4; i++) {
						if ((i&1) == 0) {
							p[0] = GETPIXELI(pData, 0); 
							p[1] = GETPIXELI(pData, 0);
							p[2] = GETPIXELI(pData, 0);
							p[3] = GETPIXELI(pData, 0);
						}
						pat[0] = (*pData)[0];
						pat[1] = (*pData)[1];
						pat[2] = (*pData)[2];
						pat[3] = (*pData)[3];
						(*pData) += 4;
						patternQuadrant4Pixels(*pFrame, pat[0], pat[1], pat[2], pat[3], p);
						if (i & 1) {
							*pFrame -= (4*g_width - 4);
						} else {
							*pFrame += 4*g_width;
						}
					}
				} else {
					for (i=0; i<8; i++) {
						if ((i&3) == 0) {
							p[0] = GETPIXELI(pData, 0);
							p[1] = GETPIXELI(pData, 0);
							p[2] = GETPIXELI(pData, 0);
							p[3] = GETPIXELI(pData, 0);
						}
						pat[0] = (*pData)[0];
						pat[1] = (*pData)[1];
						patternRow4Pixels(*pFrame, pat[0], pat[1], p);
						*pFrame += g_width;
						(*pData) += 2;
					}
				}
			}
		break;

		case 0xb:
			for (i=0; i<8; i++) {
#if BYTE_ORDER == BIG_ENDIAN
				ubyte frame_tmp[16];
				memcpy(&frame_tmp, *pData, 16);
				ushort *swap_tmp;
				for (j = 0; j < 16; j += 2) {
					swap_tmp = (ushort*)(frame_tmp + j);
					*swap_tmp = INTEL_SHORT(*swap_tmp);
				}
				memcpy(*pFrame, &frame_tmp, 16);
#else
				memcpy(*pFrame, *pData, 16);
#endif
				*pFrame += g_width;
				*pData += 16;
				*pDataRemain -= 16;
			}
		break;

		case 0xc:
			for (i=0; i<4; i++) {
				p[0] = GETPIXEL(pData, 0);
				p[1] = GETPIXEL(pData, 2);
				p[2] = GETPIXEL(pData, 4);
				p[3] = GETPIXEL(pData, 6);
				for (j=0; j<2; j++) {
					for (k=0; k<4; k++) {
						(*pFrame)[2*k] = p[k];
						(*pFrame)[2*k+1] = p[k];
					}
					*pFrame += g_width;
				}
				*pData += 8;
				*pDataRemain -= 8;
			}
		break;

		case 0xd:
			for (i=0; i<2; i++) {
				p[0] = GETPIXEL(pData, 0);
				p[1] = GETPIXEL(pData, 2);
				for (j=0; j<4; j++) {
					for (k=0; k<4; k++) {
						(*pFrame)[k*g_width+j] = p[0];
						(*pFrame)[k*g_width+j+4] = p[1];
					}
				}
				*pFrame += 4*g_width;
				*pData += 4;
				*pDataRemain -= 4;
			}
		break;

		case 0xe:
			p[0] = GETPIXEL(pData, 0);
			for (i = 0; i < 8; i++) {
				for (j = 0; j < 8; j++) {
					(*pFrame)[j] = p[0];
				}
				*pFrame += g_width;
			}
			*pData += 2;
			*pDataRemain -= 2;
		break;

		case 0xf:
			p[0] = GETPIXEL(pData, 0);
			p[1] = GETPIXEL(pData, 1);
			for (i=0; i<8; i++) {
				for (j=0; j<8; j++) {
					(*pFrame)[j] = p[(i+j)&1];
				}
				*pFrame += g_width;
			}
			*pData += 4;
			*pDataRemain -= 4;
		break;

		default:
	break;
	}

	*pFrame = pDstBak+8;
}
