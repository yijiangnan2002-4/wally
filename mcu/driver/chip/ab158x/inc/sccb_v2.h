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


//#include "drv_features.h"

//#if (defined(DRV_I2C_25_SERIES))
//#if (defined(MT6225))

#if defined(__USBDL_BOOTROM__)
//#include "br_MTK_BB.h"
#else
//#include "sal_intr.h"
//#include "drv_comm.h"
//#include "MT6276.h"
#endif

#include "hal_platform.h"

#define SCCB_MAXIMUM_TRANSACTION_LENGTH 8
#define SCCB_CLOCK_RATE 13000 //13MHz

//#define I2C_V2_DVT

#ifdef I2C_V2_DVT
   #define I2C_DMA_ENABLED //Only used in DVT
#endif


typedef enum
{
 SCCB_OWNER_CAMERA = 0,
 SCCB_OWNER_TP,
 SCCB_OWNER_MS,
 SCCB_NUM_OF_OWNER
}SCCB_OWNER;


typedef enum
{
 SCCB_TRANSACTION_COMPLETE,
 SCCB_TRANSACTION_FAIL
}SCCB_TRANSACTION_RESULT;


typedef enum
{
 SCCB_READY_STATE,
 SCCB_BUSY_STATE
}SCCB_STATE;

/* Transaction mode for new SCCB APIs */
typedef enum
{
 SCCB_TRANSACTION_FAST_MODE,
 SCCB_TRANSACTION_HIGH_SPEED_MODE
}SCCB_TRANSACTION_MODE;

typedef enum
{
 SCCB_100KB, //101.5KB
 SCCB_200KB, //203.1KB
 SCCB_300KB, //295.5KB
 SCCB_400KB, //382.4KB
 /* HS Mode */
 SCCB_460KB, //464.3KB
 SCCB_540KB, //541.7KB
 SCCB_650KB, //650.0KB
 SCCB_720KB, //722.0KB

 SCCB_810KB, //812.5KB
 SCCB_930KB, //928.6KB
 SCCB_1100KB, //1083.3KB
 SCCB_1300KB, //1300.0KB
 SCCB_1625KB, //1625.0KB
 SCCB_2150KB, //2166.6KB
 SCCB_3250KB  //3250.6KB
}SCCB_SPEED_ENUM;

/* Transaction mode for existing SCCB APIs */
typedef enum
{
   SCCB_SW_8BIT=1,
   SCCB_SW_16BIT,   
   SCCB_HW_8BIT,
   SCCB_HW_16BIT
} SCCB_MODE_ENUM;

typedef struct
{
 uint8_t sccb_mode; // Transaction mode for existing SCCB APIs 

 bool get_handle_wait; //When get handle wait until the sccb is avaliable
 
 uint8_t slave_address; //the address of the slave device
 
 uint8_t delay_len; //number of half pulse between transfers in a trasaction

 SCCB_TRANSACTION_MODE transaction_mode; //SCCB_TRANSACTION_FAST_MODE or SCCB_TRANSACTION_HIGH_SPEED_MODE
 
 uint32_t Fast_Mode_Speed; //The speed of sccb fast mode(Kb)
 
 uint32_t HS_Mode_Speed; //The speed of sccb high speed mode(Kb)

 #if (defined(I2C_DMA_ENABLED))
 bool is_DMA_enabled; //Transaction via DMA instead of 8-byte FIFO
 #endif

}sccb_config_struct;

typedef struct
{
 sccb_config_struct  sccb_config;

 uint8_t fs_sample_cnt_div; //these two parameters are used to specify sccb clock rate
 uint8_t fs_step_cnt_div;  //half pulse width=step_cnt_div*sample_cnt_div*(1/13Mhz)

 uint8_t hs_sample_cnt_div; //these two parameters are used to specify sccb clock rate
 uint8_t hs_step_cnt_div;  //half pulse width=step_cnt_div*sample_cnt_div*(1/13Mhz)

 SCCB_TRANSACTION_RESULT transaction_result; /*The result of the end of transaction 
            (SCCB_TRANSACTION_COMPLETE|SCCB_TRANSACTION_FAIL)*/
  
}sccb_handle_struct;

typedef struct
{
 volatile SCCB_STATE  state;
 uint8_t  owner;

 uint8_t number_of_read;
 uint8_t* read_buffer;
 
 #if (defined(I2C_DMA_ENABLED))
 bool is_DMA_enabled;
 #endif
 
}sccb_status_struct;

//---------------MT6225 Register Definitions----------------//
#if (defined(MT6225))
#define SCCB_SERIAL_CLK_PIN  8
#define SCCB_SERIAL_DATA_PIN  9
   #define SCCB_GPIO_SCL_MODE       1
   #define SCCB_GPIO_SDA_MODE       1
#elif(defined(MT6223))
   #define SCCB_SERIAL_CLK_PIN  45
   #define SCCB_SERIAL_DATA_PIN  46
   #define SCCB_GPIO_SCL_MODE       3
   #define SCCB_GPIO_SDA_MODE       3
#endif

#define I2C_base 0xB0160000

/* Register Definitions */
#define REG_I2C_DATA_PORT      *((volatile unsigned char *) (I2C_base + 0x00))
#define REG_I2C_SLAVE_ADDR      *((volatile unsigned short int *) (I2C_base + 0x04))
#define REG_I2C_INT_MASK      *((volatile unsigned short int *) (I2C_base + 0x08))
#define REG_I2C_INT_STA      *((volatile unsigned short int *) (I2C_base + 0x0c))
#define REG_I2C_CONTROL      *((volatile unsigned short int *) (I2C_base + 0x10))
#define REG_I2C_CONTROL2           *((volatile unsigned short int *) (I2C_base + 0x14)) 
#define REG_I2C_TRANSFER_LEN       *((volatile unsigned short int *) (I2C_base + 0x20))
#define REG_I2C_TRANSFER_LEN_AUX   *((volatile unsigned short int *) (I2C_base + 0x24))
#define REG_I2C_TRANSAC_LEN        *((volatile unsigned short int *) (I2C_base + 0x28))
#define REG_I2C_DELAY_LEN          *((volatile unsigned short int *) (I2C_base + 0x2c))
#define REG_I2C_TIMING             *((volatile unsigned short int *) (I2C_base + 0x30))
#define REG_I2C_CLK                *((volatile unsigned short int *) (I2C_base + 0x34))
#define REG_I2C_START              *((volatile unsigned short int *) (I2C_base + 0x38))
#define REG_I2C_FIFO_STAT          *((volatile unsigned short int *) (I2C_base + 0x40))
#define REG_I2C_FIFO_ADDR_CLR      *((volatile unsigned short int *) (I2C_base + 0x48))
#define REG_I2C_IO_CONFIG          *((volatile unsigned short int *) (I2C_base + 0x50))
#define REG_I2C_HS_MODE            *((volatile unsigned short int *) (I2C_base + 0x60))
#define REG_I2C_SOFTRESET          *((volatile unsigned short int *) (I2C_base + 0x70))


/* Register masks */
#define I2C_3_BIT_MASK   0x07
#define I2C_4_BIT_MASK   0x0f
#define I2C_6_BIT_MASK   0x3f
#define I2C_8_BIT_MASK   0xff

#define I2C_FIFO_THRESH_MASK 0x07

#define I2C_AUX_LEN_MASK    0x1f00

#define I2C_MASTER_READ    0x01
#define I2C_MASTER_WRITE    0x00 

//#define I2C_CTL_MODE_BIT    0x01
#define I2C_CTL_RS_STOP_BIT   0x02
#define I2C_CTL_DMA_EN_BIT    0x04
#define I2C_CTL_CLK_EXT_EN_BIT   0x08
#define I2C_CTL_DIR_CHANGE_BIT   0x10
#define I2C_CTL_ACK_ERR_DET_BIT  0x20 
#define I2C_CTL_TRANSFER_LEN_CHG_BIT 0x40

#define I2C_DATA_READ_ADJ_BIT  0x8000

#define I2C_SCL_MODE_BIT   0x01
#define I2C_SDA_MODE_BIT   0x02
#define I2C_BUS_DETECT_EN_BIT  0x04  

#define I2C_ARBITRATION_BIT  0x01
#define I2C_CLOCK_SYNC_BIT   0x02
#define I2C_BUS_BUSY_DET_BIT  0x04

#define I2C_HS_EN_BIT    0x01
#define I2C_HS_NACK_ERR_DET_EN_BIT 0x02
#define I2C_HS_MASTER_CODE_MASK 0x70
#define I2C_HS_STEP_CNT_DIV_MASK 0x700
#define I2C_HS_SAMPLE_CNT_DIV_MASK 0x7000
 
/* I2C Status */
#define I2C_FIFO_FULL_STATUS  0x01
#define I2C_FIFO_EMPTY_STATUS  0x02

/* Register Settings */
#define SET_I2C_SLAVE_ADDRESS(n,rw) REG_I2C_SLAVE_ADDR=(uint8_t)((((n>>1)<<1) + rw) & I2C_8_BIT_MASK);

#define DISABLE_I2C_INT  REG_I2C_INT_MASK = 0
#define ENABLE_I2C_INT   REG_I2C_INT_MASK = I2C_4_BIT_MASK 

#define CLEAR_I2C_STA   REG_I2C_INT_STA=I2C_4_BIT_MASK;

//#define SET_I2C_FAST_SPEED_MODE REG_I2C_CONTROL &= ~I2C_CTL_MODE_BIT;
//#define SET_I2C_HIGH_SPEED_MODE REG_I2C_CONTROL |= I2C_CTL_MODE_BIT;
#define SET_I2C_ST_BETWEEN_TRANSFER REG_I2C_CONTROL &= ~I2C_CTL_RS_STOP_BIT;
#define SET_I2C_RS_BETWEEN_TRANSFER REG_I2C_CONTROL |= I2C_CTL_RS_STOP_BIT;
#define ENABLE_I2C_DMA_TRANSFER  REG_I2C_CONTROL |= I2C_CTL_DMA_EN_BIT;
#define ENABLE_I2C_CLOCK_EXTENSION  REG_I2C_CONTROL |= I2C_CTL_CLK_EXT_EN_BIT;
#define ENABLE_I2C_DIR_CHANGE   REG_I2C_CONTROL |= I2C_CTL_DIR_CHANGE_BIT;
#define ENABLE_I2C_ACK_ERR_DET   REG_I2C_CONTROL |= I2C_CTL_ACK_ERR_DET_BIT;
#define ENABLE_I2C_TRANSFER_LEN_CHG REG_I2C_CONTROL |= I2C_CTL_TRANSFER_LEN_CHG_BIT;

#define DISABLE_I2C_DMA_TRANSFER  REG_I2C_CONTROL &= ~I2C_CTL_DMA_EN_BIT;
#define DISABLE_I2C_CLOCK_EXTENSION REG_I2C_CONTROL &= ~I2C_CTL_CLK_EXT_EN_BIT;
#define DISABLE_I2C_DIR_CHANGE   REG_I2C_CONTROL &= ~I2C_CTL_DIR_CHANGE_BIT;
#define DISABLE_I2C_ACK_ERR_DET  REG_I2C_CONTROL &= ~I2C_CTL_ACK_ERR_DET_BIT;
#define DISABLE_I2C_TRANSFER_LEN_CHG REG_I2C_CONTROL &= ~I2C_CTL_TRANSFER_LEN_CHG_BIT;

#define SET_I2C_TRANSFER_LENGTH(n)  REG_I2C_TRANSFER_LEN &= ~I2C_8_BIT_MASK;\
          REG_I2C_TRANSFER_LEN |=(n)& I2C_8_BIT_MASK;
#define SET_I2C_AUX_TRANSFER_LENGTH(n) REG_I2C_TRANSFER_LEN &= ~ I2C_AUX_LEN_MASK;\
          REG_I2C_TRANSFER_LEN |= (((n<<8)& I2C_AUX_LEN_MASK));
 
#define SET_I2C_TRANSACTION_LENGTH(n) REG_I2C_TRANSAC_LEN =(n)& I2C_8_BIT_MASK;
#define SET_I2C_DELAY_LENGTH(n)  REG_I2C_DELAY_LEN =(n)& I2C_8_BIT_MASK;

#define SET_I2C_STEP_CNT_DIV(n) REG_I2C_TIMING &= ~I2C_6_BIT_MASK;\
         REG_I2C_TIMING |= ((n) & I2C_6_BIT_MASK);
#define SET_I2C_SAMPLE_CNT_DIV(n) REG_I2C_TIMING &=  ~(I2C_3_BIT_MASK<<8);\
         REG_I2C_TIMING |= ((n)& I2C_3_BIT_MASK)<<8;
#define SET_I2C_DATA_READ_TIME(n) REG_I2C_TIMING &=  ~(I2C_3_BIT_MASK<<12);\
         REG_I2C_TIMING |= ((n)& I2C_3_BIT_MASK)<<12;
#define ENABLE_I2C_DATA_READ_ADJ REG_I2C_TIMING |= I2C_DATA_READ_ADJ_BIT ;
#define DISABLE_I2C_DATA_READ_ADJ REG_I2C_TIMING &= ~I2C_DATA_READ_ADJ_BIT;
     
#define START_I2C_TRANSACTION  REG_I2C_START = 0x01;

#define I2C_FIFO_FULL    ((REG_I2C_FIFO_STAT>>1)&0x01)
#define I2C_FIFO_EMPTY    (REG_I2C_FIFO_STAT & 0x01)

#define SET_I2C_RX_FIFO_THRESH(n) REG_I2C_FIFO_THRESH &= ~I2C_FIFO_THRESH_MASK;\
         REG_I2C_FIFO_THRESH |= ((n) & I2C_FIFO_THRESH_MASK);
#define SET_I2C_TX_FIFO_THRESH(n) REG_I2C_FIFO_THRESH &= ~(I2C_FIFO_THRESH_MASK<<8);\
         REG_I2C_FIFO_THRESH |= (((n) & I2C_FIFO_THRESH_MASK)<<8);

#define CLEAR_I2C_FIFO    REG_I2C_FIFO_ADDR_CLR= 0x01;

#define SET_I2C_SCL_NORMAL_MODE REG_I2C_IO_CONFIG &= ~I2C_SCL_MODE_BIT;
#define SET_I2C_SCL_WIRED_AND_MODE REG_I2C_IO_CONFIG |= I2C_SCL_MODE_BIT;
#define SET_I2C_SDA_NORMAL_MODE REG_I2C_IO_CONFIG &= ~I2C_SDA_MODE_BIT;
#define SET_I2C_SDA_WIRED_AND_MODE REG_I2C_IO_CONFIG |= I2C_SDA_MODE_BIT;
#define ENABLE_I2C_BUS_DETECT  REG_I2C_IO_CONFIG |= I2C_BUS_DETECT_EN_BIT;
#define DISABLE_I2C_BUS_DETECT  REG_I2C_IO_CONFIG &= ~I2C_BUS_DETECT_EN_BIT;

#define ENABLE_I2C_CLOCK_SYNC  REG_I2C_MULTI_MASTER |= I2C_ARBITRATION_BIT;
#define ENABLE_DATA_ARBITION  REG_I2C_MULTI_MASTER |= I2C_CLOCK_SYNC_BIT;
#define ENABLE_I2C_BUS_BUSY_DET REG_I2C_MULTI_MASTER |= I2C_BUS_BUSY_DET_BIT;
#define DISABLE_I2C_CLOCK_SYNC  REG_I2C_MULTI_MASTER &= ~I2C_ARBITRATION_BIT;
#define DISABLE_DATA_ARBITION  REG_I2C_MULTI_MASTER &= ~I2C_CLOCK_SYNC_BIT;
#define DISABLE_I2C_BUS_BUSY_DET REG_I2C_MULTI_MASTER &= ~I2C_BUS_BUSY_DET_BIT;

#define SET_I2C_HIGH_SPEED_MODE_800KB REG_I2C_HS_MODE = 0x0703;
#define SET_I2C_HIGH_SPEED_MODE_1000KB REG_I2C_HS_MODE = 0x0503;

#define SET_I2C_FAST_MODE   REG_I2C_HS_MODE &= ~I2C_HS_EN_BIT;
#define SET_I2C_HS_MODE   REG_I2C_HS_MODE |= I2C_HS_EN_BIT;
#define ENABLE_I2C_NAKERR_DET  REG_I2C_HS_MODE |= I2C_HS_NACK_ERR_DET_EN_BIT;
#define DISABLE_I2C_NAKERR_DET  REG_I2C_HS_MODE &= ~I2C_HS_NACK_ERR_DET_EN_BIT;
#define SET_I2C_HS_MASTER_CODE(n) REG_I2C_HS_MODE &=  ~(I2C_HS_MASTER_CODE_MASK);\
         REG_I2C_HS_MODE |= (((n)<<4)& I2C_HS_MASTER_CODE_MASK);
#define SET_I2C_HS_STEP_CNT_DIV(n) REG_I2C_HS_MODE &=  ~(I2C_HS_STEP_CNT_DIV_MASK);\
         REG_I2C_HS_MODE |= (((n)<<8)& I2C_HS_STEP_CNT_DIV_MASK);
#define SET_I2C_HS_SAMPLE_CNT_DIV(n) REG_I2C_HS_MODE &=  ~(I2C_HS_SAMPLE_CNT_DIV_MASK);\
         REG_I2C_HS_MODE |= (((n)<<12)& I2C_HS_SAMPLE_CNT_DIV_MASK);     

#define RESET_I2C   REG_I2C_SOFTRESET = 0x01;
 
//---------------- DMA ----------------
//#define DMA_base  0x80030000 -->defined in /inc/reg_base.h
#define DMA_base      0x80030000

/* Regidter Definitions */
#define REG_DMA_CHANNEL_CONTROL(c) *((volatile unsigned int *) (DMA_base + 0x14+ (c<<8)))
#define REG_DMA_CHANNEL_START(c) *((volatile unsigned int *) (DMA_base + 0x18+ (c<<8)))
#define REG_DMA_PROG_ADDR(c)  *((volatile unsigned int *) (DMA_base + 0x2c+ (c<<8)))
#define REG_DMA_TRANSFER_COUNT(c) *((volatile unsigned int *) (DMA_base + 0x10+ (c<<8)))

/* Master Definitions*/
#define DMA_MASTER_I2C_TX 0x11
#define DMA_MASTER_I2C_RX 0x12
#define DMA_MASTER_IRDA_TX 0x02
#define DMA_MASTER_IRDA_RX 0x03

/* Register masks */
#define DMA_CON_DIR_MASK 0x40000
#define DMA_CON_MAS_MASK 0x01f00000

#define I2C_SET_TX_DMA_CONTROL(c,m)  REG_DMA_CHANNEL_CONTROL(c) = 0x00000014;\
      REG_DMA_CHANNEL_CONTROL(c) |= (((m)<<20) & DMA_CON_MAS_MASK);

#define I2C_SET_RX_DMA_CONTROL(c,m)  REG_DMA_CHANNEL_CONTROL(c) = 0x00040018;\
      REG_DMA_CHANNEL_CONTROL(c) |= (((m)<<20) & DMA_CON_MAS_MASK);

#define I2C_SET_DMA_PROGRAMMABLE_ADDR(c,addr) REG_DMA_PROG_ADDR(c) = (addr);
#define I2C_SET_DMA_TRANSFER_COUNT(c,size) REG_DMA_TRANSFER_COUNT(c)= size ;
#define I2C_START_DMA_TRANSFER(c)  REG_DMA_CHANNEL_START(c) = 0x8000;
#define I2C_STOP_DMA_TRANSFER(c)  REG_DMA_CHANNEL_START(c) = 0;
 
/****** SW definitions******/
#define I2C_READ_BIT 0x01
#define I2C_WRITE_BIT 0x00
 
#define I2C_TRANSAC_COMPLETE 0x01
#define I2C_TRANSAC_ACK_ERR 0x02
#define I2C_HS_NACK_ERR 0x04

void i2c_init(void);
void i2c_set_transaction_speed(SCCB_OWNER owner,SCCB_TRANSACTION_MODE mode,uint32_t* Fast_Mode_Speed,uint32_t* HS_Mode_Speed);
void i2c_config(SCCB_OWNER owner,sccb_config_struct* para);

SCCB_TRANSACTION_MODE i2c_get_transaction_mode(SCCB_OWNER owner);
SCCB_TRANSACTION_RESULT i2c_write(SCCB_OWNER owner,uint8_t* para,uint32_t datalen);
SCCB_TRANSACTION_RESULT i2c_read(SCCB_OWNER owner,uint8_t* para,uint32_t datalen);
SCCB_TRANSACTION_RESULT i2c_cont_write(SCCB_OWNER owner,uint8_t* para,uint32_t datalen,uint32_t transfer_num);
SCCB_TRANSACTION_RESULT i2c_cont_read(SCCB_OWNER owner,uint8_t* para,uint32_t datalen,uint32_t transfer_num);
SCCB_TRANSACTION_RESULT i2c_write_and_read(SCCB_OWNER owner,uint8_t* in_buffer,uint32_t in_len,uint8_t* out_buffer,uint32_t out_len);

void sccb_write(SCCB_OWNER owner,uint32_t cmd, uint32_t param);
void sccb_multi_write(SCCB_OWNER owner,uint32_t cmd, uint32_t *param, uint8_t num);
void sccb_cont_write(SCCB_OWNER owner,uint32_t cmd, uint32_t spec_cmd, uint32_t param);
uint32_t sccb_read (SCCB_OWNER owner,uint32_t cmd);
uint32_t sccb_phase3_read (SCCB_OWNER owner,uint32_t cmd);
uint8_t sccb_multi_read (SCCB_OWNER owner,uint32_t cmd, uint32_t *param, uint8_t num);
uint32_t sccb_cont_read (SCCB_OWNER owner,uint32_t cmd, uint32_t spec_cmd);

#if (defined(I2C_DMA_ENABLED))
void i2c_set_DMA(SCCB_OWNER owner,bool enable);
#endif

//#endif //DRV_I2C_25_SERIES
