/* Copyright Statement:
 *
 * (C) 2022  Airoha Technology Corp. All rights reserved.
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
 * @addtogroup apps apps
 * @{
 * @addtogroup apps_dsp0_headset_ref_design dsp0_headset_ref_design
 * @{

@par Overview
  - Application description
    - This application demonstrates headset features based on the chip for the production of audio.

@par Directory contents
  - Source and header files
    - \b inc:                         Common header files.
    - \b src:                         Common files for system start.
  - Project configuration files using XT-XCC
    - \b XT-XCC/feature.mk:           The feature configuration file.
    - \b XT-XCC/feature_"board".mk:   The feature configuration file for a different project.
    - \b XT-XCC/Makefile:             The Makefile.
    - \b XT-XCC/dsp0_flash.lcf:       The linker script.

@par Run the application
  - How to build the headset_ref_design application
    - make command "./build.sh <board> dsp0_headset_ref_design -f=feature_<board>.mk"
      under the SDK/dsp root folder.
    - make command "./build.sh <board> headset_ref_design -fm=feature_<board>.mk -fd0=feature_<board>.mk" 
      under the SDK root folder.
  - How to download the headset_ref_design application
    - Install and use Flash Tool ("sdk_root/tools/PC_tool_Win.zip") to download the
      application binary headset_ref_design.bin.
  - How to run each feature
    - Power on the board and use the smartphone to search for and connect to the BT device.
      Then use BT device as a headset.

@par Troubleshooting
  - Use [IC_CONFIG]_Airoha_Tool_Kit(ATK)_vx.x.x.7z to connect to the board and receive the logs.

@par Contact information
Please report bugs or submit any questions to the Airoha AE Team.
*/
/**
 * @}
 * @}
 * @}
 */
