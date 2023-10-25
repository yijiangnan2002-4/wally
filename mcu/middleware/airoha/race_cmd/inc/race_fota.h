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


#ifndef __RACE_FOTA_H__
#define __RACE_FOTA_H__


#include "race_cmd_feature.h"
#ifdef RACE_FOTA_CMD_ENABLE
#include "race_cmd.h"


/**
 * @addtogroup Race_CMD_Group Race CMD
 * @{
 * @addtogroup  Race_FOTA Race FOTA
 * @{
 * Race FOTA is a sub-module of the race command module. It provides the basic
 * download flow of FOTA over SPP, BLE and so on.
 */


#ifdef __cplusplus
extern "C"
{
#endif


////////////////////////////////////////////////////////////////////////////////
// CONSTANT DEFINITIONS ////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
/**
 * @brief This macro defines the default transmit interval in milliseconds for FOTA using SPP.
 */
#define RACE_FOTA_DEFAULT_SPP_TRANSMIT_INTERVAL_IN_MS  (200)

/**
 * @brief This macro defines the default transmit interval in milliseconds for FOTA using BLE.
 */
#define RACE_FOTA_DEFAULT_BLE_TRANSMIT_INTERVAL_IN_MS  (350)


////////////////////////////////////////////////////////////////////////////////
// TYPE DEFINITIONS ////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
/**
 * @brief This enum defines the FOTA mode for race command FOTA.
 */
typedef enum {
    RACE_FOTA_MODE_BACKGROUND = 0,  /* FOTA background mode: FOTA download continues even if a call or music is on-going. FOTA download speed is low and will not be affected by call or music. */
    RACE_FOTA_MODE_ACTIVE,          /* FOTA active mode: A call has greater priority than the FOTA download which has priority over music. For example, when there is an on-going call, the FOTA download stops. When FOTA download starts, music playback will be paused. The download speed for FOTA is high. */
    RACE_FOTA_MODE_ADAPTIVE,        /* FOTA adaptive mode: FOTA download continues even if an ongoing call or music playback. However the download speed will decrease when there is call or music on-going. The FOTA download speed increases when there is neither call nor music. */

    RACE_FOTA_MODE_MAX              /* The maximum value of this enum. It must not exceed 0x0F. */
} race_fota_mode_enum;

////////////////////////////////////////////////////////////////////////////////
// FUNCTION DECLARATIONS /////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
/**
 * @brief Function for cancelling FOTA. There are race_fota and Bisto_fota. This
 * API can only cancel race_fota.
 * @return RACE_ERRCODE_SUCCESS succeed; otherwise, fail.
 */
RACE_ERRCODE race_fota_cancel(void);


/**
 * @brief Function for querying whether FOTA is running.
 * @param[in] is_race_fota    There are race_fota and Bisto_fota. TRUE query
 * race_fota; FALSE query Bisto_fota.
 * @return TRUE FOTA is running; FALSE FOTA is not running.
 */
bool race_fota_is_running(bool is_race_fota);


/**
 * @brief Function for checking if the current FOTA mode is the active mode or
 * not. There are the active mode and the background mode.
 * @return TRUE It is the active mode; FALSE It is the background mode. The
 * return value is valid only when race fota is running.
 */
bool race_fota_is_active_mode(void);


/**
 * @brief Function for getting the current FOTA mode. Please be aware that the return value is valid only when race cmd FOTA is running.
 * @return the current FOTA mode.
 */
race_fota_mode_enum race_fota_get_fota_mode(void);


/**
 * @brief Function for setting the transmit interval in ms between two continous race cmds
 * before FOTA starts.
 * @param[in] spp_transmit_interval The transmit interval for FOTA using SPP.
 * @param[in] ble_transmit_interval The transmit interval for FOTA using BLE.
 * @return RACE_ERRCODE_SUCCESS succeed; otherwise, fail.
 */
RACE_ERRCODE race_fota_set_transmit_interval(uint16_t spp_transmit_interval, uint16_t ble_transmit_interval);

/**
 * @brief The function is used to check whether Race FOTA is busy or not.
 * Generally, Race FOTA is considered to be in busy state when there is data in the FOTA partition.
 * @return true means Race FOTA is busy, false means it is idle.
 */
bool race_fota_is_busy(void);

#ifdef __cplusplus
}
#endif

/**
 * @}
 * @}
 */

#endif /* RACE_FOTA_CMD_ENABLE */
#endif /* __RACE_FOTA_H__ */

