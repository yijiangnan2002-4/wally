#ifndef __LIGHT_TEST_H
#define __LIGHT_TEST_H 			   
#include "hal.h"
#include "hal_i2c_master.h"



extern uint8_t final_trim_leddr;
extern uint8_t final_trim_osc;
extern uint8_t final_trim_ldo;

#define FACTORY_TEST
#define R0X12_DEF         0x90 //0x40


//I2C device address (7bits)
#define HX300X_ID (0x44)
#define HX300X_ID_2 (0x45)

#define PIO_HX300X_INT HAL_GPIO_10
#define BSP_HX300X_I2C_PORT HAL_I2C_MASTER_0

#define CALI_STEP1 (1<<0)
#define CALI_STEP2 (1<<1)
#define CALI_STEP3 (1<<2)
#define CALI_STEP4 (1<<3)





//#define L_PS 1800
//#define H_PS 1900

uint8_t HX300x_Infrared_Rev1(uint32_t H_PS ,uint32_t L_PS);
uint32_t read_ps_data(void);
void hx300x_Set_Thres(uint16_t h_thrd, uint16_t l_thrd);
void HX300x_init(void);
void hx300x_Set_gain(void);
void hx300x_Set_cur(void);
uint16_t hx300x_int_handle(void);
uint16_t hx300x_read_ps_data(void);
uint8_t HX300x_Calibration(void);

uint8_t Calibration_First_IIC_INT_check(void);
uint16_t Calibration_Second_OpenAir(void);
void Calibration_Third_GreyCard(void *pEvt);
void Calibration_Fourth_RemoveGreyCard(void *pEvt);
uint8_t Calibration_Fifth_SaveDataToFlash(void);
void hx300x_spp_stop_log(void);
void hx300x_spp_open_log(void);
uint8_t HX300X_reg_ctr_write(uint8_t addr, uint8_t data);
uint8_t HX300X_reg_ctr_read(uint8_t addr, uint8_t *data);
void HX300X_read_all_reg(void);
void bsp_component_hx300x_init(void);


#endif


