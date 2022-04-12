#pragma once

#include "Board.h"

void ConvertPdm2Pcm(uint16_t *pdmData128, float32_t *pcmOutBuffer16);
void PCM_post_processing(float *PCM_buff, int size);

#ifdef __GNUC__
//#define d(res,a,b)	__ASM volatile ("VSUB.F32 %[result],%[op1], %[op2]\n\t"	"VABS.F32 %[result], %[result]\n\t":[result] "+t" (res):[op1] "t" (a),[op2] "t"
//(b)	);		//res=|a-b|
#define abzf(res, f) res = ((f) >= 0 ? (f) : (0 - f)) // res=|f|
#define d(res, a, b) res = (((a) - (b)) >= 0 ? ((a) - (b)) : (0 - ((a) - (b))))
#else
#define abzf(res, f)                                                                                                                                           \
	__ASM volatile {                                                                                                                                           \
		VABS.F32 res, f;                                                                                                                                       \
	} // res=|f|
#define d(res, a, b)                                                                                                                                           \
	__ASM volatile {                                                                                                                                           \
		VSUB.F32 res, a, b;                                                                                                                                    \
		VABS.F32 res, res;                                                                                                                                     \
	} // res=|a-b|
#endif