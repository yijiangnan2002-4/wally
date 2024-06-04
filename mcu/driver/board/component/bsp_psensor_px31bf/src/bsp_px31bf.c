/**
  * @file    bsp_px31bf.c
  *
  * @author  wayne.xiong
  *
  * @date    2022/3/24
  * 
  * @brief   Driver of proximity sensor px31bf
**/

#include<string.h>
#include "bsp_px31bf.h"
#include "apps_debug.h"
#include "app_psensor_px31bf_activity.h"
#include "ui_shell_activity.h"
#include "ui_shell_manager.h"
#include "apps_events_event_group.h"
#include "app_customer_nvkey_operation.h"
#include "apps_events_interaction_event.h"
#include "bsp_hx300x.h"

uint8_t g_bsp_psensor_using_ic = 0xFF;


//******************************************************************
hal_i2c_status_t PX31BF_I2C_Write(uint8_t reg, uint8_t* data, uint8_t length)
{//LSB first
    uint8_t i2cBuffer[8];
    i2cBuffer[0] = reg;
    memcpy(i2cBuffer + 1, data, length);
    return hal_i2c_master_send_polling(BSP_PX31BF_I2C_PORT, PX31BF_ID, (uint8_t *)i2cBuffer, length + 1);
	
}

hal_i2c_status_t PX31BF_I2C_Write_byte(uint8_t reg, uint8_t data)
{
	return PX31BF_I2C_Write(reg, &data, 1);
}


//******************************************************************
hal_i2c_status_t PX31BF_I2C_Read(uint8_t reg, uint8_t* data, uint8_t length)
{
    hal_i2c_master_send_polling(BSP_PX31BF_I2C_PORT, PX31BF_ID, &reg, 1);
    return hal_i2c_master_receive_polling(BSP_PX31BF_I2C_PORT, PX31BF_ID, data, length);
}


//******************************************************************
void bsp_px31bf_enable(uint8_t enable)
{
	
	if(enable)
	{
		PX31BF_I2C_Write_byte( 0xF0, 0x02);
		hal_gpt_delay_ms(10); // need to delay 10ms after enable
	}
	else
	{
		PX31BF_I2C_Write_byte( 0xF0, 0x00);
		hal_gpt_delay_ms(5); // need to delay 5ms after disable
	}
}


//ISR
static uint8_t bsp_px31bf_ISR(void) 
{
	uint8_t near_far_flag = 0;
	uint8_t buf, IntFlag = 0;

	PX31BF_I2C_Read( 0xFE, &IntFlag, 1); //read near / far status


	if(IntFlag & 0x02) 
	{// checking PsInt, make shure the ISR is from sensor
		PX31BF_I2C_Read( 0xFF, &buf, 1); //read near / far status

		if(buf & 0x80) 
			near_far_flag = 0; //far event
		else
			near_far_flag = 1; //near event

		PX31BF_I2C_Write_byte(0xFE, 0x00); //release interrupt pin and flag
	}

	return near_far_flag;
}

static void bsp_px31bf_callback(void *data)
{

#ifdef MTK_IN_EAR_FEATURE_ENABLE
		uint16_t *p_wear_status = (uint16_t *)pvPortMalloc(sizeof(uint16_t)); /* free by ui shell */
		
		*p_wear_status = (uint16_t)bsp_px31bf_ISR();

		APPS_LOG_MSGID_I("px31bf:interrupt pin ret = %d", 1, *p_wear_status);
		app_set_ir_isr_status(*p_wear_status);

		
		ui_shell_remove_event(EVENT_GROUP_UI_SHELL_APP_INTERACTION, APPS_EVENTS_INTERACTION_UPDATE_IN_EAR_STA_EFFECT);
		if((*p_wear_status)==1)
		{
			ui_shell_send_event(true, EVENT_PRIORITY_HIGNEST, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
									 APPS_EVENTS_INTERACTION_UPDATE_IN_EAR_STA_EFFECT, (void *)p_wear_status, sizeof(bool),
									 NULL, 500);
		}
		else
		{
			ui_shell_send_event(true, EVENT_PRIORITY_HIGNEST, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
									 APPS_EVENTS_INTERACTION_UPDATE_IN_EAR_STA_EFFECT, (void *)p_wear_status, sizeof(bool),
									 NULL, 100);
		}
#endif

    /* Clear interrupt flag to receive new interrupt */
    hal_eint_unmask(EINT_PSENSOR);						
}

void bsp_px31bf_config_calibration(void)
{	
	uint8_t PsDacCtrl = 0;
	uint8_t PsCtDac = 0;
	uint16_t PsCal = 0;
	uint8_t Calibrate_Status = 0;
	
	// load calibration value from flash memory (add by customer)
	app_nvkey_psensor_calibration_cfg_read(&PsDacCtrl, &PsCtDac, &PsCal, &Calibrate_Status);

	if(Calibrate_Status)
	{
		bsp_px31bf_enable(0);

		if(PsDacCtrl == 0)
			PsDacCtrl = 1;
		
		PX31BF_I2C_Write_byte(0x65, PsDacCtrl); //set PsDacCtrl
		PX31BF_I2C_Write_byte(0x68, PsCtDac); //set PsCal
		PX31BF_I2C_Write(0x69, (uint8_t *)&PsCal, 2); //set PsCal
		
		bsp_px31bf_enable(1);
	}
	else
	{
		//APPS_LOG_MSGID_I("px31bf not calibrate yet, do auto calibrate now!", 0);
		PsDacCtrl = 0x51;
		PX31BF_I2C_Write_byte(0x65, PsDacCtrl); //set PsDacCtrl
		//bsp_px31bf_auto_dac();
	}
	
}

//Initial Sensor
static void bsp_px31bf_initial(void)
{
	uint16_t PsensorThresholdHigh = 0;
	uint16_t PsensorThresholdLow = 0;
	uint8_t temp =0;
	uint8_t psensor_setting[4] = {0};

	app_nvkey_psensor_setting_read(psensor_setting);
	
	uint8_t hw_version [6] = {0};
	bool old_version = 0;
	uint8_t Ps_W = psensor_setting[0];
	uint8_t Ps_C = psensor_setting[1];
	uint8_t Ps_Drv = psensor_setting[2];
	
	app_nvkey_hw_version_read(hw_version, 5);	
	if(memcmp((void*)hw_version, "1.0.0", 5) == 0)
	{
		old_version = 1;
	}	


	PX31BF_I2C_Write_byte(0xF4, 0xEE); // soft reset
	hal_gpt_delay_ms(30); // waiting for soft reset ready
	PX31BF_I2C_Write_byte(0x60, PsCtrl); // ADC output = 10-bits, Mean = 1

	if(Ps_W & 0x80)
		Ps_W &= 0x7F;
	else
	{
		if(old_version)
		{
			Ps_W = PsPuw_old;
		}
		else
		{
			Ps_W = PsPuw;
		}
	}

	if(Ps_C & 0x80)
		Ps_C &= 0x7F;
	else
	{
		if(old_version)
		{
			Ps_C = PsPuc_old;
		}
		else
		{
			Ps_C = PsPuc;
		}
	}

	if(Ps_Drv & 0x80)
		Ps_Drv &= 0x7F;
	else
	{
		if((memcmp((void*)hw_version, "3.0.0", 5) == 0 )
			|| (memcmp((void*)hw_version, "2.0.0", 5) == 0)
			|| (memcmp((void*)hw_version, "1.0.0", 5) == 0)
			)
		{
			Ps_Drv = PsDrv;
		}
		else
		{
			Ps_Drv = PsDrv_new;
		}
	}	
	
	PX31BF_I2C_Write_byte(0x61, Ps_W); // VSCEL pulses width
	
	PX31BF_I2C_Write_byte(0x62, Ps_C); // VSECL pulses count
	PX31BF_I2C_Write_byte(0x64, Ps_Drv); // VSCEL driving current
	PX31BF_I2C_Write_byte(0x4F, WaitTime); // Sensor waiting time

	app_nvkey_psensor_threshold_read(&temp, &PsensorThresholdHigh, &PsensorThresholdLow);	
	//High, Low Threshold setting path. Form default or factory calibration value
	if(PsensorThresholdHigh ==0 || PsensorThresholdLow == 0)
	{
		PsensorThresholdHigh = PsDefaultThresholdHigh;
		if(old_version)
		{
			PsensorThresholdLow = 400;
		}
		else
		{
			PsensorThresholdLow = PsDefaultThresholdLow;
		}		
	}

	PX31BF_I2C_Write_byte(0x6C, (PsensorThresholdLow & 0x00FF) );
	PX31BF_I2C_Write_byte(0x6D, ((PsensorThresholdLow>>8) & 0x00FF) );
	PX31BF_I2C_Write_byte(0x6E, (PsensorThresholdHigh & 0x00FF) );
	PX31BF_I2C_Write_byte(0x6F, ((PsensorThresholdHigh>>8) & 0x00FF) );
	
	PX31BF_I2C_Write_byte(0xFE, 0x00 ); // clear status flag
	bsp_px31bf_enable(1); //PX31BF Enable
}

static void bsp_px31bf_HW_config(void)
{
	//config power & reset	
	//TODO:		

    /* Config i2c peripheral */
    hal_i2c_config_t i2c_config;
	
    i2c_config.frequency = HAL_I2C_FREQUENCY_100K;
    hal_i2c_status_t ret = hal_i2c_master_init(BSP_PX31BF_I2C_PORT, &i2c_config);	
}

#if 0
static TimerHandle_t xBsp_ir_red_timer;

static void bsp_px31bf_read_reg_test(TimerHandle_t pxTimer)
{
	uint8_t ir_data[2] = {0};
	uint8_t reg_68[3] = {0};	
	uint8_t reg_65 = 0;
	uint16_t ps_data = 0;
	uint16_t reg_69 = 0;
	
	if(PX31BF_I2C_Read(0x00, ir_data, 2) != HAL_I2C_STATUS_OK) //Get PS Data.
		printf("px31bf test read 0x00 error!");

	if(PX31BF_I2C_Read(0x65, &reg_65, 1) != HAL_I2C_STATUS_OK) //Get PS Data.
		printf("px31bf test read 0x65 error!");	

	if(PX31BF_I2C_Read(0x68, reg_68, 3) != HAL_I2C_STATUS_OK) //Get PS Data.
		printf("px31bf test read 0x68 error!");	

	ps_data = (ir_data[1]<<8)|ir_data[0];
	reg_69 = (reg_68[2]<<8) |reg_68[1];
	printf("px31bf test reg00=%d, reg65=%d, reg68=%d, reg69=%d", ps_data, reg_65, reg_68[0], reg_69);	
	
}

#endif

bool bsp_is_px31bf_using(void)
{
	uint8_t i = 0;
	bool ret = 0;

	for(i=0;i<5;i++)
	{
		uint8_t data = 0;
	
		if(HAL_I2C_STATUS_OK == PX31BF_I2C_Read(0x00, &data, 1))
		{
			APPS_LOG_MSGID_I("psensor px31bf:iic checking reg00 = 0x%x", 1, data);
			ret = 1;
			break;
		}
	}

	return ret;
}



static void bsp_component_px31bf_init(void)
{		
    hal_eint_config_t eint_config;
	hal_eint_status_t ret; 

	//bsp_px31bf_HW_config();
	
	ret = hal_eint_mask(EINT_PSENSOR);
	APPS_LOG_MSGID_I("px31bf:hal_eint_mask ret = %d", 1, ret);

	eint_config.debounce_time = 5;
	eint_config.trigger_mode  = HAL_EINT_EDGE_FALLING;

	ret = hal_eint_init(EINT_PSENSOR, &eint_config);
	APPS_LOG_MSGID_I("px31bf:hal_eint_init ret = %d", 1, ret);

	ret = hal_eint_register_callback((hal_eint_number_t)EINT_PSENSOR, (hal_eint_callback_t)bsp_px31bf_callback, NULL);
	APPS_LOG_MSGID_I("px31bf:hal_eint_register_callback ret = %d", 1, ret);

	hal_eint_unmask(EINT_PSENSOR);

	bsp_px31bf_initial();

#if 0
	xBsp_ir_red_timer = xTimerCreate("ir read timer", 2000, pdTRUE, NULL, bsp_px31bf_read_reg_test);
    if (xTimerStart(xBsp_ir_red_timer, 0) != pdPASS) {
        APPS_LOG_MSGID_E("ir read timer xTimerStart fail\n", 0);
    }
#endif

}

uint8_t bsp_component_psensor_ic(void)
{
	return g_bsp_psensor_using_ic;
}

void bsp_component_psensor_init(void)
{
	uint8_t hw_version [6] = {0};
	
	bsp_px31bf_HW_config();

	app_nvkey_hw_version_read(hw_version, 5);	
	if(/*hw_version[0] >= 0x36 &&*/ bsp_is_hx300x_using())//hw version 6.x.x
	{
		APPS_LOG_MSGID_I("psensor init hx300x is using", 0);
		bsp_component_hx300x_init();
		g_bsp_psensor_using_ic = PSENSOR_HX300X_USING;
	}
	else if(bsp_is_px31bf_using())
	{
		APPS_LOG_MSGID_I("psensor init px31bf is using", 0);
		bsp_component_px31bf_init();
		g_bsp_psensor_using_ic = PSENSOR_PX31BF_USING;
	}
	else
	{
		APPS_LOG_MSGID_E("psensor iic error!", 0);
	}

}


//Sensor crosstalk calibration:
uint8_t bsp_px31bf_auto_dac(void)
{
	uint8_t buff[5] = {0};
	uint16_t PsData = 0;
	bool first_data = true;
	uint8_t Calibrate_Status = 0;
	
	//Setting Variable
	uint8_t PsDacCtrl_val = 0;
	uint8_t PsFineGain_Val = 0;
	//uint8_t PsCoasGain_Val = 0;
	uint8_t PsFineCt_Val = 0;
	int16_t PsFineCt_Val_temp = 0;
	
	//PI Control variable
	bool PI_Control = true;
	int32_t dp = 0;
	int32_t di = 0;
	uint8_t last_try = 0;
	
	//Bisection method
	uint8_t PsFineCt_Val_Max = 127;
	uint8_t PsFineCt_Val_Min = 0;
	uint8_t psensor_setting[4] = {0};

	app_nvkey_psensor_setting_read(psensor_setting);
	

	uint8_t hw_version [6] = {0};
	bool old_version = 0;
	uint8_t Ps_W = psensor_setting[0];
	uint8_t Ps_C = psensor_setting[1];
	uint8_t Ps_Drv = psensor_setting[2];
	
	app_nvkey_hw_version_read(hw_version, 5);	
	if(memcmp((void*)hw_version, "1.0.0", 5) == 0)
	{
		old_version = 1;
	}	
	
	//Sensor Initial
	bsp_px31bf_enable(0); //Disable Sensor
	PX31BF_I2C_Write_byte(0x60, PsCtrl ); //10bits, Mean 30ms, PsGain default
	
	if(Ps_W & 0x80)
		Ps_W &= 0x7F;
	else
	{
		if(old_version)
		{
			Ps_W = PsPuw_old;
		}
		else
		{
			Ps_W = PsPuw;
		}
	}

	if(Ps_C & 0x80)
		Ps_C &= 0x7F;
	else
	{
		if(old_version)
		{
			Ps_C = PsPuc_old;
		}
		else
		{
			Ps_C = PsPuc;
		}
	}

	if(Ps_Drv & 0x80)
		Ps_Drv &= 0x7F;
	else
	{
		if((memcmp((void*)hw_version, "3.0.0", 5) == 0 )
			|| (memcmp((void*)hw_version, "2.0.0", 5) == 0)
			|| (memcmp((void*)hw_version, "1.0.0", 5) == 0)
			)
		{
			Ps_Drv = PsDrv;
		}
		else
		{
			Ps_Drv = PsDrv_new;
		}
	}

	PX31BF_I2C_Write_byte(0x61, Ps_W); //Pulse width	
	PX31BF_I2C_Write_byte(0x62, Ps_C ); //Pulses count
	PX31BF_I2C_Write_byte(0x64, Ps_Drv ); //vscel driving
	PX31BF_I2C_Write_byte(0x4F, 0x00 ); //WaitTime = 0
	PX31BF_I2C_Write_byte(0x65, 0x01); //Reset PsDacCtrl
	PX31BF_I2C_Write_byte(0x68, 0x00 ); //Reset PsFineCt
	PX31BF_I2C_Write_byte(0x69, 0x00 ); //Reset PsCal low byte
	PX31BF_I2C_Write_byte(0x6A, 0x00 ); //Reset PsCal high byte
	PX31BF_I2C_Write_byte(0xF1, 0x01 ); //Close INT pin output
	PX31BF_I2C_Write_byte(0xF2, 0x10 ); //Enable Data Ready Interrupt Halt
	PX31BF_I2C_Write_byte(0xFE, 0x00 ); //Clear Interrupt Flag
	PX31BF_I2C_Write_byte(0x7B, 0x08 ); //Enable Fast-En(Factor function)

	PsFineGain_Val = 0x01;
	//PsCoasGain_Val = 0x05;

	bsp_px31bf_enable(1); //Enable Sensor
	
	//First Step
	while (1)
	{
		if(PX31BF_I2C_Read(0xFE, buff, 4) != HAL_I2C_STATUS_OK) //Get Interrupt flag and PS Data.
			return 0;
		
		if ((buff[0] & 0x10) == 0x10) //Data Ready flag
		{
			PsData = (uint16_t)buff[2] + ((uint16_t)buff[3] << 8);

			if (first_data) //Ignore the first data.
			{
				first_data = false;
				PX31BF_I2C_Write_byte(0xFE, 0x00 ); //Clear Interrupt Flag
				continue;
			}

			//With last try and PS Data > 0, finish the calibration else keep going.
			if (last_try == 1 && PsData > 0)
				break;
			else
				last_try = 0;
			
			if (PsFineCt_Val > 0)
			{
				//The PsCtDac is over spec, try to use the bisection method to get the right setting.
				if (PsData == 0)
				{
					PsFineCt_Val_Max = (uint8_t)PsFineCt_Val;
					PI_Control = false;
				}
				//PS Data <= target value, finish the calibration.
				else if (PsData <= TARGET_PXY)
					break;
				//With the bisection method, we get the last value. finish calibration.
				else if (PsFineCt_Val == PsFineCt_Val_Min || PsFineCt_Val == PsFineCt_Val_Max)
					break;
			}
			//PS Data <= target value, finish the calibration.
			else if (PsData <= TARGET_PXY)
				break;

			if (PI_Control) //Get the setting with PI control.
			{
				dp = PsData - TARGET_PXY;
				di += dp;
				PsFineCt_Val_temp = (int16_t) PsFineCt_Val
				+ (int16_t)((dp >> 6) + ((di >> 6) + (di >>8))) + (dp >=0 ? 1 : -1);

				if (PsFineCt_Val_temp > 127)
				{
					if (PsFineGain_Val == 0x0F)
					{
						last_try = 1;
						PsFineCt_Val_temp = 127;
					}
					else
					{
						if (dp > (TARGET_PXY <<1)) //If PS Data > (target value) x 2
						{
							PsFineGain_Val <<=1; //New PsCtGain = PsCtGain x2
						}
						else if (dp > TARGET_PXY) //If PS Data > target value
						{
							PsFineGain_Val +=2; //New PsCtGain = PsCtGain + 2
						}
						else
						{
							PsFineGain_Val ++; // New PsCtGain = PsCtGain + 1
						}
						
						if (PsFineGain_Val == 0x00)
							PsFineGain_Val = 1;
						else if (PsFineGain_Val > 0x0F)
							PsFineGain_Val = 0x0F;
						
						PsFineCt_Val_temp = 64;
						PsDacCtrl_val = (PsDacCtrl_val & 0xF0) | PsFineGain_Val;
						PX31BF_I2C_Write_byte(0x65, PsDacCtrl_val );
					}
				}
			}
			else
			{
				if (PsData > TARGET_PXY)
					PsFineCt_Val_Min = (uint8_t) PsFineCt_Val_temp;

				if (PsData < PXY_FULL_RANGE && PsData > TARGET_PXY) //Reduce calculate time.
					PsFineCt_Val_temp += 1;
				else
					PsFineCt_Val_temp = (int16_t)(PsFineCt_Val_Min + PsFineCt_Val_Max) >> 1;
			}
			
			PsFineCt_Val = (uint8_t) PsFineCt_Val_temp;
			PX31BF_I2C_Write_byte(0x68, PsFineCt_Val );
			PX31BF_I2C_Write_byte(0xFE, 0x00 ); //Clear Interrupt Flag
		}
	}

	bsp_px31bf_enable(0); //Shutdown sensor
	PX31BF_I2C_Write_byte(0xFE, 0x00 ); //Clear IntFlag                 
	PX31BF_I2C_Write_byte(0xF2, 0x00 ); //DataHalt Disable restore defau
	PX31BF_I2C_Write_byte(0x7B, 0x00 ); //FastEn Disable restore default

	//Second Step to calibrate PsCal
	bsp_px31bf_enable(1); //Enable sensor

	uint8_t index = 0;
	uint32_t Sum = 0;

	do
	{
		PX31BF_I2C_Read(0xFE, buff, 4);

		if ((buff[0] & 0x10) == 0x10)
		{
			PsData = (uint16_t)buff[2] + ((uint16_t)buff[3] << 8);
			buff[0] = 0x00;
			PX31BF_I2C_Write_byte(0xFE, 0x00 );

			if(index > 1) //Ignore the first two data
				Sum += PsData;

			index++;
		}
	}while (index < 10);
		
	bsp_px31bf_enable(0); //Shutdown sensor
	
	PsData = (uint16_t)(Sum >> 3) + 20;
	PX31BF_I2C_Write(0x69, (uint8_t *)&PsData, 2);
	PX31BF_I2C_Write_byte(0xF1, 0x03 ); // Open INT pin output
	PX31BF_I2C_Write_byte(0x4F, WaitTime ); //WaitTime = 170ms
	
	//Save calibration value to flash memory (this function have to add by customer)
	//reg 0x65 (PsDacCtrl_val)
	//reg 0x68 (PsFineCt_Val)
	//reg 0x69, 0x6a (PsData)
	Calibrate_Status = 1;
	if(PsDacCtrl_val == 0)
		PsDacCtrl_val = 1;
		
	app_nvkey_psensor_calibration_cfg_write(PsDacCtrl_val, PsFineCt_Val, PsData, Calibrate_Status);
	
	bsp_px31bf_enable(1); //Enable sensor
	
	return 1;
}

uint16_t bsp_px31bf_Threshold_Factory_Calibrate(void)
{
	uint8_t ir_data[2] = {0};
	uint16_t ps_data = 0;
	
	if(PX31BF_I2C_Read(0x00, ir_data, 2) == HAL_I2C_STATUS_OK) //Get PS Data.
	{
		ps_data = (ir_data[1]<<8) |ir_data[0];
		return ps_data;
	}
	
	return 0xFFFF;
}

void bsp_px31bf_PsData_read(uint8_t* buff)
{	

	app_nvkey_psensor_threshold_read(buff, buff+1, buff+3);

	if(PX31BF_I2C_Read(0x00, buff+5, 2) != HAL_I2C_STATUS_OK) //Get PS Data.
		return;
}


void bsp_px31bf_PsData_read_reg(uint8_t* buff)
{
	if(PX31BF_I2C_Read(0x65, buff , 1) != HAL_I2C_STATUS_OK) //Get Crosstalk Data.
		return;	
	
	hal_gpt_delay_ms(30); 
	if(PX31BF_I2C_Read(0x6C, buff+1, 4) != HAL_I2C_STATUS_OK) //Get PsThreshold
		return;
	
	hal_gpt_delay_ms(30); 
	if(PX31BF_I2C_Read(0x69, buff + 5, 2) != HAL_I2C_STATUS_OK) //Get Crosstalk Data.
		return;	
}


