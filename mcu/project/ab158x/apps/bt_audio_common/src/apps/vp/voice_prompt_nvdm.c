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
 * File: voice_prompt_nvdm.c
 *
 * Description: This file provide implementation of voice prompt file management.
 *
 */

#include "FreeRTOS.h"
#include "voice_prompt_nvdm.h"
#include "nvkey.h"
#include "nvkey_id_list.h"
#include "syslog.h"
#include "voice_prompt_local.h"

#define ROFSID_APP_VP_LANG 0x7100    /* The ROFS ID of VP language info. */

#define LOG_TAG  "VP_NVDM"

voice_prompt_nv_ctl_t gAppVpCtl;  /* The variable records context. */
uint8_t giSInited;          /* The flag of whether this module is initialized. */


static uint8_t *voice_prompt_nvdm_get_data(uint32_t NVKey)
{
    uint8_t *pBuffer = NULL;
    uint32_t len;

    do {
        /* Query length before malloc buffer. */
        if (nvkey_data_item_length(NVKey, &len) != NVKEY_STATUS_OK) {
            VP_LOG_MSGID_E(LOG_TAG" Error: init VP nvdm len fail, NVKey: 0x%x", 1, NVKey);
            break;
        }

        pBuffer = (uint8_t *)pvPortMalloc(len);
        if (pBuffer == NULL) {
            VP_LOG_MSGID_E(LOG_TAG "Error: malloc for langbuffer fail, need size: %d.", 1, len);
            break;
        }

        if (nvkey_read_data(NVKey, pBuffer, &len) != NVKEY_STATUS_OK) {
            VP_LOG_MSGID_E(LOG_TAG" Error: init VP nvdm fail, NVKey: 0x%x", 1, NVKey);
            vPortFree(pBuffer);
            pBuffer = NULL;
            break;
        }
    } while (0);

    return pBuffer;
}

/**
* @brief       This function convert ROFS ID between big endian and little endian.
* @param[in]   pFileId, the input ID buffer.
* @param[out]  fileId, the output ID buffer.
* @return      true means success.
*/
static bool voice_prompt_nvdm_convert_file_id(uint8_t *pFileId, uint16_t *fileId)
{
    uint16_t fIdHighByte, fIdLowByte;

    if (!pFileId) {
        return false;
    }

    fIdLowByte = *pFileId;
    fIdHighByte = *(pFileId + 1);

    *fileId = (uint16_t)((fIdHighByte << 8) | fIdLowByte);
    return true;
}

/**
* @brief      This function is for debug use, and dump the event table.
* @param[in]  pTable, the table to be dump.
* @param[in]  evtNum, the event number in the table.
*/
static void voice_prompt_nvdm_dump_evt_table(uint8_t *pTable, uint8_t evtNum)
{
    uint16_t i = 0, j = 0;
    uint8_t *table = pTable;
    voice_prompt_nv_event_t *pEvtInfo;

    VP_LOG_MSGID_I(LOG_TAG" Table detail, table: 0x%x, evt num: %d", 2, pTable, evtNum);
    for (; i < evtNum; i++) {
        pEvtInfo = (voice_prompt_nv_event_t *)table;
        /* Move the pointer to File ID data. */
        table += sizeof(voice_prompt_nv_event_t);

        VP_LOG_MSGID_I(LOG_TAG" evt id: 0x%x, fileCnt: 0x%x", 2, pEvtInfo->eventId, pEvtInfo->fileCnt);

        if (pEvtInfo->fileCnt != 0) {
            uint16_t fileId = 0;
            for (j = 0; j < pEvtInfo->fileCnt; j++) {
                voice_prompt_nvdm_convert_file_id(table, &fileId);
                VP_LOG_MSGID_I(LOG_TAG" fileId: 0x%0x", 1, fileId);
                /* Move to the next File ID, 2 bytes of a File ID. */
                table += 2;
            }
        } else {
            VP_LOG_MSGID_I(LOG_TAG" null fileId", 0);
        }
    }
}

/**
* @brief       This function gets ROFS file ID by VP ID in an event table.
* @param[in]   pTable, the event table research in.
* @param[in]   evtNum, the event number of the event table.
* @param[in]   VpId, the VP ID.
* @param[out]  fileId, the output buffer of the file ID.
* @return      true means success.
*/
static bool voice_prompt_nvdm_get_file_id(uint8_t *pTable, uint8_t evtNum, uint8_t VpId, uint16_t *fileId)
{
    uint16_t i = 0;
    uint8_t *table = pTable;
    uint8_t found = 0;
    voice_prompt_nv_event_t *pEvtInfo;

    for (; i < evtNum; i++) {
        pEvtInfo = (voice_prompt_nv_event_t *)table;
        /* Move the pointer to File ID data. */
        table += sizeof(voice_prompt_nv_event_t);

        if (pEvtInfo->fileCnt != 0) {
            /* Get fist file id as we need. */
            if (pEvtInfo->eventId == VpId && voice_prompt_nvdm_convert_file_id(table, fileId)) {
                found = 1;
                break;
            }
            /* Move to next event table. */
            table += 2 * pEvtInfo->fileCnt;
        }
    }

    if (!found) {
        VP_LOG_MSGID_E(LOG_TAG" Error: Counldn't found file id with VpId: 0x%x.", 1, VpId);
        return false;
    }
    return true;
}


void voice_prompt_nvdm_init()
{
    uint8_t i, err = 0;
    voice_prompt_nv_lang_cnt_t *pLang;
    voice_prompt_nv_lang_info_t *pLangInfo;
    voice_prompt_nv_event_num_t *pEvent;
    uint8_t *langBuffer = NULL;
    uint8_t *dataBuffer, *selParaBuffer;
    ROFS_FILEINFO_T *rofs_vp_lang;
    uint16_t rofsFileID = 0;

    if (giSInited == 1) {
        //VP_LOG_MSGID_I(LOG_TAG" VP nvdm has inited. cur lang: 0x%x ", 1,
        //               gAppVpCtl.langInfo[gAppVpCtl.pSelPara->langId].langCodec);
        return ;
    }

    do {
        /* Step1: read lang cnt. */
        rofs_vp_lang = ROFS_fopen((unsigned short)ROFSID_APP_VP_LANG);
        if (rofs_vp_lang) {
            langBuffer = (uint8_t *)ROFS_address(rofs_vp_lang);
        }
        if (langBuffer == NULL) {
            err = 1;
            break;
        }
        pLang = (voice_prompt_nv_lang_cnt_t *)langBuffer;

        gAppVpCtl.langCnt = pLang->langCnt;
        gAppVpCtl.pLangBuffer = langBuffer;

        VP_LOG_MSGID_I(LOG_TAG" langCnt: %d.", 1, gAppVpCtl.langCnt);

        /* Step2: read event number and event table of every language. */
        for (i = 0; i < gAppVpCtl.langCnt; i++) {
            pLangInfo = (voice_prompt_nv_lang_info_t *)(((uint8_t *)pLang) + sizeof(voice_prompt_nv_lang_cnt_t) + i * sizeof(voice_prompt_nv_lang_info_t));
            gAppVpCtl.langInfo[i].langCodec = pLangInfo->langCodec;

            voice_prompt_nvdm_convert_file_id((uint8_t *) & (pLangInfo->rofsFileId), &rofsFileID);
            VP_LOG_MSGID_I(LOG_TAG" rofsFileID: %d.", 1, rofsFileID);

            rofs_vp_lang = ROFS_fopen((unsigned short)rofsFileID);
            dataBuffer = (uint8_t *)ROFS_address(rofs_vp_lang);
            if (dataBuffer == NULL) {
                err = 1;
                break;
            }

            pEvent = (voice_prompt_nv_event_num_t *)dataBuffer;
            gAppVpCtl.langInfo[i].pBuffer = dataBuffer;
            if (pEvent) {
                gAppVpCtl.langInfo[i].eventNum = pEvent->eventNum;
                gAppVpCtl.langInfo[i].pEventTable = ((uint8_t *)pEvent) + sizeof(voice_prompt_nv_event_num_t);

                VP_LOG_MSGID_I(LOG_TAG" langCodec: 0x%x, nvkey: 0x%0x, eventNum: 0x%x, table: 0x%x", 4,
                               pLangInfo->langCodec, pLangInfo->rofsFileId, pEvent->eventNum, gAppVpCtl.langInfo[i].pEventTable);

#ifdef DEBUG_VP
                voice_prompt_nvdm_dump_evt_table(gAppVpCtl.langInfo[i].pEventTable, gAppVpCtl.langInfo[i].eventNum);
#endif
            }
        }

        /* Step3: read VP select parameter in nvkey. */
        selParaBuffer = voice_prompt_nvdm_get_data(NVID_APP_VP_SELECT_PARA);
        if (selParaBuffer == NULL) {
            /* Selected para nvkey has been removed after factory reset, set to default and write nvkey. */
            selParaBuffer = (uint8_t *)pvPortMalloc(sizeof(voice_prompt_nv_sel_para_t));
            if (selParaBuffer == NULL) {
                VP_LOG_MSGID_I(LOG_TAG" sel para malloc fail!", 0);
                err = 1;
                break;
            }
            memset(selParaBuffer, 0, sizeof(voice_prompt_nv_sel_para_t));
            VP_LOG_MSGID_I(LOG_TAG" using default language!", 0);
        }

        gAppVpCtl.pSelPara = (voice_prompt_nv_sel_para_t *)selParaBuffer;
    } while (FALSE);

    if (err == 0) {
        giSInited = 1;
    }
}

bool voice_prompt_nvdm_find_vp_file(uint8_t VpId, uint16_t *fileId)
{
    uint8_t i = 0;

    if (giSInited != 1) {
        //VP_LOG_MSGID_E(LOG_TAG" Error: VP NVDM has not been initilized.", 0);
        return false;
    }

    i = gAppVpCtl.pSelPara->langId;

    if (voice_prompt_nvdm_get_file_id(gAppVpCtl.langInfo[i].pEventTable,
                                      gAppVpCtl.langInfo[i].eventNum, VpId, fileId)) {
        return true;
    }

    VP_LOG_MSGID_E(LOG_TAG" Error: can no found file with vp id 0x%x", 1, VpId);

    return false;
}

void voice_prompt_nvdm_dump_all()
{
    uint8_t i = 0;

    for (; i < gAppVpCtl.langCnt; i++) {
        VP_LOG_MSGID_I(LOG_TAG" land codec: 0x%x", 1, gAppVpCtl.langInfo[i].langCodec);
        voice_prompt_nvdm_dump_evt_table(gAppVpCtl.langInfo[i].pEventTable, gAppVpCtl.langInfo[i].eventNum);
    }

    VP_LOG_MSGID_I(LOG_TAG" landID: 0x%x, selRound: 0x%x, selTime: 0x%x", 3,
                   gAppVpCtl.pSelPara->langId, gAppVpCtl.pSelPara->selectRound, gAppVpCtl.pSelPara->selectTime);
}

void voice_prompt_nvdm_deinit()
{
    giSInited = 0;

    for (uint8_t i = 0; i < gAppVpCtl.langCnt; i++) {
        if (gAppVpCtl.langInfo[i].pBuffer) {
            vPortFree(gAppVpCtl.langInfo[i].pBuffer);
        }
    }

    if (gAppVpCtl.pLangBuffer) {
        vPortFree(gAppVpCtl.pLangBuffer);
    }

    if (gAppVpCtl.pSelPara) {
        vPortFree(gAppVpCtl.pSelPara);
    }

    memset(&gAppVpCtl, 0, sizeof(voice_prompt_nv_ctl_t));
}

bool voice_prompt_nvdm_get_current_lang(uint8_t *langId)
{
    if (giSInited != 1) {
        //VP_LOG_MSGID_E(LOG_TAG" Error: VP NVDM has not been initilized.", 0);
        return false;
    }

    *langId = gAppVpCtl.pSelPara->langId;
    return true;
}

bool voice_prompt_nvdm_set_current_lang(uint8_t index)
{
    if (giSInited != 1) {
        //VP_LOG_MSGID_E(LOG_TAG" Error: VP NVDM has not been initilized.", 0);
        return false;
    }

    if (index >= gAppVpCtl.langCnt) {
        VP_LOG_MSGID_E(LOG_TAG" Error: invalide lang index: 0x%x", 1, index);
        return false;
    }

    /* Set selected VP language in context. */
    gAppVpCtl.pSelPara->langId = index;
    VP_LOG_MSGID_E(LOG_TAG" set lang: 0x%x, codec: 0x%x", 2, gAppVpCtl.pSelPara->langId, gAppVpCtl.langInfo[index].langCodec);
    /* Write to nvkey. */
    if (NVKEY_STATUS_OK == nvkey_write_data(NVID_APP_VP_SELECT_PARA, (uint8_t *)gAppVpCtl.pSelPara, sizeof(voice_prompt_nv_sel_para_t))) {
        return true;
    } else {
        return false;
    }
}

bool voice_prompt_nvdm_get_lang_cnt(uint16_t *langCnt)
{
    if (giSInited != 1) {
        return false;
    }

    *langCnt = gAppVpCtl.langCnt;
    return true;
}

bool voice_prompt_nvdm_get_support_lang(uint16_t *buffer)
{
    if (giSInited != 1) {
        return false;
    }

    for (int i = 0; i < gAppVpCtl.langCnt; i++) {
        buffer[i] = gAppVpCtl.langInfo[i].langCodec;
    }
    return true;

}


