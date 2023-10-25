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

#ifndef __HAL_MPU_INTERNAL_H__
#define __HAL_MPU_INTERNAL_H__
#include "hal_mpu.h"

#ifdef HAL_MPU_MODULE_ENABLED


#ifdef __cplusplus
extern "C" {
#endif

typedef union {
    struct {
        uint32_t ENABLE: 1;                     /**< Enables the MPU */
        uint32_t HFNMIENA: 1;                   /**< Enables the operation of MPU during hard fault, NMI, and FAULTMASK handlers */
        uint32_t PRIVDEFENA: 1;                 /**< Enables privileged software access to the default memory map */
        uint32_t _reserved0: 29;                /**< Reserved */
    } b;
    uint32_t w;
} MPU_CTRL_Type;

typedef uint32_t MPU_REGION_EN_Type;

/*---*------shareability  --------*-----Access permissions -------------------------*-----Execute Never -----*/
/*---*  SH[4:3]  Normal memory    *       AP[2;1]           Aceess                  *          XN  Executable*/
/*---*----------------------------*-------------------------------------------------*------------------------*/
/*---*  00       Non-shareable    *       00       Read/write,privileged code only  *          0       Yes   */
/*---*  01       Reserved         *       01       Read/write,any privileged level  *          1       No    */
/*---*  10       outer Shareable  *       10       Read-only,privileged code only   *                        */
/*---*  11       Inner Shareable  *       11       Read-only,any privilege level    *                        */

typedef union {
    struct {
        uint32_t XN:     1 ;                   /**< Execute Never. Defines whether code can be executed from this region. */
        uint32_t AP:     2 ;                   /**< Access permissions. */
        uint32_t SH:     2 ;                   /**< Shareability. Defines the shareability domain of this region for Normal memory. */
        uint32_t ADDR:   27;                   /**< Region base address field */
    } b;
    uint32_t w;
} MPU_RBAR_Type;

typedef union {
    struct {
        uint32_t EN:         1  ;                /**< Enable. Region enable. 0-Region disable, 1-Region enable.*/
        uint32_t ATTRINDX :  3  ;                /**< Attribute index. Associates a set of attributes in the MPU_MAIR0 and MPU_MAIR1 fields.*/
        uint32_t Reserved0:  1  ;                /**< Reserved bit. */
        uint32_t LIMIT    :  27 ;                /**< Limit address. The upper inclusive limit of the selected MPU memory region
                                                      This value is postfixed with 0x1F to provide the limit address to be checked against.*/
    } b;
    uint32_t w;
} MPU_RLAR_Type;                                /**< MPU Region Limit Address Register.
                                                     The MPU_RLAR defines the limit address of the MPU region selected by the MPU_RNR.*/


/*---*******************************************************------*/
/*   **For Normal Memory: When MAIR_ATTR[7:4] is not 0000:**      */
/*---*******************************************************------*/
/*---|Attr<n>[7:4]--*----Attributes------------------------*--*-Attr<n>[3:0]-----*--- Attributes---                      */
/*---|--------------*--------------------------------------*--*------------------*---------------------------------------*/
/*---|0000          *    see Device memory                 *--*   0000           *    UNPREDICTABLE                      */
/*---|00RW *        *    Outer Write-through transient     *--*   00RW *         *    inner Write-through transient      */
/*---|0100          *    Outer Non-cacheable               *--*   0100           *    inner Non-cacheable                */
/*---|01RW *        *    Outer Write-back transient        *--*   01RW           *    inner Write-back transient         */
/*---|10RW          *    Outer Write-through non-transient *--*   10RW           *    inner Write-through non-transient  */
/*---|11RW          *    Outer Write-back non-transient    *--*   11RW           *    inner Write-back non-transient     */

/*---*******************************************************------*/
/*   **For Device Memory: When MAIR_ATTR[7:4] is 0000:    **      */
/*---*******************************************************------*/
/*---*-Attr<n>[7:0]-----*--- Attributes---------------------------*/
/*---*------------------*-----------------------------------------*/
/*---*  00000000        *    Device-nGnRnE memory                 */
/*---*  00000100        *    Device-nGnRE memory                  */
/*---*  00001000        *    Device-nGRE memory                   */
/*---*  00001100        *    Device-GRE memory                    */
/*---*  0000XXRW        *    UNPREDICTABLE (when RW != 00 )       */

typedef union {
    struct {
        uint32_t ATTR0:      8  ;               /**< Memory attribute encoding for MPU regions with an AttrIndex of 0.*/
        uint32_t ATTR1:      8  ;               /**< Memory attribute encoding for MPU regions with an AttrIndex of 1.*/
        uint32_t ATTR2:      8  ;               /**< Memory attribute encoding for MPU regions with an AttrIndex of 2.*/
        uint32_t ATTR3:      8  ;               /**< Memory attribute encoding for MPU regions with an AttrIndex of 3.*/
    } b;
    uint32_t w;
} MPU_MAIR0_Type;                               /**< MPU Memory Attribute Indirection Registers 0.*/

typedef union {
    struct {
        uint32_t ATTR4:      8  ;               /**< Memory attribute encoding for MPU regions with an AttrIndex of 4.*/
        uint32_t ATTR5:      8  ;               /**< Memory attribute encoding for MPU regions with an AttrIndex of 5.*/
        uint32_t ATTR6:      8  ;               /**< Memory attribute encoding for MPU regions with an AttrIndex of 6.*/
        uint32_t ATTR7:      8  ;               /**< Memory attribute encoding for MPU regions with an AttrIndex of 7.*/
    } b;
    uint32_t w;
} MPU_MAIR1_Type;                               /**< MPU Memory Attribute Indirection Registers 1.*/

typedef struct {
    MPU_RBAR_Type mpu_rbar;
    MPU_RLAR_Type mpu_rlar;
} MPU_ENTRY_Type;


extern MPU_CTRL_Type g_mpu_ctrl;
/*bit indicate which region is protected by MPU, debug used flag*/
extern MPU_REGION_EN_Type g_mpu_region_en;
extern MPU_ENTRY_Type g_mpu_entry[HAL_MPU_REGION_MAX];

/* On cortex-m33, the memory attribute must be set,
 * otherwise the default is Device memory.
 * And ARMv8M does not allow unaligned access to Devicve Memory.
 */
extern MPU_MAIR0_Type g_mpu_mair0;
extern MPU_MAIR1_Type g_mpu_mair1;

/* Save MPU status before entering deepsleep */
void mpu_status_save(void);

/* Restore MPU status after leaving deepsleep */
void mpu_status_restore(void);


#ifdef __cplusplus
}
#endif

#endif /* HAL_MPU_MODULE_ENABLED */
#endif /* __HAL_MPU_INTERNAL_H__ */

