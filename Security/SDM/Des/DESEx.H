/*--------------------------------------------------------------------------------
DES.H: DES Algorithm Program from the Book Appliced Cryptography, Bruce Schneier
--------------------------------------------------------------------------------*/

#ifndef _DES_H_
#define _DES_H_

#define EN0   0      /* MODE == encrypt */
#define DE1   1      /* MODE == decrypt */
#include "../stdafx.h"

typedef struct
{
    unsigned long ek[32];
    unsigned long dk[32];
} des_ctx;

#ifdef __cplusplus
extern "C" {
#endif

void des_key(des_ctx *dc, unsigned char *key);
void des_enc(des_ctx *dc, unsigned char *Data, int blocks);
void des_dec(des_ctx *dc, unsigned char *Data, int blocks);

#ifdef __cplusplus
}
#endif

#endif //_DES_H_