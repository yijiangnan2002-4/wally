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

#include "hal_flash.h"

#ifdef HAL_FLASH_MODULE_ENABLED
#include "memory_attribute.h"
#include "hal_flash_custom_memorydevice.h"
#include "hal_flash_opt.h"
#include "hal_flash_general_types.h"
#include "hal_flash_mtd_internal.h"
#include "air_chip.h"
#include "hal_flash_sf.h"
#include "hal_flash_mtd_sf_dal.h"
#include "hal_flash_sfi_hw.h"
#include "hal_gpt.h"

#define SFI_IsQPIMode()      (SFC->RW_SF_DIRECT_CTL & SFI_QPI_EN)
#define SFI_IsQPIMode_CS1()  (SFC->RW_SF_DIRECT_CTL2 & SFI_QPI_EN)


//Extern APIs
extern bool CMEM_CheckValidDeviceID(uint8_t *id);
extern bool NOR_FLASH_BUSY;
extern bool NOR_FLASH_SUSPENDED;
extern uint8_t nor_id[4];


//-----------------------------------------------------------------------------
/*!
  @brief
    Reverse byte order in a DWORD
*/
ATTR_TEXT_IN_SYSRAM uint32_t SFI_ReverseByteOrder(uint32_t num)
{
    unsigned int ret = 0;
    //remove the highest byte of address 0x18******
    num = num & (~HAL_FLASH_BASE_ADDRESS);
    ret |= ((num >> SFC_GENERIC_24_BIT_OFFSET) & SFC_GENERIC_0x000000FF_MASK);
    ret |= ((num >> SFC_GENERIC_8_BIT_OFFSET)  & SFC_GENERIC_0x0000FF00_MASK);
    ret |= ((num << SFC_GENERIC_8_BIT_OFFSET)  & SFC_GENERIC_0x00FF0000_MASK);
    ret |= ((num << SFC_GENERIC_24_BIT_OFFSET) & SFC_GENERIC_0xFF000000_MASK);

    return ret;
}

//-----------------------------------------------------------------------------
/*!
  @brief
    Enter macro mode, and set SFI_MAC_SIO_SEL if QPI mode is enabled.
*/
ATTR_TEXT_IN_TCM void SFI_MacEnable(const uint16_t CS)
{
    uint32_t val;
    //Mask SFC AHB Channel 2 and 3 during SFC Mac mode/ Flash busy
    SFI_MaskAhbChannel(1);

    val = SFC->RW_SF_MAC_CTL;
    if (CS == 0x0) {
        if (SFI_IsQPIMode()) {
            val |= SFI_MAC_SIO_SEL;
        }
    }

    val |= SFI_MAC_EN;

    //Wait SFC idle before issue Macro command
#if !defined(__UBL__) //Wait until SFC AHB Channel 2 become IDLE before enter Macro mode
    while (!(SFC->RW_SF_MISC_CTL3 & SFI_CH2_TRANS_IDLE));
    while (!(SFC->RW_SF_MISC_CTL & SFI_REQ_IDLE));
#endif

    SFC->RW_SF_MAC_CTL = val;

    /* Force resetting of the FIFO pointer to
     * avoid reading the wrong device status
     * under certain low-voltage conditions.
     */
    *((volatile uint32_t *)(0x42030144)) |= (uint32_t)(1U << 16);
    *((volatile uint32_t *)(0x40040070)) |= (uint32_t)(1U << 31);
    *((volatile uint32_t *)(0x42030144)) &= ~(uint32_t)(1U << 16);

    *((volatile uint32_t *)(0x42030144)) |= (uint32_t)(1U << 16);
    *((volatile uint32_t *)(0x40040070)) &= ~(uint32_t)(1U << 31);
    *((volatile uint32_t *)(0x42030144)) &= ~(uint32_t)(1U << 16);
}

//-----------------------------------------------------------------------------
/*!
  @brief
    Leaves macro mode, and returns to direct read mode
  @remarks
*/
ATTR_TEXT_IN_TCM void SFI_MacLeave(const uint16_t CS)
{
    uint32_t val;

    // clear SF_TRIG and leave MACRO mode
    val = SFC->RW_SF_MAC_CTL;
    val &= ~(SFI_TRIG | SFI_MAC_SIO_SEL | SFI_MAC_SEL);
    SFC->RW_SF_MAC_CTL = val;

    // SW workaround for MTxxxx F2S write early response and Slave engine latency
    while ((SFC->RW_SF_MAC_CTL & SFI_WIP_READY));

    //Disable SFI_MAC_EN must after polling SFI_WIP_READY for MT6250 HW DCM
    //Because once SFI_MAC_EN is disabled, Idle bit of SFC will be set, HW DCM may gate SFCLK.
    //Then SFI_WIP_READAY will keep high because it depends on SFCLK.
    val = SFC->RW_SF_MAC_CTL;
    val &= ~(SFI_MAC_EN);
    SFC->RW_SF_MAC_CTL = val;

    // SW workaround for MTxxxx F2S write early response and Slave engine latency
    while ((SFC->RW_SF_MAC_CTL & SFI_MAC_EN));

#if !defined(__UBL__)//UnMask SFC AHB Channel 2 and 3 when Flash is not busy
    /*Notice: This variable has not been initialized during ScanSerialFlashBlcoks() in Init stage*/
    /*SFI channel2 and channel3 may be locked and unable to use in maui if NOR_FLASH_BUSY != false*/
    /*Solution: Always Unmask SFI channel2 and channel3 in the end of ScanSerialFlashBlocks()*/
    if ((NOR_FLASH_BUSY == false)) {
        SFI_MaskAhbChannel(0);
    }
#endif

}

//-----------------------------------------------------------------------------
/*!
  @brief
    Send commands placed in GPRAM (in macro mode)
*/
ATTR_TEXT_IN_TCM uint32_t SFI_MacTrigger(const uint16_t CS)
{
    uint32_t  val = 0;
    uint32_t  misc_ctl = 0;

    val = SFC->RW_SF_MAC_CTL;

    // trigger SFC
    val |= (SFI_TRIG | SFI_MAC_EN);
    if (CS == 0x1) {  //for CS1
        val |= SFI_MAC_SEL;
    }

    SFC->RW_SF_MAC_CTL = val;

    // wait for SFC ready
    while (!(SFC->RW_SF_MAC_CTL & SFI_WIP_READY));
    while ((SFC->RW_SF_MAC_CTL & SFI_WIP));
    return misc_ctl;
}

//-----------------------------------------------------------------------------
/*!
  @brief
    Set trigger and send GPRAM data toward serial Flash
    Leave macro mode when done
*/
ATTR_TEXT_IN_TCM void SFI_MacWaitReady(const uint16_t CS)
{
    SFI_MacTrigger(CS);
    SFI_MacLeave(CS);
}

//-----------------------------------------------------------------------------
/*!
  @brief
    Issue generic command to serial Flash (max: 4 bytes)

  @param[in] cmd The command to be sent, the first shift out byte is MSB.
  @remarks This function is only called by bootloader.
           Other callers must be aware of interrupts during the MAC mode
*/
ATTR_TEXT_IN_TCM void SFI_Dev_Command(const uint16_t CS, const uint32_t cmd)
{
    uint32_t cmdi, cmdo;
    uint8_t  len;

    for (cmdi = cmd, len = 0, cmdo = 0; cmdi != 0 ; cmdi = cmdi >> SFC_GENERIC_8_BIT_OFFSET, len++) {
        cmdo = (cmdo << SFC_GENERIC_8_BIT_OFFSET) | (cmdi & SFC_GENERIC_0xFF_MASK);
    }

    if (len == 0) {
        return;
    }

    SFI_MacEnable(CS);
    SFC_GPRAM->RW_SF_GPRAM_DATA = cmdo;
    SFC->RW_SF_MAC_OUTL = len;
    SFC->RW_SF_MAC_INL = 0;
    SFI_MacWaitReady(CS);

    return;
}

//-----------------------------------------------------------------------------
/*!
  @brief
    Send command and address to serial Flash

  @param[in] cmd The command to be sent
  @param[in] address The address to be sent, followed by cmd.
  @remarks This function is only called by bootloader.
           Other callers must be aware of interrupts during the MAC mode
*/
ATTR_TEXT_IN_SYSRAM void SFI_Dev_CommandAddress(const uint16_t CS, const uint8_t cmd, const uint32_t address, const uint32_t address_bytes)
{
    uint32_t cmd1, cmd2;

    cmd2 = SFI_ReverseByteOrder(address);

    if (address_bytes == 3) {
        cmd1 = (cmd2 & SFC_GENERIC_0xFFFFFF00_MASK ) | cmd;
    } else {
        cmd1 = (cmd2 << SFC_GENERIC_8_BIT_OFFSET) | cmd;
        cmd2 = cmd2 >> SFC_GENERIC_24_BIT_OFFSET;
    }

    SFC_GPRAM->RW_SF_GPRAM_DATA = cmd1;
    SFC_GPRAM->RW_SF_GPRAM_DATA_OF_4 = cmd2;
    SFC->RW_SF_MAC_OUTL = address_bytes + 1;
    SFC->RW_SF_MAC_INL = 0;

    SFI_MacEnable(CS);
    SFI_MacWaitReady(CS);
    return;
}

//-----------------------------------------------------------------------------
/*!
  @brief
    GPRAM access: Write 1 byte command and 3 bytes address

  @param[in] cmd The command to be sent
  @param[in] address The address followed by the command.
*/
ATTR_TEXT_IN_SYSRAM uint32_t SFI_GPRAM_Write_C1A3(const uint32_t cmd, const uint32_t address)
{
    return ((SFI_ReverseByteOrder(address) & SFC_GENERIC_0xFFFFFF00_MASK ) | (cmd & SFC_GENERIC_0xFF_MASK ));
}

//-----------------------------------------------------------------------------
/*!
  @brief
    GPRAM access: Write 1 byte command, 4 bytes address, and 3 bytes of data

  @param[in] cmd The command to be sent
  @param[in] address The address followed by the command.
  @param[in] p_data The pointer-pointer to the data buffer.
  @param[in] length The pointer to the length of the data buffer.
  @remarks This function fills in the 2nd GPRAM register with
     LSB address, and 3 bytes (at most) of the data buffer. It also
     move the given pointer to next buffer address, and subtract the length of
     the data buffer for next GPRAM buffer copy.
*/
#if defined(__SFI_4BYTES_ADDRESS__)
ATTR_TEXT_IN_SYSRAM uint32_t SFI_GPRAM_Write_C1A4(const uint32_t cmd, const uint32_t address, uint8_t **p_data, uint32_t *length)
{
    sf_uint    tmp;
    uint32_t cmd1, cmd2;
    uint32_t i;

    cmd2 = SFI_ReverseByteOrder(address);
    cmd1 = cmd2 << SFC_GENERIC_8_BIT_OFFSET | (cmd & SFC_GENERIC_0xFF_MASK );
    cmd2 = cmd2 >> SFC_GENERIC_24_BIT_OFFSET;

    tmp.u8[0] = cmd2 & SFC_GENERIC_0x000000FF_MASK;
    for (i = 1; i < 4 && i <((*length)+1); i++) {
        tmp.u8[i] = **p_data;
        (*p_data)++;
    }
    cmd2 = tmp.u32;
    *length = (*length > 3) ? (*length - 3) : 0;
    SFC_GPRAM->RW_SF_GPRAM_DATA_OF_4 = cmd2;
    return cmd1;
}
#endif

//-----------------------------------------------------------------------------
/*!
  @brief
    GPRAM access: Write data to GPRAM

  @param[in] gpram_offset The offset of the GPRAM in terms of byte, it must be
             multiple of four.
  @param[in] buff The pointer to the data buffer
  @param[in] length The length of the data buffer.
  @remarks The given gpram offset must be 4 bytes aligned, and the function will
           copy the data buffer to the GPRAM WORD/HALF-WORD/BYTE wise
           according to the starting address of the buffer.
*/
ATTR_TEXT_IN_SYSRAM void SFI_GPRAM_Write(uint32_t gpram_offset, void *buff, int32_t length)
{
    uint32_t i;
    uint8_t *p_data = (uint8_t *)buff;
    sf_uint tmp;

    gpram_offset += RW_SFI_GPRAM_DATA;

    // source address is 4-byte aligned, and gpram_offset is 4-bytes aligned, too
    if (0 == ((uint32_t)buff & 0x3))    {
        for (; length > 0; gpram_offset += 4, p_data += 4, length -= 4) {
            SFI_WriteReg32(gpram_offset, *(uint32_t *)p_data);
        }
    }
    // source address is 2-byte aligned
    else if (0 == ((uint32_t)p_data & 0x1))     {
        for (; length > 0; gpram_offset += 4, p_data += 4, length -= 4) {
            tmp.u16[0] = *(uint16_t *)p_data;
            tmp.u16[1] = *((uint16_t *)p_data + 1);
            SFI_WriteReg32(gpram_offset, tmp.u32); // save local_data to SFI GPRAM
        }
    }
    // source data is NOT 2-byte aligned
    else {
        for (; length > 0; gpram_offset += 4, length -= 4) {
            for (i = 0; i < 4 && i < length; i++) {
                tmp.u8[i] = *p_data++;
            }
            SFI_WriteReg32(gpram_offset, tmp.u32); // save local_data to SFI GPRAM
        }
    }
}


//-----------------------------------------------------------------------------
/*!
  @brief
    Issue generic command to serial Flash, and read results.

  @param[in] cmd Pointer to the commands that to be sent
  @param[out] data Pointer to the data buffer that to be stored
  @param[in] outl Length of commands (in bytes)
  @param[in] intl Length of read data

  @remarks This function shall only be invoked in bootloader.
           Other callers must be aware of interrupts during the MAC mode
*/
ATTR_TEXT_IN_SYSRAM void SFI_Dev_Command_Ext(const uint16_t CS, const uint8_t *cmd, uint8_t *data, const uint16_t outl, const uint16_t inl)
{
    uint32_t      tmp;
    uint32_t      i, j;
    uint8_t       *p_data, *p_tmp;

    p_tmp = (uint8_t *)(&tmp);

    for (i = 0, p_data = ((uint8_t *)RW_SFI_GPRAM_DATA); i < outl; p_data += 4)   {
        for (j = 0, tmp = 0; i < outl && j < 4; i++, j++) {
            p_tmp[j] = cmd[i];
        }
        SFI_WriteReg32(p_data, tmp);
    }
    SFC->RW_SF_MAC_OUTL = outl;
    SFC->RW_SF_MAC_INL = inl;
    SFI_MacEnable(CS);
    SFI_MacWaitReady(CS);

    for (i = 0, p_data = ((uint8_t *)RW_SFI_GPRAM_DATA + outl); i < inl; ++i, ++data, ++p_data) {
        *data = SFI_ReadReg8(p_data);
    }
    return;
}


/*!
  @brief  it is used in  BL: custom_setSFIExt --> SFI_Dev_Command_List 
*/
ATTR_TEXT_IN_RAM static void write_en(const uint16_t CS, const uint32_t cmd, uint8_t outlen)
{
    uint32_t savedMask;

    savedMask = SaveAndSetIRQMask();
    SFI_MacEnable(CS);
    SFC_GPRAM->RW_SF_GPRAM_DATA = cmd;
    SFC->RW_SF_MAC_OUTL = outlen;
    SFC->RW_SF_MAC_INL = 0;
    SFI_MacWaitReady(CS);
    RestoreIRQMask(savedMask);
    return;
}



/*!
  @brief  it is used in  BL: custom_setSFIExt --> SFI_Dev_Command_List 
*/
ATTR_TEXT_IN_RAM static uint32_t ReadStatus(const uint16_t CS, uint8_t cmd)
{
    uint32_t savedMask;
    uint32_t result;

    savedMask = SaveAndSetIRQMask();
    SFI_MacEnable(CS);
    SFC_GPRAM->RW_SF_GPRAM_DATA = cmd;
    SFC->RW_SF_MAC_OUTL = 1;
    SFC->RW_SF_MAC_INL = 1;
    SFI_MacWaitReady(CS);
    result = (unsigned char)((SFC_GPRAM->RW_SF_GPRAM_DATA) >> SFC_GENERIC_8_BIT_OFFSET);
    RestoreIRQMask(savedMask);
    return result;
}


//-----------------------------------------------------------------------------
/*!
  @brief
    Issue multiple generic commands to serial Flash

  @param[in] cmdlist Pointer to the list of the commands that to be sent to serial Flash.

  @remarks 1. This function is only called by bootloader.
              Other callers must be aware of interrupts during the MAC mode
           2. The Command List Format:
                cmd 1- {QPI/SPI, length in bytes, byte1, byte2, ... },
                cmd 2- {QPI/SPI, length in bytes, byte1, byte2, ... },
                ...
                end  - {SF_UNDEF}
              Example:
                {SPI, 1, 0x38}, {QPI, 2, 0xC0, 0x02}, {SF_UNDEF}
           3. If the first command is 0x05, the second byte will be the status polling bit mask.
              The function will wait till (status & mask) euqals zero.
              Example: {SPI, 1, 0x05, 0x01} - polling status register till bit0 becomes zero.

*/
ATTR_TEXT_IN_SYSRAM void SFI_Dev_Command_List(const uint16_t CS, const uint8_t *cmdlist)
{
    uint32_t i = 0;
    uint32_t val;

    if (cmdlist == NULL) {
        return;
    }

    val = SFI_DevMode; // denotes the orignal mode

    while (1) {
        switch (cmdlist[i]) {
            case SPI:
            case SPIQ:
                SFI_SetDevMode(CS, 0);
                break;   // Switch to SPI mode
            case QPI:
                SFI_SetDevMode(CS, SFI_QPI_EN);
                break;   // Switch to QPI mode
            default:
                SFI_SetDevMode(CS, val);
                return;  // Resume to orignal mode and returns
        }
        // When Read Status (05h) is encountered
        if (cmdlist[i + 2] == SF_CMD_READ_SR) {
            uint8_t status;
            // The second byte is the bit mask, polling till zero.
            do {
                SFI_Dev_Command_Ext(CS, &cmdlist[i + 2], &status, 1, 1);
            } while (status & cmdlist[i + 3]);
        }
        // Commands other than 05h
        else {
            if ((((nor_id[0] == 0xEF) && (nor_id[1] == 0x60) && (nor_id[2] == 0x17)) ||
                 ((nor_id[0] == 0xEF) && (nor_id[1] == 0x80) && (nor_id[2] == 0x17)))) {
                if (cmdlist[i + 2] == SF_CMD_WRITE_SR) {
                    uint8_t status;
                    //uint8_t cmd[0];
                    uint8_t data[6];
                    write_en(CS, SF_CMD_WREN, 1);
                    status = ReadStatus(CS,SF_CMD_READ_SR);
                    if ((status & SF_WINBOND_PROTECT_TOP_128KB_W25Q64JW) == SF_WINBOND_PROTECT_TOP_128KB_W25Q64JW) {
                        uint32_t j;
                        for (j =0; j < cmdlist[i + 1];j++) {
                            data[j] = cmdlist[i + 3 + j];
                        }
                        data[0] |= SF_WINBOND_PROTECT_TOP_128KB_W25Q64JW;
                        SFI_Dev_Command_Ext(CS, &data[0], NULL, cmdlist[i + 1], 0); // send command
                    } else {
                        SFI_Dev_Command_Ext(CS, &cmdlist[i + 2], NULL, cmdlist[i + 1], 0); // send command
                    }
                } else {
                    SFI_Dev_Command_Ext(CS, &cmdlist[i + 2], NULL, cmdlist[i + 1], 0); // send command
                }
            }else {
                SFI_Dev_Command_Ext(CS, &cmdlist[i + 2], NULL, cmdlist[i + 1], 0); // send command
            }
        }

        i += (cmdlist[i + 1] + 2);
    }
}

/*!
  @brief
    Query this function to get SF unique ID (not gurantee unique because Vendor limitation)
  @param[in]
    buffer: Read 8 byte unique ID buffer
  @retval
    true: Read unique ID success
    false: Read uinque ID fail
*/
ATTR_TEXT_IN_SYSRAM bool SFI_Dev_GetUniqueID(uint8_t *buffer)
{
    int i = 0;
    for (i = 0; i < 8; i++) {
        buffer[i] = 0xFF;
    }
    return true;
}


extern SF_MTD_Data EntireDiskMtdData;
#define DPDReadyTime (100) //Enter and Leave Deep Power Down mode need 100us at most

ATTR_ZIDATA_IN_TCM uint32_t SFI_MAC_CTL_BACKUP;
ATTR_ZIDATA_IN_TCM uint32_t SFI_DIRECT_CTL_BACKUP;
ATTR_ZIDATA_IN_TCM uint32_t SFI_MISC_CTL_BACKUP;
ATTR_ZIDATA_IN_TCM uint32_t SFI_MISC_CTL2_BACKUP;
ATTR_ZIDATA_IN_TCM uint32_t SFI_DLY_CTL_3_BACKUP;

/*!
  @brief  it is used in  BL: custom_setSFIExt --> SFI_Dev_Command_List 
*/
ATTR_TEXT_IN_SYSRAM void SF_DAL_DEV_Enter_DPD(void)
{
    /* SFC registers is not in AO domain, will be reset in sleep mode. Back up them here*/
    SFI_MAC_CTL_BACKUP = SFC->RW_SF_MAC_CTL;
    SFI_DIRECT_CTL_BACKUP = SFC->RW_SF_DIRECT_CTL;
    SFI_MISC_CTL_BACKUP = SFC->RW_SF_MISC_CTL;
    SFI_MISC_CTL2_BACKUP = SFC->RW_SF_MISC_CTL2;
    SFI_DLY_CTL_3_BACKUP = SFC->RW_SF_DLY_CTL3;

    //SFI_Dev_Command(0, 0xB9);
    //hal_gpt_delay_us(DPDReadyTime);
}

ATTR_TEXT_IN_TCM void SF_DAL_DEV_Leave_DPD(void)
{
    /* Restore SFC registers before issue any cmd */
    SFC->RW_SF_MAC_CTL = SFI_MAC_CTL_BACKUP;
    SFC->RW_SF_DIRECT_CTL = SFI_DIRECT_CTL_BACKUP;
    SFC->RW_SF_MISC_CTL = SFI_MISC_CTL_BACKUP;
    SFC->RW_SF_MISC_CTL2 = SFI_MISC_CTL2_BACKUP;
    SFC->RW_SF_DLY_CTL3 = SFI_DLY_CTL_3_BACKUP;

    /* Issue leave DPD Command */
    //SFI_Dev_Command(0, NOR_FLASH_LeaveDPDCmd);
    //hal_gpt_delay_us(DPDReadyTime);
}

#else  // !HAL_FLASH_MODULE_ENABLED

#include "memory_attribute.h"
bool SFI_Dev_GetUniqueID(uint8_t *buffer)
{
    return true;
}

ATTR_TEXT_IN_TCM void SF_DAL_DEV_Enter_DPD(void)
{
}

ATTR_TEXT_IN_TCM void SF_DAL_DEV_Leave_DPD(void)
{
}

#endif //#ifdef HAL_FLASH_MODULE_ENABLED


