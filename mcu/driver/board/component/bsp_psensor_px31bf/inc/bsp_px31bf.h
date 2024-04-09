/**
  * @file    bsp_px31bf.h
  *
  * @author  wayne.xiong
  *
  * @date    2022/3/24
  * 
  * @brief   Driver of proximity sensor px31bf
**/



#ifndef _BSP_PX31BF_H_
#define _BSP_PX31BF_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "hal.h"
#include "hal_i2c_master.h"


#define EINT_PSENSOR HAL_EINT_NUMBER_5



//I2C device address (7bits)
#define PX31BF_ID (0x1C)

#define BSP_PX31BF_I2C_PORT HAL_I2C_MASTER_0

#define PsBits (0x01) //10-bits
#define PsMean (0x00) //Mean 1
#define PsCtrl ((PsBits << 4) | (PsMean << 6) | (0x05))
#define PsPuw (0x01) //16 width = 32 us
#define PsPuc (0x01) //2 count
#define PsPuw_old (0x10) //16 width = 32 us
#define PsPuc_old (0x02) //2 count
#define PsDrv (0x0B) //12 mA
#ifdef PROJECT_GAHAN
#define PsDrv_new (0x04) //12 mA
#else
#define PsDrv_new (0x0B) //12 mA
#endif
#define PsDrvCtrl (PsDrv)
#define WaitTime (0x11) //170 ms
#define PsWaitAlgo (0x01)
#define PsIntAlgo (0x01)
#define PsPers (0x04)
//PsInt asserted after 4 consecutive PsData meets the PsInt criteria
#define PsAlgoCtrl ((PsWaitAlgo << 5) | (PsIntAlgo << 4) | (PsPers))
#define DefaultThreshold 1 //1 = fixed threshold, 0 = factory threshold
#define PsDefaultThresholdHigh 600
#define PsDefaultThresholdLow 250
#define LoadCtCalibrationSetting 0 //1 = load close talk calibration setting
#define PXY_FULL_RANGE ((1 << (PsBits + 9)) - 1)
#define TARGET_PXY ((PXY_FULL_RANGE + 1) >> 2)


//Function form MCU
hal_i2c_status_t PX31BF_I2C_Write(uint8_t reg, uint8_t* data, uint8_t length);
hal_i2c_status_t PX31BF_I2C_Read(uint8_t reg, uint8_t* data, uint8_t length);

void bsp_component_psensor_init(void);

void bsp_px31bf_enable(uint8_t enable);
void bsp_px31bf_config_calibration(void);
uint8_t bsp_px31bf_auto_dac(void);
void bsp_px31bf_PsData_read(uint8_t* buff);
void bsp_px31bf_PsData_read_reg(uint8_t* buff);
uint16_t bsp_px31bf_Threshold_Factory_Calibrate(void);
uint8_t bsp_component_psensor_ic(void);




#endif //_BSP_PX31BF_H_



