#ifndef _IGO_NR_H_
#define _IGO_NR_H_

#include <types.h>

#if defined(AIR_AI_NR_PREMIUM_ENABLE) || defined(AIR_AI_NR_PREMIUM_INEAR_ENABLE)

typedef enum {
    _NB = 0,
    _WB,
    _SWB,
    _BAND_MODE_MAX
} _BAND_MODE;

typedef enum {
    _ANC_OFF = 0,
    _ANC_ON,
    _PASSTHROUGH,
    _SWITCH_MODE_MAX
} _SWITCH_MODE;

typedef struct {
    uint32_t v1;
    uint32_t v2;
    uint32_t v3;
    uint32_t v4;
} version_info_t;

typedef enum {
    _NR_LEVEL1 = 0,
    _NR_LEVEL2,
    _NR_LEVEL_MAX
} _NR_LEVEL;

#define IGO_NR_MEMSIZE (63 * 1024)

typedef void (*p_IGO_NR_Get_Lib_Version)(version_info_t *version_info);
typedef void (*p_IGO_NR_Init)(void *DRAM_work, uint32_t *nvkey_nr_para, void*ff_eq_table, void *fb_eq_table, uint8_t switch_mode, uint8_t nr_level, uint8_t band_mode);
typedef void (*p_IGO_NR_Prcs)(int16_t* FF_out, int16_t* FB_out, int16_t* ref, int16_t* NR_out, uint32_t *nvkey_nr_para, uint8_t switch_mode, uint8_t nr_level, uint32_t gain, uint32_t wind_indication, void *DRAM_work);

extern void *g_igo_txnr_export_parameters[];

// Call this API at initial state, this API should be called only once. 
// DRAM_work: Pointer to work memory at DRAM which dynamically allocated by framework 
// Nr_para: Pointer to 4byte x 10 word tuning parameter data
// nvkey_nr_para: Pointer to 4bytes x 24 tuning parameter data
// switch_mode: Type of switching mode 
//     0: ANC_OFF
//     1: ANC_ON
//     2: PASS_THROUGH
#define IGO_NR_Init                 ((p_IGO_NR_Init)g_igo_txnr_export_parameters[0])

// Call this API for every frames. 
// FF_out: The address of FF MIC audio output 
// FB_out: The address of FB MIC audio output 
// ref: The address of reference signal 
// NR_out: the address of NR output 
// DRAM_work: Pointer to work memory at DRAM which dynamically allocated by framework 
#define IGO_NR_Prcs                 ((p_IGO_NR_Prcs)g_igo_txnr_export_parameters[1])

#define IGO_NR_Get_Lib_Version      ((p_IGO_NR_Get_Lib_Version)g_igo_txnr_export_parameters[2])

#endif

#endif
