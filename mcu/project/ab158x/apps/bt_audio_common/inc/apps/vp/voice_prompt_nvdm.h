/* Copyright Statement:
 *
 * (C) 2017  Airoha Technology Corp. All rights reserved.
 *
 * This software/firmware and related documentation ("Airoha Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to Airoha Technology Corp. ("Airoha") and/or its licensors.
 * Without the prior written permission of Airoha and/or its licensors,
 * any reproduction, modification, use or disclosure of Airoha Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 * You may only use, reproduce, modify, or distribute (as applicable) Airoha Software
 * if you have agreed to and been bound by the applicable license agreement with
 * Airoha ("License Agreement") and been granted explicit permission to do so within
 * the License Agreement ("Permitted User").  If you are not a Permitted User,
 * please cease any access or use of Airoha Software immediately.
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT AIROHA SOFTWARE RECEIVED FROM AIROHA AND/OR ITS REPRESENTATIVES
 * ARE PROVIDED TO RECEIVER ON AN "AS-IS" BASIS ONLY. AIROHA EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES AIROHA PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH AIROHA SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN AIROHA SOFTWARE. AIROHA SHALL ALSO NOT BE RESPONSIBLE FOR ANY AIROHA
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND AIROHA'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO AIROHA SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT AIROHA'S OPTION, TO REVISE OR REPLACE AIROHA SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * AIROHA FOR SUCH AIROHA SOFTWARE AT ISSUE.
 */

/**
 * File: app_voice_prompt_nvdm.h
 *
 * Description: This file defines the interface of app_voice_prompt_nvdm.c.
 *
 */

#ifndef __VOICE_PROMPT_NVDM_H__
#define __VOICE_PROMPT_NVDM_H__

#include "rofs.h"
#include "voice_prompt_nvkey_struct.h"

#define MAX_VP_LANGUAGE      5    /* Maximum number of VP languages. */

#ifndef PACKED
#define PACKED  __attribute__((packed))
#endif

/**************************************************************************************************
 * Structure of VP language infos stored in ROFS/NVkey.
 **************************************************************************************************/
/*
 ---------------------------------------------------------------------------------
 |   LCnt  |   Codec   |   ROFSID   |   Codec   |   ROFSID   |...
 ---------------------------------------------------------------------------------
 */

/**
 *  @brief This structure defines the info of an VP language.
 */
typedef struct {
    uint16_t langCodec;    /**<  Codec of current language. */
    uint16_t rofsFileId;   /**<  The ROFS ID of the current language. */
} PACKED voice_prompt_nv_lang_info_t;

/**
 *  @brief This structure defines the language info.
*/
typedef struct {
    uint8_t langCnt;    /**<  Language count. */
} PACKED voice_prompt_nv_lang_cnt_t;


/**************************************************************************************************
 * Structure of VP file infos of an language in ROFS/NVkey.
 **************************************************************************************************/
/*
 ----------------------------------------------------------------------------------------------------
 | EvtNum | EvtID  |FileCnt |     FileID     | EvtID  |FileCnt |     FileID     |     FileID     |...
 ----------------------------------------------------------------------------------------------------
*/

/**
 *  @brief This structure defines the info of an VP event in ROFS/NVkey.
*/
typedef struct {
    uint8_t eventId;    /**<  VP event ID, or VP index. */
    uint8_t fileCnt;    /**<  VP file count of an event ID. */
} PACKED voice_prompt_nv_event_t;

/**
 *  @brief This structure defines the header of VP file infos of an language in ROFS/NVkey.
*/
typedef struct {
    uint8_t eventNum;    /**<  VP event number of the current language. */
} PACKED voice_prompt_nv_event_num_t;

/**
 *  @brief This structure defines the info of an VP language.
*/
typedef struct {
    uint16_t langCodec;         /**<  Codec of current language. */
    uint8_t eventNum;           /**<  VP event number of the current language. */
    uint8_t *pEventTable;       /**<  Pointer to VP event table of current language. */
    uint8_t *pBuffer;           /**<  Pointer to VP file data buffer. */
} voice_prompt_nv_lang_t;

/**
 *  @brief This structure defines the context of VP files.
*/
typedef struct {
    uint16_t langCnt;                                  /**<  Language count. */
    uint8_t *pLangBuffer;                              /**<  Info buffer. */
    voice_prompt_nv_lang_t langInfo[MAX_VP_LANGUAGE];   /**<  Language infos. */
    voice_prompt_nv_sel_para_t *pSelPara;                    /**<  Selected language info. */
} voice_prompt_nv_ctl_t;

/**
* @brief      This function is to init VP file usage and languages.
*/
void voice_prompt_nvdm_init();

/**
* @brief       This function is to convert VP index to file ID.
* @param[in]   VpId, VP index or event ID.
* @param[out]  fileId, output buffer of file ID.
* @return      true means convert success.
*/
bool voice_prompt_nvdm_find_vp_file(uint8_t VpId, uint16_t *fileId);

/**
* @brief      This function is to dump all VP tables, only debug use.
*/
void voice_prompt_nvdm_dump_all();

/**
* @brief       This function is to get current selected language.
* @param[out]  langId, output buffer of selected language ID.
* @return      true means success.
*/
bool voice_prompt_nvdm_get_current_lang(uint8_t *langId);

/**
* @brief      This function is to set selected language.
* @param[in]  langIdx, the language ID to be selected.
* @return     true means success.
*/
bool voice_prompt_nvdm_set_current_lang(uint8_t langIdx);

/**
* @brief       This function is to get supported language count.
* @param[out]  langCnt, output buffer of language count.
* @return      true means success.
*/
bool voice_prompt_nvdm_get_lang_cnt(uint16_t *langCnt);

/**
* @brief       This function is to get all supported language codec.
* @param[out]  langCnt, output buffer of language codec.
* @return      true means success.
*/
bool voice_prompt_nvdm_get_support_lang(uint16_t *buffer);
#endif

