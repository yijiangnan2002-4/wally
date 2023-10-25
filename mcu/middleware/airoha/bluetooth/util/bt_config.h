/* Copyright Statement:
 *
 * (C) 2005-2016  MediaTek Inc. All rights reserved.
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. ("MediaTek") and/or its licensors.
 * Without the prior written permission of MediaTek and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 * You may only use, reproduce, modify, or distribute (as applicable) MediaTek Software
 * if you have agreed to and been bound by the applicable license agreement with
 * MediaTek ("License Agreement") and been granted explicit permission to do so within
 * the License Agreement ("Permitted User").  If you are not a Permitted User,
 * please cease any access or use of MediaTek Software immediately.
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT MEDIATEK SOFTWARE RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES
 * ARE PROVIDED TO RECEIVER ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 */
/* MediaTek restricted information */

#ifndef __BT_CONFIG_H__
#define __BT_CONFIG_H__

#if (!defined(BT_NO_DEBUG) && !defined(BT_DEBUG))
#define BT_DEBUG
#endif

/****** Debug Feature ******/
#ifdef BT_DEBUG
/* Note: Please enable BT_BQB in project config at the same time */
#define BT_BQB

//#define BT_DEBUG_NO_MM
//#define BT_DEBUG_NO_TIMER
//#define BT_DEBUG_NO_HCI
//#define BT_DEBUG_NO_GAP

//#define BT_DEBUG_NO_I
//#define BT_DEBUG_NO_D

#define BT_OOM_TEST_SUPPORT

//#define BT_RX_LOOPBACK

//#define BT_USE_SW_ECC
#endif

/****** Advanced Feature ******/
#ifdef __BT_HB_DUO__
#define __MTK_AWS_MCE_ENABLE__
#endif

//#define power_p
/* Enable for handling disconnect 0x3e problem */
#define BT_USE_DISCONNECT_0X3E_ENHANCE

/****** Timeout ******/

/*
 * 5s for command complete or command status.
 * Timeout will cause firmware assert.
 */
#define BT_HCI_CMD_TIMEOUT      8000  //Change to 8s to avoid unnessary cmd timeout.

/*
 * The Response Timeout eXpired (RTX) timer: the minimum initial value is 1s
 * and the maximum initial value is 60s.
 */
#define BT_L2CAP_CMD_TIMEOUT    60000
#define BT_SMP_DEFAULT_TIMEOUT  30000
#define BT_ATT_DEFAULT_TIMEOUT  30000
#ifdef BT_USE_DISCONNECT_0X3E_ENHANCE
#define BT_GAP_LE_DISCONNECT_0X3E_ENHANCEMENT_TIMEOUT  14000
#endif

#endif /*__BT_CONFIG_H__*/

