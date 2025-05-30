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
 
#ifndef __MSDC_CUSTOM_CONFIG_H__
#define __MSDC_CUSTOM_CONFIG_H__


/*****************************************************************************************/
/************************************* Common part ***************************************/
/*****************************************************************************************/

/*Define MSDC0 IO driving capability. 
    0:MSDC_IO_4MA
    1:MSDC_IO_8MA
    2:MSDC_IO_12MA
    3:MSDC_IO_16MA
*/
#define MSDC_DATA_LINE_DRIVING_CAPABILITY (1)



/*************************************************************************************/
/************************************* WIFI part ***************************************/
/*************************************************************************************/

/*Define the MSDC port is used by WIFI.
    0: HAL_SDIO_PORT
*/
#define WIFI_USE_MSDC_PORT_NUMBER (0)

/*Define the bus width.
    1: HAL_SDIO_BUS_WIDTH_1
    2: HAL_SDIO_BUS_WIDTH_4
*/
#define WIFI_MSDC_BUS_WIDTH  (2)

/*Define the bus clock, for bus clock,it should be less than 50000, the unit is kHz.*/
#define WIFI_MSDC_BUS_CLOCK  (22000)   /*Define bus clock to 22MHz.*/





/*************************************************************************************/
/************************************* FS part ***************************************/
/*************************************************************************************/

/*Define the MSDC port is used by FS.
    0: HAL_SD_PORT
*/
#define FS_USE_MSDC_PORT_NUMBER (0)

/*Define the bus width.
    1: HAL_SD_BUS_WIDTH_1
    2: HAL_SD_BUS_WIDTH_4
*/
#define FS_MSDC_BUS_WIDTH  (2)

/*  Define the bus clock, for bus clock,it should be less than or equal to MSDC source clock, the unit is kHz. */
/*  But bus clock does not represent the speed of MSDC peripherals(EMMC/SD/SDIO), */
/*  In actual use, the reading and writing speed of the peripheral will be lower than the bus clock.  */
/*  If the vcore voltage changes, the MSDC source clock should change accordingly.  */
/*  MSDC_CLK      vcore_voltage      CPU_CLK  */
/*    52MHz            >=0.8V         >=104MHz  */
/*    26MHz       0.73v or 0.75V        52MHz  */
#define FS_MSDC_BUS_CLOCK  (52000)   /* Define bus clock to 52MHz. */




/********************************************************************************************/
/************************************* Mass Storage part ***************************************/
/********************************************************************************************/

/*Define the MSDC port is used by Mass Storage. It is always equal to FS_USE_MSDC_PORT_NUMBER.
    0: HAL_SD_PORT
*/
#define MASS_STORAGE_USE_MSDC_PORT_NUMBER (0)

/*Define the bus width.
    1: HAL_SD_BUS_WIDTH_1
    2: HAL_SD_BUS_WIDTH_4
*/
#define MASS_STORAGE_MSDC_BUS_WIDTH  (2)

/*  Define the bus clock, for bus clock,it should be less than or equal to MSDC source clock, the unit is kHz. */
/*  But bus clock does not represent the speed of MSDC peripherals(EMMC/SD/SDIO), */
/*  In actual use, the reading and writing speed of the peripheral will be lower than the bus clock.  */
/*  If the vcore voltage changes, the MSDC source clock should change accordingly.  */
/*  MSDC_CLK      vcore_voltage      CPU_CLK  */
/*    52MHz            >=0.8V         >=104MHz  */
/*    26MHz       0.73v or 0.75V        52MHz  */
#define MASS_STORAGE_MSDC_BUS_CLOCK  (52000)   /* Define bus clock to 52MHz. */


#endif

