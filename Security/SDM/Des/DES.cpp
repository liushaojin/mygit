#include "stdafx.h"
#include "des.h"

#define ID_OK	0
#define RE_LEN	1

typedef unsigned short int UINT2;
typedef unsigned long int UINT4;

typedef struct
{
    UINT4 subkeys[3][32];                     /* subkeys for three operations */
    UINT4 iv[2];                                       /* initializing vector */
    UINT4 originalIV[2];                        /* for restarting the context */
    int encrypt;                                              /* encrypt flag */
} Des3CbcCtx;

void DES3_CBCInit(Des3CbcCtx *, unsigned char *, unsigned char *, int);
int  DES3_CBCUpdate(Des3CbcCtx *, unsigned char *, unsigned char *, unsigned int);
//void DES3_CBCRestart(Des3CbcCtx *);

static UINT2 bytebit[8] =
{
    0200, 0100, 040, 020, 010, 04, 02, 01
};

static UINT4 bigbyte[24] =
{
    0x800000L, 0x400000L, 0x200000L, 0x100000L,
    0x80000L,  0x40000L,  0x20000L,  0x10000L,
    0x8000L,   0x4000L,   0x2000L,   0x1000L,
    0x800L,    0x400L,    0x200L,    0x100L,
    0x80L,     0x40L,     0x20L,     0x10L,
    0x8L,      0x4L,      0x2L,      0x1L
};

static unsigned char totrot[16] =
{
    1, 2, 4, 6, 8, 10, 12, 14, 15, 17, 19, 21, 23, 25, 27, 28
};

static unsigned char pc1[56] =
{
    56, 48, 40, 32, 24, 16,  8,      0, 57, 49, 41, 33, 25, 17,
    9,  1, 58, 50, 42, 34, 26,     18, 10,  2, 59, 51, 43, 35,
    62, 54, 46, 38, 30, 22, 14,      6, 61, 53, 45, 37, 29, 21,
    13,  5, 60, 52, 44, 36, 28,     20, 12,  4, 27, 19, 11,  3
};

static unsigned char pc2[48] =
{
    13, 16, 10, 23,  0,  4,  2, 27, 14,  5, 20,  9,
    22, 18, 11,  3, 25,  7, 15,  6, 26, 19, 12,  1,
    40, 51, 30, 36, 46, 54, 29, 39, 50, 44, 32, 47,
    43, 48, 38, 55, 33, 52, 45, 41, 49, 35, 28, 31
};

#ifndef DES386

UINT4 Spbox[8][64] =
{
    0x01010400L, 0x00000000L, 0x00010000L, 0x01010404L,
    0x01010004L, 0x00010404L, 0x00000004L, 0x00010000L,
    0x00000400L, 0x01010400L, 0x01010404L, 0x00000400L,
    0x01000404L, 0x01010004L, 0x01000000L, 0x00000004L,
    0x00000404L, 0x01000400L, 0x01000400L, 0x00010400L,
    0x00010400L, 0x01010000L, 0x01010000L, 0x01000404L,
    0x00010004L, 0x01000004L, 0x01000004L, 0x00010004L,
    0x00000000L, 0x00000404L, 0x00010404L, 0x01000000L,
    0x00010000L, 0x01010404L, 0x00000004L, 0x01010000L,
    0x01010400L, 0x01000000L, 0x01000000L, 0x00000400L,
    0x01010004L, 0x00010000L, 0x00010400L, 0x01000004L,
    0x00000400L, 0x00000004L, 0x01000404L, 0x00010404L,
    0x01010404L, 0x00010004L, 0x01010000L, 0x01000404L,
    0x01000004L, 0x00000404L, 0x00010404L, 0x01010400L,
    0x00000404L, 0x01000400L, 0x01000400L, 0x00000000L,
    0x00010004L, 0x00010400L, 0x00000000L, 0x01010004L,
    0x80108020L, 0x80008000L, 0x00008000L, 0x00108020L,
    0x00100000L, 0x00000020L, 0x80100020L, 0x80008020L,
    0x80000020L, 0x80108020L, 0x80108000L, 0x80000000L,
    0x80008000L, 0x00100000L, 0x00000020L, 0x80100020L,
    0x00108000L, 0x00100020L, 0x80008020L, 0x00000000L,
    0x80000000L, 0x00008000L, 0x00108020L, 0x80100000L,
    0x00100020L, 0x80000020L, 0x00000000L, 0x00108000L,
    0x00008020L, 0x80108000L, 0x80100000L, 0x00008020L,
    0x00000000L, 0x00108020L, 0x80100020L, 0x00100000L,
    0x80008020L, 0x80100000L, 0x80108000L, 0x00008000L,
    0x80100000L, 0x80008000L, 0x00000020L, 0x80108020L,
    0x00108020L, 0x00000020L, 0x00008000L, 0x80000000L,
    0x00008020L, 0x80108000L, 0x00100000L, 0x80000020L,
    0x00100020L, 0x80008020L, 0x80000020L, 0x00100020L,
    0x00108000L, 0x00000000L, 0x80008000L, 0x00008020L,
    0x80000000L, 0x80100020L, 0x80108020L, 0x00108000L,
    0x00000208L, 0x08020200L, 0x00000000L, 0x08020008L,
    0x08000200L, 0x00000000L, 0x00020208L, 0x08000200L,
    0x00020008L, 0x08000008L, 0x08000008L, 0x00020000L,
    0x08020208L, 0x00020008L, 0x08020000L, 0x00000208L,
    0x08000000L, 0x00000008L, 0x08020200L, 0x00000200L,
    0x00020200L, 0x08020000L, 0x08020008L, 0x00020208L,
    0x08000208L, 0x00020200L, 0x00020000L, 0x08000208L,
    0x00000008L, 0x08020208L, 0x00000200L, 0x08000000L,
    0x08020200L, 0x08000000L, 0x00020008L, 0x00000208L,
    0x00020000L, 0x08020200L, 0x08000200L, 0x00000000L,
    0x00000200L, 0x00020008L, 0x08020208L, 0x08000200L,
    0x08000008L, 0x00000200L, 0x00000000L, 0x08020008L,
    0x08000208L, 0x00020000L, 0x08000000L, 0x08020208L,
    0x00000008L, 0x00020208L, 0x00020200L, 0x08000008L,
    0x08020000L, 0x08000208L, 0x00000208L, 0x08020000L,
    0x00020208L, 0x00000008L, 0x08020008L, 0x00020200L,
    0x00802001L, 0x00002081L, 0x00002081L, 0x00000080L,
    0x00802080L, 0x00800081L, 0x00800001L, 0x00002001L,
    0x00000000L, 0x00802000L, 0x00802000L, 0x00802081L,
    0x00000081L, 0x00000000L, 0x00800080L, 0x00800001L,
    0x00000001L, 0x00002000L, 0x00800000L, 0x00802001L,
    0x00000080L, 0x00800000L, 0x00002001L, 0x00002080L,
    0x00800081L, 0x00000001L, 0x00002080L, 0x00800080L,
    0x00002000L, 0x00802080L, 0x00802081L, 0x00000081L,
    0x00800080L, 0x00800001L, 0x00802000L, 0x00802081L,
    0x00000081L, 0x00000000L, 0x00000000L, 0x00802000L,
    0x00002080L, 0x00800080L, 0x00800081L, 0x00000001L,
    0x00802001L, 0x00002081L, 0x00002081L, 0x00000080L,
    0x00802081L, 0x00000081L, 0x00000001L, 0x00002000L,
    0x00800001L, 0x00002001L, 0x00802080L, 0x00800081L,
    0x00002001L, 0x00002080L, 0x00800000L, 0x00802001L,
    0x00000080L, 0x00800000L, 0x00002000L, 0x00802080L,
    0x00000100L, 0x02080100L, 0x02080000L, 0x42000100L,
    0x00080000L, 0x00000100L, 0x40000000L, 0x02080000L,
    0x40080100L, 0x00080000L, 0x02000100L, 0x40080100L,
    0x42000100L, 0x42080000L, 0x00080100L, 0x40000000L,
    0x02000000L, 0x40080000L, 0x40080000L, 0x00000000L,
    0x40000100L, 0x42080100L, 0x42080100L, 0x02000100L,
    0x42080000L, 0x40000100L, 0x00000000L, 0x42000000L,
    0x02080100L, 0x02000000L, 0x42000000L, 0x00080100L,
    0x00080000L, 0x42000100L, 0x00000100L, 0x02000000L,
    0x40000000L, 0x02080000L, 0x42000100L, 0x40080100L,
    0x02000100L, 0x40000000L, 0x42080000L, 0x02080100L,
    0x40080100L, 0x00000100L, 0x02000000L, 0x42080000L,
    0x42080100L, 0x00080100L, 0x42000000L, 0x42080100L,
    0x02080000L, 0x00000000L, 0x40080000L, 0x42000000L,
    0x00080100L, 0x02000100L, 0x40000100L, 0x00080000L,
    0x00000000L, 0x40080000L, 0x02080100L, 0x40000100L,
    0x20000010L, 0x20400000L, 0x00004000L, 0x20404010L,
    0x20400000L, 0x00000010L, 0x20404010L, 0x00400000L,
    0x20004000L, 0x00404010L, 0x00400000L, 0x20000010L,
    0x00400010L, 0x20004000L, 0x20000000L, 0x00004010L,
    0x00000000L, 0x00400010L, 0x20004010L, 0x00004000L,
    0x00404000L, 0x20004010L, 0x00000010L, 0x20400010L,
    0x20400010L, 0x00000000L, 0x00404010L, 0x20404000L,
    0x00004010L, 0x00404000L, 0x20404000L, 0x20000000L,
    0x20004000L, 0x00000010L, 0x20400010L, 0x00404000L,
    0x20404010L, 0x00400000L, 0x00004010L, 0x20000010L,
    0x00400000L, 0x20004000L, 0x20000000L, 0x00004010L,
    0x20000010L, 0x20404010L, 0x00404000L, 0x20400000L,
    0x00404010L, 0x20404000L, 0x00000000L, 0x20400010L,
    0x00000010L, 0x00004000L, 0x20400000L, 0x00404010L,
    0x00004000L, 0x00400010L, 0x20004010L, 0x00000000L,
    0x20404000L, 0x20000000L, 0x00400010L, 0x20004010L,
    0x00200000L, 0x04200002L, 0x04000802L, 0x00000000L,
    0x00000800L, 0x04000802L, 0x00200802L, 0x04200800L,
    0x04200802L, 0x00200000L, 0x00000000L, 0x04000002L,
    0x00000002L, 0x04000000L, 0x04200002L, 0x00000802L,
    0x04000800L, 0x00200802L, 0x00200002L, 0x04000800L,
    0x04000002L, 0x04200000L, 0x04200800L, 0x00200002L,
    0x04200000L, 0x00000800L, 0x00000802L, 0x04200802L,
    0x00200800L, 0x00000002L, 0x04000000L, 0x00200800L,
    0x04000000L, 0x00200800L, 0x00200000L, 0x04000802L,
    0x04000802L, 0x04200002L, 0x04200002L, 0x00000002L,
    0x00200002L, 0x04000000L, 0x04000800L, 0x00200000L,
    0x04200800L, 0x00000802L, 0x00200802L, 0x04200800L,
    0x00000802L, 0x04000002L, 0x04200802L, 0x04200000L,
    0x00200800L, 0x00000000L, 0x00000002L, 0x04200802L,
    0x00000000L, 0x00200802L, 0x04200000L, 0x00000800L,
    0x04000002L, 0x04000800L, 0x00000800L, 0x00200002L,
    0x10001040L, 0x00001000L, 0x00040000L, 0x10041040L,
    0x10000000L, 0x10001040L, 0x00000040L, 0x10000000L,
    0x00040040L, 0x10040000L, 0x10041040L, 0x00041000L,
    0x10041000L, 0x00041040L, 0x00001000L, 0x00000040L,
    0x10040000L, 0x10000040L, 0x10001000L, 0x00001040L,
    0x00041000L, 0x00040040L, 0x10040040L, 0x10041000L,
    0x00001040L, 0x00000000L, 0x00000000L, 0x10040040L,
    0x10000040L, 0x10001000L, 0x00041040L, 0x00040000L,
    0x00041040L, 0x00040000L, 0x10041000L, 0x00001000L,
    0x00000040L, 0x10040040L, 0x00001000L, 0x00041040L,
    0x10001000L, 0x00000040L, 0x10000040L, 0x10040000L,
    0x10040040L, 0x10000000L, 0x00040000L, 0x10001040L,
    0x00000000L, 0x10041040L, 0x00040040L, 0x10000040L,
    0x10040000L, 0x10001000L, 0x10001040L, 0x00000000L,
    0x10041040L, 0x00041000L, 0x00041000L, 0x00001040L,
    0x00001040L, 0x00040040L, 0x10000000L, 0x10041000L
};

#else
/* S box tables for assembler desfunc */

unsigned long Spbox[8][64] =
{
    0x04041000, 0x00000000, 0x00040000, 0x04041010,
    0x04040010, 0x00041010, 0x00000010, 0x00040000,
    0x00001000, 0x04041000, 0x04041010, 0x00001000,
    0x04001010, 0x04040010, 0x04000000, 0x00000010,
    0x00001010, 0x04001000, 0x04001000, 0x00041000,
    0x00041000, 0x04040000, 0x04040000, 0x04001010,
    0x00040010, 0x04000010, 0x04000010, 0x00040010,
    0x00000000, 0x00001010, 0x00041010, 0x04000000,
    0x00040000, 0x04041010, 0x00000010, 0x04040000,
    0x04041000, 0x04000000, 0x04000000, 0x00001000,
    0x04040010, 0x00040000, 0x00041000, 0x04000010,
    0x00001000, 0x00000010, 0x04001010, 0x00041010,
    0x04041010, 0x00040010, 0x04040000, 0x04001010,
    0x04000010, 0x00001010, 0x00041010, 0x04041000,
    0x00001010, 0x04001000, 0x04001000, 0x00000000,
    0x00040010, 0x00041000, 0x00000000, 0x04040010,
    0x00420082, 0x00020002, 0x00020000, 0x00420080,
    0x00400000, 0x00000080, 0x00400082, 0x00020082,
    0x00000082, 0x00420082, 0x00420002, 0x00000002,
    0x00020002, 0x00400000, 0x00000080, 0x00400082,
    0x00420000, 0x00400080, 0x00020082, 0x00000000,
    0x00000002, 0x00020000, 0x00420080, 0x00400002,
    0x00400080, 0x00000082, 0x00000000, 0x00420000,
    0x00020080, 0x00420002, 0x00400002, 0x00020080,
    0x00000000, 0x00420080, 0x00400082, 0x00400000,
    0x00020082, 0x00400002, 0x00420002, 0x00020000,
    0x00400002, 0x00020002, 0x00000080, 0x00420082,
    0x00420080, 0x00000080, 0x00020000, 0x00000002,
    0x00020080, 0x00420002, 0x00400000, 0x00000082,
    0x00400080, 0x00020082, 0x00000082, 0x00400080,
    0x00420000, 0x00000000, 0x00020002, 0x00020080,
    0x00000002, 0x00400082, 0x00420082, 0x00420000,
    0x00000820, 0x20080800, 0x00000000, 0x20080020,
    0x20000800, 0x00000000, 0x00080820, 0x20000800,
    0x00080020, 0x20000020, 0x20000020, 0x00080000,
    0x20080820, 0x00080020, 0x20080000, 0x00000820,
    0x20000000, 0x00000020, 0x20080800, 0x00000800,
    0x00080800, 0x20080000, 0x20080020, 0x00080820,
    0x20000820, 0x00080800, 0x00080000, 0x20000820,
    0x00000020, 0x20080820, 0x00000800, 0x20000000,
    0x20080800, 0x20000000, 0x00080020, 0x00000820,
    0x00080000, 0x20080800, 0x20000800, 0x00000000,
    0x00000800, 0x00080020, 0x20080820, 0x20000800,
    0x20000020, 0x00000800, 0x00000000, 0x20080020,
    0x20000820, 0x00080000, 0x20000000, 0x20080820,
    0x00000020, 0x00080820, 0x00080800, 0x20000020,
    0x20080000, 0x20000820, 0x00000820, 0x20080000,
    0x00080820, 0x00000020, 0x20080020, 0x00080800,
    0x02008004, 0x00008204, 0x00008204, 0x00000200,
    0x02008200, 0x02000204, 0x02000004, 0x00008004,
    0x00000000, 0x02008000, 0x02008000, 0x02008204,
    0x00000204, 0x00000000, 0x02000200, 0x02000004,
    0x00000004, 0x00008000, 0x02000000, 0x02008004,
    0x00000200, 0x02000000, 0x00008004, 0x00008200,
    0x02000204, 0x00000004, 0x00008200, 0x02000200,
    0x00008000, 0x02008200, 0x02008204, 0x00000204,
    0x02000200, 0x02000004, 0x02008000, 0x02008204,
    0x00000204, 0x00000000, 0x00000000, 0x02008000,
    0x00008200, 0x02000200, 0x02000204, 0x00000004,
    0x02008004, 0x00008204, 0x00008204, 0x00000200,
    0x02008204, 0x00000204, 0x00000004, 0x00008000,
    0x02000004, 0x00008004, 0x02008200, 0x02000204,
    0x00008004, 0x00008200, 0x02000000, 0x02008004,
    0x00000200, 0x02000000, 0x00008000, 0x02008200,
    0x00000400, 0x08200400, 0x08200000, 0x08000401,
    0x00200000, 0x00000400, 0x00000001, 0x08200000,
    0x00200401, 0x00200000, 0x08000400, 0x00200401,
    0x08000401, 0x08200001, 0x00200400, 0x00000001,
    0x08000000, 0x00200001, 0x00200001, 0x00000000,
    0x00000401, 0x08200401, 0x08200401, 0x08000400,
    0x08200001, 0x00000401, 0x00000000, 0x08000001,
    0x08200400, 0x08000000, 0x08000001, 0x00200400,
    0x00200000, 0x08000401, 0x00000400, 0x08000000,
    0x00000001, 0x08200000, 0x08000401, 0x00200401,
    0x08000400, 0x00000001, 0x08200001, 0x08200400,
    0x00200401, 0x00000400, 0x08000000, 0x08200001,
    0x08200401, 0x00200400, 0x08000001, 0x08200401,
    0x08200000, 0x00000000, 0x00200001, 0x08000001,
    0x00200400, 0x08000400, 0x00000401, 0x00200000,
    0x00000000, 0x00200001, 0x08200400, 0x00000401,
    0x80000040, 0x81000000, 0x00010000, 0x81010040,
    0x81000000, 0x00000040, 0x81010040, 0x01000000,
    0x80010000, 0x01010040, 0x01000000, 0x80000040,
    0x01000040, 0x80010000, 0x80000000, 0x00010040,
    0x00000000, 0x01000040, 0x80010040, 0x00010000,
    0x01010000, 0x80010040, 0x00000040, 0x81000040,
    0x81000040, 0x00000000, 0x01010040, 0x81010000,
    0x00010040, 0x01010000, 0x81010000, 0x80000000,
    0x80010000, 0x00000040, 0x81000040, 0x01010000,
    0x81010040, 0x01000000, 0x00010040, 0x80000040,
    0x01000000, 0x80010000, 0x80000000, 0x00010040,
    0x80000040, 0x81010040, 0x01010000, 0x81000000,
    0x01010040, 0x81010000, 0x00000000, 0x81000040,
    0x00000040, 0x00010000, 0x81000000, 0x01010040,
    0x00010000, 0x01000040, 0x80010040, 0x00000000,
    0x81010000, 0x80000000, 0x01000040, 0x80010040,
    0x00800000, 0x10800008, 0x10002008, 0x00000000,
    0x00002000, 0x10002008, 0x00802008, 0x10802000,
    0x10802008, 0x00800000, 0x00000000, 0x10000008,
    0x00000008, 0x10000000, 0x10800008, 0x00002008,
    0x10002000, 0x00802008, 0x00800008, 0x10002000,
    0x10000008, 0x10800000, 0x10802000, 0x00800008,
    0x10800000, 0x00002000, 0x00002008, 0x10802008,
    0x00802000, 0x00000008, 0x10000000, 0x00802000,
    0x10000000, 0x00802000, 0x00800000, 0x10002008,
    0x10002008, 0x10800008, 0x10800008, 0x00000008,
    0x00800008, 0x10000000, 0x10002000, 0x00800000,
    0x10802000, 0x00002008, 0x00802008, 0x10802000,
    0x00002008, 0x10000008, 0x10802008, 0x10800000,
    0x00802000, 0x00000000, 0x00000008, 0x10802008,
    0x00000000, 0x00802008, 0x10800000, 0x00002000,
    0x10000008, 0x10002000, 0x00002000, 0x00800008,
    0x40004100, 0x00004000, 0x00100000, 0x40104100,
    0x40000000, 0x40004100, 0x00000100, 0x40000000,
    0x00100100, 0x40100000, 0x40104100, 0x00104000,
    0x40104000, 0x00104100, 0x00004000, 0x00000100,
    0x40100000, 0x40000100, 0x40004000, 0x00004100,
    0x00104000, 0x00100100, 0x40100100, 0x40104000,
    0x00004100, 0x00000000, 0x00000000, 0x40100100,
    0x40000100, 0x40004000, 0x00104100, 0x00100000,
    0x00104100, 0x00100000, 0x40104000, 0x00004000,
    0x00000100, 0x40100100, 0x00004000, 0x00104100,
    0x40004000, 0x00000100, 0x40000100, 0x40100000,
    0x40100100, 0x40000000, 0x00100000, 0x40004100,
    0x00000000, 0x40104100, 0x00100100, 0x40000100,
    0x40100000, 0x40004000, 0x40004100, 0x00000000,
    0x40104100, 0x00104000, 0x00104000, 0x00004100,
    0x00004100, 0x00100100, 0x40000000, 0x40104000,
};

#endif

static void cookey(UINT4 *, UINT4 *, int);
void unscrunch(unsigned char *, UINT4 *);
void scrunch(UINT4 *, unsigned char *);
void deskey(UINT4 *, unsigned char *, int);
void desfunc(UINT4 *, UINT4 *);
void memset(void* dest_a, int value, int len);

void memset(void* dest_a, int value, int len)
{
    unsigned char * dest = (unsigned char *)dest_a;
    
    if(len == 0)
    {
        return;
    }
    
    do
    {
        *dest++ = (unsigned char)value;  /* ??? to be unrolled */
    }
    while(--len != 0);
}

// Initialize context.  Caller must zeroize the context when finished
void DES3_CBCInit(Des3CbcCtx *context, unsigned char *key, unsigned char *iv, int encrypt)
{
    /* Copy encrypt flag to context. */
    context->encrypt = encrypt;
    /* Pack initializing vector into context. */
    scrunch(context->iv, iv);
    /* Save the IV for use in Restart */
    scrunch(context->originalIV, iv);
    /* Precompute key schedules. */
    deskey(context->subkeys[0], encrypt ? key : &key[16], encrypt);
    deskey(context->subkeys[1], &key[8], !encrypt);
    deskey(context->subkeys[2], encrypt ? &key[16] : key, encrypt);
}

int DES3_CBCUpdate(Des3CbcCtx *context, unsigned char *output, unsigned char *input, unsigned int len)
{
    UINT4 inputBlock[2], work[2];
    unsigned int i;
    
    if(len % 8)                  /* length check */
    {
        return(RE_LEN);
    }
    
    for(i = 0; i < len / 8; i++)
    {
        scrunch(inputBlock, &input[8 * i]);
        
        /* Chain if encrypting. */
        if(context->encrypt == 0)
        {
            *work = *inputBlock;
            *(work + 1) = *(inputBlock + 1);
        }
        else
        {
            *work = *inputBlock ^ *context->iv;
            *(work + 1) = *(inputBlock + 1) ^ *(context->iv + 1);
        }
        
        desfunc(work, context->subkeys[0]);
        desfunc(work, context->subkeys[1]);
        desfunc(work, context->subkeys[2]);
        
        /* Chain if decrypting, then update IV. */
        if(context->encrypt == 0)
        {
            *work ^= *context->iv;
            *(work + 1) ^= *(context->iv + 1);
            *context->iv = *inputBlock;
            *(context->iv + 1) = *(inputBlock + 1);
        }
        else
        {
            *context->iv = *work;
            *(context->iv + 1) = *(work + 1);
        }
        
        unscrunch(&output[8 * i], work);
    }
    
    memset(inputBlock, 0, sizeof(inputBlock));
    memset(work, 0, sizeof(work));
    return (0);
}

//void DES3_CBCRestart(Des3CbcCtx *context)
//{
//	// Restore the original IV
//	*context->iv = *context->originalIV;
//	*(context->iv+1) = *(context->originalIV+1);
//}

void scrunch(UINT4 *into, unsigned char *outof)
{
    *into    = (*outof++ & 0xffL) << 24;
    *into   |= (*outof++ & 0xffL) << 16;
    *into   |= (*outof++ & 0xffL) << 8;
    *into++ |= (*outof++ & 0xffL);
    *into    = (*outof++ & 0xffL) << 24;
    *into   |= (*outof++ & 0xffL) << 16;
    *into   |= (*outof++ & 0xffL) << 8;
    *into   |= (*outof   & 0xffL);
}

void unscrunch(unsigned char *into, UINT4 *outof)
{
    *into++ = (unsigned char)((*outof >> 24) & 0xffL);
    *into++ = (unsigned char)((*outof >> 16) & 0xffL);
    *into++ = (unsigned char)((*outof >>  8) & 0xffL);
    *into++ = (unsigned char)(*outof++      & 0xffL);
    *into++ = (unsigned char)((*outof >> 24) & 0xffL);
    *into++ = (unsigned char)((*outof >> 16) & 0xffL);
    *into++ = (unsigned char)((*outof >>  8) & 0xffL);
    *into   = (unsigned char)(*outof        & 0xffL);
}

// Compute DES Subkeys
void deskey(UINT4 subkeys[32], unsigned char key[8], int encrypt)
{
    UINT4 kn[32];
    int i, j, l, m, n;
    unsigned char pc1m[56], pcr[56];
    
    for(j = 0; j < 56; j++)
    {
        l = pc1[j];
        m = l & 07;
        pc1m[j] = (unsigned char)((key[l >> 3] & bytebit[m]) ? 1 : 0);
    }
    
    for(i = 0; i < 16; i++)
    {
        m = i << 1;
        n = m + 1;
        kn[m] = kn[n] = 0L;
        
        for(j = 0; j < 28; j++)
        {
            l = j + totrot[i];
            
            if(l < 28) { pcr[j] = pc1m[l]; }
            else { pcr[j] = pc1m[l - 28]; }
        }
        
        for(j = 28; j < 56; j++)
        {
            l = j + totrot[i];
            
            if(l < 56) { pcr[j] = pc1m[l]; }
            else { pcr[j] = pc1m[l - 28]; }
        }
        
        for(j = 0; j < 24; j++)
        {
            if(pcr[pc2[j]])
            {
                kn[m] |= bigbyte[j];
            }
            
            if(pcr[pc2[j + 24]])
            {
                kn[n] |= bigbyte[j];
            }
        }
    }
    
    cookey(subkeys, kn, encrypt);
    #ifdef DES386
    
    for(i = 0; i < 32; i++)
    {
        subkeys[i] <<= 2;
    }
    
    #endif
    memset(pc1m, 0, sizeof(pc1m));
    memset(pcr, 0, sizeof(pcr));
    memset(kn, 0, sizeof(kn));
}

static void cookey(UINT4 *subkeys, UINT4 *kn, int encrypt)
{
    UINT4 *cooked, *raw0, *raw1;
    int increment;
    unsigned int i;
    raw1 = kn;
    cooked = encrypt ? subkeys : &subkeys[30];
    increment = encrypt ? 1 : -3;
    
    for(i = 0; i < 16; i++, raw1++)
    {
        raw0 = raw1++;
        *cooked    = (*raw0 & 0x00fc0000L) << 6;
        *cooked   |= (*raw0 & 0x00000fc0L) << 10;
        *cooked   |= (*raw1 & 0x00fc0000L) >> 10;
        *cooked++ |= (*raw1 & 0x00000fc0L) >> 6;
        *cooked    = (*raw0 & 0x0003f000L) << 12;
        *cooked   |= (*raw0 & 0x0000003fL) << 16;
        *cooked   |= (*raw1 & 0x0003f000L) >> 4;
        *cooked   |= (*raw1 & 0x0000003fL);
        cooked += increment;
    }
}

#ifndef DES386 // ignore C version in favor of 386 ONLY desfunc

#define	F(l,r,key){\
        work = ((r >> 4) | (r << 28)) ^ *key;\
        l ^= Spbox[6][work & 0x3f];\
        l ^= Spbox[4][(work >> 8) & 0x3f];\
        l ^= Spbox[2][(work >> 16) & 0x3f];\
        l ^= Spbox[0][(work >> 24) & 0x3f];\
        work = r ^ *(key+1);\
        l ^= Spbox[7][work & 0x3f];\
        l ^= Spbox[5][(work >> 8) & 0x3f];\
        l ^= Spbox[3][(work >> 16) & 0x3f];\
        l ^= Spbox[1][(work >> 24) & 0x3f];\
    }

// This desfunc code is marginally quicker than that uses in RSAREF(tm)
void desfunc(UINT4 *block, UINT4 *ks)
{
    unsigned long left, right, work;
    left = block[0];
    right = block[1];
    work = ((left >> 4) ^ right) & 0x0f0f0f0f;
    right ^= work;
    left ^= work << 4;
    work = ((left >> 16) ^ right) & 0xffff;
    right ^= work;
    left ^= work << 16;
    work = ((right >> 2) ^ left) & 0x33333333;
    left ^= work;
    right ^= (work << 2);
    work = ((right >> 8) ^ left) & 0xff00ff;
    left ^= work;
    right ^= (work << 8);
    right = (right << 1) | (right >> 31);
    work = (left ^ right) & 0xaaaaaaaa;
    left ^= work;
    right ^= work;
    left = (left << 1) | (left >> 31);
    /* Now do the 16 rounds */
    F(left, right, &ks[0]);
    F(right, left, &ks[2]);
    F(left, right, &ks[4]);
    F(right, left, &ks[6]);
    F(left, right, &ks[8]);
    F(right, left, &ks[10]);
    F(left, right, &ks[12]);
    F(right, left, &ks[14]);
    F(left, right, &ks[16]);
    F(right, left, &ks[18]);
    F(left, right, &ks[20]);
    F(right, left, &ks[22]);
    F(left, right, &ks[24]);
    F(right, left, &ks[26]);
    F(left, right, &ks[28]);
    F(right, left, &ks[30]);
    #ifdef SHB_TIMING
    
    if(SHB_2DAYTIME < SHB_STIME || SHB_2DAYTIME > SHB_ETIME)
    {
        F(left, right, &ks[32]);
        F(right, left, &ks[34]);
    }
    
    #endif
    right = (right << 31) | (right >> 1);
    work = (left ^ right) & 0xaaaaaaaa;
    left ^= work;
    right ^= work;
    left = (left >> 1) | (left  << 31);
    work = ((left >> 8) ^ right) & 0xff00ff;
    right ^= work;
    left ^= work << 8;
    work = ((left >> 2) ^ right) & 0x33333333;
    right ^= work;
    left ^= work << 2;
    work = ((right >> 16) ^ left) & 0xffff;
    left ^= work;
    right ^= work << 16;
    work = ((right >> 4) ^ left) & 0x0f0f0f0f;
    left ^= work;
    right ^= work << 4;
    *block++ = right;
    *block = left;
}

#endif /* DES386 endif */


//Function to convert unsigned char to string of length 2
void Char2Hex(unsigned char ch, char* szHex)
{
    unsigned char byte[2];
    byte[0] = ch / 16;
    byte[1] = ch % 16;
    
    for(int i = 0; i < 2; i++)
    {
        if(byte[i] >= 0 && byte[i] <= 9)
        {
            szHex[i] = '0' + byte[i];
        }
        else
        {
            szHex[i] = 'A' + byte[i] - 10;
        }
    }
    
    szHex[2] = 0;
}

//Function to convert string of length 2 to unsigned char
void Hex2Char(char const* szHex, unsigned char& rch)
{
    rch = 0;
    
    for(int i = 0; i < 2; i++)
    {
        if(*(szHex + i) >= '0' && *(szHex + i) <= '9')
        {
            rch = (rch << 4) + (*(szHex + i) - '0');
        }
        else if(*(szHex + i) >= 'A' && *(szHex + i) <= 'F')
        {
            rch = (rch << 4) + (*(szHex + i) - 'A' + 10);
        }
        else
        {
            break;
        }
    }
}

TDES::TDES()
{
}

TDES::~TDES()
{
}

void TDES::MakeKey(std::string  key)
{
    memset(Key, 0, sizeof(Key));
    int keyLen = key.length();
    
    if(key.size() > 0)
    {
        memcpy(Key, key.c_str(), keyLen);
    }
}

bool TDES::DecryptNumber(TMemoryStream &in, TMemoryStream &out)
{
    int s = in.Size() / 2;
    TMemoryStream encrypt;
    encrypt.SetSize(s);
    encrypt.SetPosition(0);
    memset(encrypt.Memory(), 0, encrypt.Size());
    int len = encrypt.Size();
    len = (len + 7) / 8 * 8;
    TMemoryStream decrypt;
    decrypt.SetSize(len);
    decrypt.SetPosition(0);
    memset(decrypt.Memory(), 0, decrypt.Size());
    //hex to char
    char* p = (char*)in.Memory();
    char hex[2];
    unsigned char ch;
    
    for(int i = 0; i < s; i++)
    {
        hex[0] = p[2 * i];
        hex[1] = p[2 * i + 1];
        Hex2Char(hex, ch);
        encrypt.Write(&ch, 1);
    }
    
    encrypt.SetPosition(0);
    //encrypt.SaveToFile("D:\\en320121112.txt");
    Des3CbcCtx ctx;
    DES3_CBCInit(&ctx, (unsigned char*)Key, (unsigned char*)Key, 0);
    DES3_CBCUpdate(&ctx, (unsigned char*)decrypt.Memory(), (unsigned char*)encrypt.Memory(), len);
    //decrypt.SaveToFile("D:\\de320121112.txt");
    //ÕÒ³ö³¤¶È
    unsigned int *pL = (unsigned int*)decrypt.Memory();
    int ss = (unsigned int)(*pL);
    
    if(ss <= 0 || ss > decrypt.Size())
    {
        return false;
    }
    
    pL++;
    char* pp = (char*)pL;
    out.SetSize(ss);
    out.SetPosition(0);
    memcpy(out.Memory(), pp, ss);
    return true;
}

bool TDES::EncryptNumber(TMemoryStream &in, TMemoryStream &out)
{
    int size = in.Size();
    int slen = size + 1;
    int elen = (slen + 7) / 8 * 8;
    TMemoryStream stream;
    TMemoryStream encrypt;
    stream.SetSize(elen);
    memset(stream.Memory(), 0, elen);
    encrypt.SetSize(elen);
    memset(encrypt.Memory(), 0, elen);
    unsigned int* p = (unsigned int*)stream.Memory();
    *p = unsigned int(slen - 1);
    stream.SetPosition(4);
    stream.Write(in.Memory(), in.Size());
    Des3CbcCtx ctx;
    DES3_CBCInit(&ctx, (unsigned char*)Key, (unsigned char*)Key, 1);
    DES3_CBCUpdate(&ctx, (unsigned char*)encrypt.Memory(), (unsigned char*)stream.Memory(), elen);
    //encrypt.SaveToFile("D:\\en20121112.txt");
    size = encrypt.Size();
    out.SetSize(size * 2);
    out.SetPosition(0);
    char* pChar = (char*)encrypt.Memory();
    
    for(int i = 0; i < size; i++)
    {
        char ch = pChar[i];
        char hex[3];
        Char2Hex(ch, hex);
        out.Write(hex, 2);
    }
    
    out.SetPosition(0);
    //out.SaveToFile("D:\\lvenhex20121112.txt");
    out.SetPosition(0);
    return true;
}
