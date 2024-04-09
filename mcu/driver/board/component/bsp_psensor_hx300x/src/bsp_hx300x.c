
#include<string.h>
#include "bsp_hx300x.h"
#include "apps_debug.h"
#include "app_psensor_px31bf_activity.h"
#include "ui_shell_activity.h"
#include "ui_shell_manager.h"
#include "apps_events_event_group.h"
#include "app_customer_nvkey_operation.h"
#include "apps_events_interaction_event.h"

#define EINT_PSENSOR HAL_EINT_NUMBER_5

static uint8_t slave_addr = HX300X_ID;

//******************************************************************
hal_i2c_status_t HX300X_I2C_Write(uint8_t reg, uint8_t* data, uint8_t length)
{//LSB first
    uint8_t i2cBuffer[8];
    i2cBuffer[0] = reg;
    memcpy(i2cBuffer + 1, data, length);
    return hal_i2c_master_send_polling(BSP_HX300X_I2C_PORT, slave_addr, (uint8_t *)i2cBuffer, length + 1);
	
}

hal_i2c_status_t TYHX_Write_REG(uint8_t reg, uint8_t data)
{
	return HX300X_I2C_Write(reg, &data, 1);
}


//******************************************************************
hal_i2c_status_t HX300X_I2C_Read(uint8_t reg, uint8_t* data, uint8_t length)
{
    hal_i2c_master_send_polling(BSP_HX300X_I2C_PORT, slave_addr, &reg, 1);
    return hal_i2c_master_receive_polling(BSP_HX300X_I2C_PORT, slave_addr, data, length);
}

uint8_t TYHX_Read_REG(uint8_t reg)
{
	uint8_t data = 0;
	
	HX300X_I2C_Read(reg, &data, 1);
	return data;
}

uint8_t HX300X_reg_ctr_write(uint8_t addr, uint8_t data)
{	
	if(HAL_I2C_STATUS_OK == TYHX_Write_REG(addr, data))
		return 0;
	else
		return 1;
}

uint8_t HX300X_reg_ctr_read(uint8_t addr, uint8_t *data)
{
	if(HAL_I2C_STATUS_OK == HX300X_I2C_Read(addr, data,1))
		return 0;
	else
		return 1;
}


/* 延时函数， 根据平台完成接口*/
void delay_ms(uint16_t time_ms)
{
	hal_gpt_delay_ms(time_ms);
}


uint32_t Table_ps_data1[10];
uint32_t Table_ps_data2[10];


void Sort(uint32_t *a,int len)
{
    int i=0;
    int j;
    int t;
    for(i=0;i<len-1;i++) 
    {
        for(j=0;j<len-i-1;j++)
        {
            if(a[j]>a[j+1]) 
            {
                t=a[j];
                a[j]=a[j+1];
                a[j+1]=t;
            }
        }
    }
}

uint32_t read_ps_data(void)
{
    uint8_t i = 0, j = 0;
    uint32_t ps_data1 = 0, ps_data1_temp = 0;
    uint8_t  databuf[2];


    for (i = 0; i < 5; i++)
    {

        databuf[0] = TYHX_Read_REG(0x07);
        databuf[1] = TYHX_Read_REG(0x08);
        ps_data1_temp = ((databuf[0]) | (databuf[1] << 8));
        //printf("HX300X  /////ps1=%d ",ps_data1_temp);
        Table_ps_data1[i] = ps_data1_temp;
        delay_ms(10);
    }

    Sort(Table_ps_data1,5);
    for (j = 1; j < 4; j++)
    {
        ps_data1 = ps_data1 + Table_ps_data1[j];
    }
    ps_data1 = ps_data1 / 3;
    //printf("HX300X  //////////////////// ");
    return ps_data1;
}

//  数据消除抖动，一次耗时约 4*(55ms + delay_time_ms) ms
uint32_t get_ps_data(uint8_t delay_time_ms)
{
    uint8_t i = 0, j = 0;
    uint32_t ps_data1 = 0, ps_data1_temp = 0;
    uint8_t  databuf[2];


    for (i = 0; i < 10; i++)
    {

        databuf[0] = TYHX_Read_REG(0x07);
        databuf[1] = TYHX_Read_REG(0x08);
        ps_data1_temp = ((databuf[0]) | (databuf[1] << 8));
        //SEGGER_RTT_printf(0,"/////ps1=%d ",ps_data1_temp);
        Table_ps_data1[i] = ps_data1_temp;
        delay_ms(delay_time_ms);
    }

    Sort(Table_ps_data1,10);
    for (j = 2; j < 7; j++)
    {
        ps_data1 = ps_data1 + Table_ps_data1[j];
    }
    ps_data1 = ps_data1 / 5;
    //SEGGER_RTT_printf(0,"//////////////////// ");
    return ps_data1;
}
//----------------------------------------
uint16_t leddr = 0x1f, H_THOD = 704, L_THOD = 496;
uint8_t reg_13_new = 0x27,gain_r0x12 = 0x70;
uint16_t k=0;
uint16_t H_PS=1600,L_PS=1200;
uint8_t Reg10=0x1F,Reg12=0x70,Reg13=0x27;
int32_t read_ps_1f,read_ps_10,read_ps_k,read_ps2=0;
int32_t read_ps3 = 0;  
uint8_t cali_process = 0;

//
#if 0
uint8_t HX300x_Calibration(void)
{
	uint8_t ret=0;
	
	//printf("HX300X   >>第一步：IIC & INT 检测！请按键.... ");
	//while(START_IN);
	ret = Calibration_First_IIC_INT_check();
	if(ret != 1)
	{
		return 0;
	}
	//printf("HX300X   >>第二步：对空测试！请按键.... ");
	//while(START_IN);
	ret = Calibration_Second_OpenAir();
	if(ret != 1)
	{
		return 0;
	}	
	//printf("HX300X   >>第三步：上灰卡测试！请按键.... ");
	//while(START_IN);
	ret = Calibration_Third_GreyCard();
	if(ret == 0)
	{
		return 0;
	}	
	//printf("HX300X   >>第四步：撤灰卡测试！请按键.... ");
	//while(START_IN);
	ret = Calibration_Fourth_RemoveGreyCard();
	if(ret != 1)
	{
		return 0;
	}
	//printf("HX300X   >>第五步：保存，请按键.... ");
	//while(START_IN);
	Calibration_Fifth_SaveDataToFlash();

	return 1;
}
#endif
//--------------------------------------------------------------------    
uint8_t Calibration_First_IIC_INT_check(void)
{
    uint8_t cnt=0;
	uint8_t Reg[2];	

	cali_process = 0;
//	
    TYHX_Write_REG(0x00, 0x01);
    TYHX_Write_REG(0x01, 0x12);
    hx300x_Set_Thres(0, 0);
    TYHX_Write_REG(0x00, 0x06);
    delay_ms(100);
	
//IIC Test	
	printf("HX300X   ...IIC Start Test...... ");
	
	Reg[0] =TYHX_Read_REG(0x01);
	Reg[1] =TYHX_Read_REG(0x00);	
    if((Reg[0] == 0x12) && (Reg[1] == 0x06))
    {
        printf("HX300X   ...IIC Communication OK... ");
    }
    else
    {
       printf("HX300X   ...IIC Communication Fail... ");
       return 3; //0:IIC 通讯失败
    }	
	
	//printf("HX300X  reg[04]=0x%x, reg[05]=0x%x, reg[06]=0x%x ", TYHX_Read_REG(0x04), TYHX_Read_REG(0x05), TYHX_Read_REG(0x06));
	printf("HX300X   ...INT Start Test...... ");
    while(1)
    {
        printf("HX300X  INT_check step 1 cnt=%d  ", cnt);			
        if(0 == hx300x_int_handle())
        {
			printf("HX300X  INT_check level1 pass ");			
            break;
        }
        cnt++;
        if(cnt > 20)
        {
  			printf("HX300X  INT_check level1 failed ");			
            return 2;		// 2: INT 失败
        }
        delay_ms(5);
    }
    delay_ms(40);
    cnt=0;
    while(1)
    {
        printf("HX300X  INT_check step 2 cnt=%d  ", cnt);			
        read_ps_data();
        delay_ms(5);
        if(1 == hx300x_int_handle())
        {
  			printf("HX300X  INT_check level2 pass ");
			printf("HX300X     !!!INT Test OK... ");
			cali_process |= CALI_STEP1;
            return 1;   //1: IIC + INT 都通过
        }
        cnt++;
        if(cnt > 20)
        {
  			printf("HX300X     !!!INT Test Fail... ");			
            return 2;		// 2: INT 失败
        }
    }
  	printf("HX300X     !!!INT Test Fail... ");			
    return 2;		// 2: INT 失败
}

//-----------------------------------------------------------------
uint16_t Calibration_Second_OpenAir(void)  
{
    uint8_t  ii = 0;
    uint32_t read_ps1 = 0;
/////////////////////////////////////////////////////////////////////////////////////////////////
	  TYHX_Write_REG(0x00,0x01);
	  TYHX_Write_REG(0x01,0x91);      // FM, original is 0xa1
	  TYHX_Write_REG(0x02,0x30);      // FM, original is 0x10
	  TYHX_Write_REG(0x03,0x66);      
	  TYHX_Write_REG(0x04,0xFF);      // FM, original is 0x00
	  TYHX_Write_REG(0x05,0x10);      // FM, original is 0x00
	  TYHX_Write_REG(0x06,0x51);      
	  TYHX_Write_REG(0x0A,0x00);      
	  TYHX_Write_REG(0x0B,0x00);	     
	  TYHX_Write_REG(0x10,0x20);      
	  TYHX_Write_REG(0x11,0xF7);      
	  TYHX_Write_REG(0x12,0x70);      // FM, original is 0x50
	  TYHX_Write_REG(0x13,0x20);      
      TYHX_Write_REG(0x14,0x45);      // FM, new added
      TYHX_Write_REG(0x15,0x01);      // FM, new added
      TYHX_Write_REG(0x16,0x20);      // FM, new added
	  TYHX_Write_REG(0x20,0x28);      // FM, original is 0x85
	  TYHX_Write_REG(0x21,0xF3);      // FM, original is 0x53
	  TYHX_Write_REG(0x00,0x0A);	
/////////////////////////////////////////////////////////////////////////////////////////////////    	
     //printf("HX300X   ..对空测试结构光开始.. ");

//cal K 
    delay_ms(35);    
    read_ps1 = get_ps_data(35);
    printf("HX300X   OFFSET DAC_R13 = %02x, STR DATA = %4d ", (0x20|ii), read_ps1);
    ii = 0;    
    while (((read_ps1 > 704) && (0 <= ii) && (ii <= 15))) 
    {
        TYHX_Write_REG(0x00, 0x01);
        TYHX_Write_REG(0x13, (0x20|ii));
        TYHX_Write_REG(0x00, 0x0A);
        delay_ms(35);

        read_ps1 = get_ps_data(35);
        printf("HX300X   OFFSET DAC_R13 = %02x, STR DATA = %4d ", (0x10|ii), read_ps1);
        
        ii++;
    }

    reg_13_new = TYHX_Read_REG(0x13);

    TYHX_Write_REG(0x00, 0x01);
    TYHX_Write_REG(0x10, 0x10); 
    TYHX_Write_REG(0x00, 0x0A);
    delay_ms(35);

    read_ps_10 = get_ps_data(35);

    TYHX_Write_REG(0x00, 0x01);
    TYHX_Write_REG(0x10, 0x1f); 
    TYHX_Write_REG(0x00, 0x0A);
    delay_ms(35);

    read_ps_1f = get_ps_data(35);

    k = (read_ps_1f - read_ps_10) / 16; //????
         
    if (k<0)
    {
        // printf("HX300X   The Calculated Str Gain Ratio K Forced to Zero!!! ");
        k=0;
    }
	cali_process |= CALI_STEP2;

	printf("HX300X  read_ps1 = %d ",read_ps1);

	return (uint16_t)read_ps1;
}		
//===================================================	
void Calibration_Third_GreyCard(void *pEvt)
{// 灰卡测试 
	uint8_t ii = 0;
	uint16_t target_ps = (L_PS+H_PS)/2;
	int32_t data_ps = 0;
	int16_t MAXleddr, MINleddr;
	uint8_t Reg[4];

	typedef struct
	{
		uint8_t status;
		uint16_t ps_data;
		uint16_t ps_k;
	}PACKED RSP;

	RSP* pRsp = (RSP*)pEvt;

    //printf("HX300X   ...灰卡测试开始　 ");

//cal gain     
    data_ps = get_ps_data(35) - read_ps_1f; 
    printf("HX300X  target_ps = %4d, data_ps = %4d ",target_ps, data_ps);

    if(data_ps!=0)
    {
        gain_r0x12 = ((10*target_ps/data_ps)*8+5)/10;
    }
    
    if(gain_r0x12>=1)
    {
        gain_r0x12 = gain_r0x12-1;
    }
    else
    {
        gain_r0x12 = 0;
    }
    printf("HX300X  Gain r0x12 = %4d ",gain_r0x12);
    if(gain_r0x12<3) gain_r0x12 = 3;
    if(gain_r0x12>15) gain_r0x12 = 15;
    
    gain_r0x12 = gain_r0x12<<4; //gain 写到寄存器高4位
    printf("HX300X  Gain r0x12 shift = 0x%x ",gain_r0x12);
    TYHX_Write_REG(0x12, gain_r0x12);
    
//find data ps in the defined range ////////////
    MAXleddr = 48;//
    MINleddr = 16;//
    TYHX_Write_REG(0x13, reg_13_new);
    
    for (ii = 0; ii <= 4; ii++)
    {
        leddr = ((MINleddr + MAXleddr) >> 1 & 0x3f);
        TYHX_Write_REG(0x00, 0x01);
        TYHX_Write_REG(0x10, leddr);
        TYHX_Write_REG(0x00, 0x0A);
        delay_ms(35);
        
        read_ps2 = get_ps_data(35);
        
        read_ps_k = ((leddr - 16) * k + read_ps_10)*((gain_r0x12>>4)+1)/8;
        
        data_ps = read_ps2 - read_ps_k;
        printf("HX300X  read_ps2= %4d;leddr_Reg0x10 = 0x%02x; DATA_PS = %4d \
k2 = %d; 1f1x = %4d;2f1x = %d  ",\
               read_ps2,TYHX_Read_REG(0x10), data_ps,k,read_ps_10,read_ps_1f);
        if (data_ps > (H_PS + L_PS)/2)
        {
            MAXleddr = leddr;// 
        }   
        else
        {
            MINleddr = leddr;
        }                
    }

	Reg[0] =TYHX_Read_REG(0x10);
    Reg[1] =TYHX_Read_REG(0x11);
    Reg[2] =TYHX_Read_REG(0x12);
    Reg[3] =TYHX_Read_REG(0x13);
	
	pRsp->ps_data = (uint16_t)data_ps;
	pRsp->ps_k = (uint16_t)read_ps_k;
	
    printf("HX300X   ...R10 = 0x%x,R11 = 0x%x,R12 = 0x%x,R13 = 0x%x,  ", Reg[0],
            Reg[1],Reg[2],Reg[3]);
    printf("HX300X   ...data_ps Code = %4d, read_ps_k = %4d  ", data_ps,read_ps_k);//理论结构光
    //printf("HX300X   ");
    cali_process |= CALI_STEP3;
	
	//data_ps:灰卡增量值 ，read_ps_k：理论结构光感值
	//return data_ps,read_ps_k; //data_ps>800,read_ps_k<2600
}
//--------------------------------------------------------------------
void Calibration_Fourth_RemoveGreyCard(void *pEvt)	 
{//撤灰卡 	
	uint16_t str_diff = 0;
	uint8_t str_diff_t = 1;   //difference of struct light 
    uint8_t h_thod_t = 1;     //high threshold test
    uint8_t l_thod_t = 1;     //low threshold test

	typedef struct
	{
		uint8_t status;
		uint8_t data_reg10;
		uint8_t data_reg12;
		uint8_t data_reg13;
		uint16_t data_h_thod;
		uint16_t data_str_diff;
		uint16_t data_read_ps3;
	}PACKED RSP;

	RSP* pRsp = (RSP*)pEvt;

	TYHX_Write_REG(0x00, 0x01);
    TYHX_Write_REG(0x10, leddr);
    TYHX_Write_REG(0x11, 0xF7);
    TYHX_Write_REG(0x12, gain_r0x12); //90
    TYHX_Write_REG(0x13, reg_13_new);
    TYHX_Write_REG(0x00, 0x0A);
    delay_ms(100);

    read_ps3 = get_ps_data(35);
    //
    if(read_ps3 > read_ps_k)
    {
        str_diff = read_ps3-read_ps_k;
    }
    else
    {
        str_diff = read_ps_k-read_ps3;
    }
    printf("HX300X    !!! str_diff = %4d ",str_diff);

    //
    H_THOD =  read_ps2;
    L_THOD =  H_THOD-300;  //出耳阈值
    
    Reg10 =TYHX_Read_REG(0x10);
    //Reg11 =TYHX_Read_REG(0x11);
    Reg12 =TYHX_Read_REG(0x12);
    Reg13 =TYHX_Read_REG(0x13);

	pRsp->data_reg10 = Reg10;
	pRsp->data_reg12 = Reg12;
	pRsp->data_reg13 = Reg13;
	pRsp->data_h_thod = H_THOD;
	pRsp->data_str_diff = str_diff;
	pRsp->data_read_ps3 = (uint16_t)read_ps3;
	
    //str_diff < 300, read_ps3<2600, H_THOD>read_ps3+700, read_ps3==str_data
    printf("HX300X   ...R10 = 0x%x,R12 = 0x%x,R13 = 0x%x, THOD_H = %d, str_diff = %d, str_data = %d", Reg10,Reg12,Reg13,H_THOD, str_diff, read_ps3);
	cali_process |= CALI_STEP4;
}
//====================================================================
uint8_t Calibration_Fifth_SaveDataToFlash(void)	
{//注：将以下参数保存到主控的flash里
	//Flash.Reg10 = Reg10;
	//Flash.Reg12 = Reg12;
	//Flash.Reg13 = Reg13;
	//Flash.H_THOD = H_THOD;
	//Flash.L_THOD = L_THOD;	
	//Flash.str_data = read_ps3;
	uint8_t ret = 0;
	if(cali_process){
		app_nvkey_hx300x_setting_save(Reg10, Reg12, Reg13, H_THOD, L_THOD, read_ps3);
		printf("HX300X   ...Save Success ");
		ret = 1;
	}else{
		printf("HX300X   ...calibration no complete, cannot save!!!");
		ret = 0;
	}

	HX300x_init();//re-init after calibration

	return ret;
}
//--------------------------------------------------------------------

#define SPP_DEBUG_BUFFER_SIZE (128)
static uint8_t debug_buffer[SPP_DEBUG_BUFFER_SIZE];
static TimerHandle_t xBsp_ir_read_timer;
static uint8_t timer_debug = 0;

void HX300X_read_all_reg(void)
{
	hal_i2c_status_t ret = HAL_I2C_STATUS_OK;
	
	uint8_t reg_00;
	uint8_t reg_01;
	uint8_t reg_02;
	uint8_t reg_03;
	uint8_t reg_04;
	uint8_t reg_05;
	uint8_t reg_06;
	uint8_t reg_09;
	uint8_t reg_10;
	uint8_t reg_11;
	uint8_t reg_12;
	uint8_t reg_13;
	uint8_t reg_16;
	uint8_t reg_20;
	uint8_t reg_21;

	ret |= HX300X_reg_ctr_read(0x00, &reg_00);
	ret |= HX300X_reg_ctr_read(0x01, &reg_01);
	ret |= HX300X_reg_ctr_read(0x02, &reg_02);
	ret |= HX300X_reg_ctr_read(0x03, &reg_03);
	ret |= HX300X_reg_ctr_read(0x04, &reg_04);
	ret |= HX300X_reg_ctr_read(0x05, &reg_05);
	ret |= HX300X_reg_ctr_read(0x06, &reg_06);
	ret |= HX300X_reg_ctr_read(0x06, &reg_09);	
	ret |= HX300X_reg_ctr_read(0x10, &reg_10);
	ret |= HX300X_reg_ctr_read(0x11, &reg_11);
	ret |= HX300X_reg_ctr_read(0x12, &reg_12);
	ret |= HX300X_reg_ctr_read(0x13, &reg_13);
	ret |= HX300X_reg_ctr_read(0x16, &reg_16);
	ret |= HX300X_reg_ctr_read(0x20, &reg_20);
	ret |= HX300X_reg_ctr_read(0x21, &reg_21);

	printf("HX300X all reg data read  read_status=%d, reg_00=0x%x, reg_01=0x%x, reg_02=0x%x, reg_03=0x%x, reg_04=0x%x, reg_05=0x%x, reg_06=0x%x, reg_10=0x%x\r\n", 
	ret, reg_00, reg_01, reg_02, reg_03, reg_04, reg_05, reg_06, reg_10);
	printf("HX300X all reg data read  read_status=%d, reg_09=0x%x, reg_11=0x%x, reg_12=0x%x,reg_13=0x%x, reg_16=0x%x, reg_20=0x%x, reg_21=0x%x \r\n", 
	ret, reg_09, reg_11, reg_12, reg_13, reg_16, reg_20, reg_21);
	memset(debug_buffer, 0x0, SPP_DEBUG_BUFFER_SIZE);
	snprintf((char *)debug_buffer,
			 SPP_DEBUG_BUFFER_SIZE,
			 "HX300X all reg data read  read_status=%d, reg_00=0x%x, reg_01=0x%x, reg_02=0x%x, reg_03=0x%x, reg_04=0x%x, reg_05=0x%x, reg_06=0x%x, reg_10=0x%x, reg_11=0x%x, reg_12=0x%x,reg_13=0x%x, reg_16=0x%x, reg_20=0x%x, reg_21=0x%x \r\n", 
			ret, reg_00, reg_01, reg_02, reg_03, reg_04, reg_05, reg_06, reg_10, reg_11, reg_12, reg_13, reg_16, reg_20, reg_21);	

	app_spp_debug_print((char *)debug_buffer, strlen((char *)debug_buffer));	

}

void hx300x_debug_ps_data(void)
{
    uint8_t ps_data_h = 0;
    uint8_t ps_data_l = 0;
    uint16_t ps_dataTemp1 = 0;
    
    ps_data_h = TYHX_Read_REG(0x08);
    ps_data_l = TYHX_Read_REG(0x07);
    
    ps_dataTemp1 = (ps_data_h << 8) + ps_data_l;  
	
    printf("HX300X  read ps data=%d  ",ps_dataTemp1);
	memset(debug_buffer, 0x0, SPP_DEBUG_BUFFER_SIZE);
	snprintf((char *)debug_buffer,
			 SPP_DEBUG_BUFFER_SIZE,
			 "HX300X  read ps data=%d\r\n", ps_dataTemp1);	
	
	app_spp_debug_print((char *)debug_buffer, strlen((char *)debug_buffer));	
}   



void hx300x_spp_stop_log(void)
{
    xTimerStop(xBsp_ir_read_timer, 0);
    timer_debug = 0;
}

void hx300x_spp_open_log(void)
{

    if (timer_debug ==0)
    {
		xBsp_ir_read_timer = xTimerCreate("inear debug timer", 1000, pdTRUE, NULL, hx300x_debug_ps_data);
		if (xTimerStart(xBsp_ir_read_timer, 0) != pdPASS) {
			APPS_LOG_MSGID_E("inear debug timer xTimerStart fail\n", 0);
		}
        timer_debug = 1;
    }
}


void hx300x_Set_Thres(uint16_t h_thrd, uint16_t l_thrd)
{
    TYHX_Write_REG(0x04, (uint8_t) (h_thrd>>4));
	TYHX_Write_REG(0x05, (uint8_t) (l_thrd>>4));
    uint8_t r0x02 = TYHX_Read_REG(0x02);
	r0x02 = ((uint8_t)(h_thrd>>6)&0xc0)|(r0x02&0x30);       
    TYHX_Write_REG(0x02, r0x02);  
    	
}
void hx300x_Set_cur(void)
{
	uint16_t reg_10 = 0;
	uint8_t reg_13 = 0;

	app_nvkey_hx300x_cur_read(&reg_10, &reg_13);
	if(reg_10 == 0 || reg_13 == 0)
	{
		reg_10 = leddr;
		reg_13 = reg_13_new;
	}	
    TYHX_Write_REG(0x10, reg_10);       //Flash 保存的leddr
    TYHX_Write_REG(0x13, reg_13);  //Flash 保存的reg_13_new 	
}

void hx300x_Set_gain(void)
{	
	uint8_t reg_12 = 0;
	
	reg_12 = app_nvkey_hx300x_Gain_read();
	if(reg_12 == 0)
	{
		reg_12 = gain_r0x12;
	}
	
    TYHX_Write_REG(0x12, reg_12);  //Flash 保存的gain
}
void HX300x_init(void)
{	
	uint16_t h_thrd =0;
	uint16_t l_thrd=0;

    TYHX_Write_REG(0x00, 0x01);
    TYHX_Write_REG(0x01, 0xC1);
    TYHX_Write_REG(0x03, 0x66);

	app_nvkey_hx300x_Thres_read(&h_thrd, &l_thrd);
	if(h_thrd == 0 || l_thrd == 0)
	{
		h_thrd = H_THOD;
		l_thrd = L_THOD;
	}
	
    hx300x_Set_Thres(h_thrd, l_thrd);  
    hx300x_Set_cur();
    hx300x_Set_gain();
    TYHX_Write_REG(0x06, 0x51); 
    TYHX_Write_REG(0x11, 0xF7);     
    
    TYHX_Write_REG(0x16, 0x20);
    TYHX_Write_REG(0x20, 0x28);
    TYHX_Write_REG(0x21, 0xF3);
    TYHX_Write_REG(0x00, 0x0A);
}


uint16_t hx300x_int_handle(void)
{
//方法一：直接读INT的高低电平  （推荐方法）      
	// INT 为高电平 ===> 入耳 （In ear）
	// INT 为低电平 ===> 出耳 （Out ear）   
	hal_gpio_data_t pin_status = 0;
	hal_gpio_status_t status = hal_gpio_get_input(PIO_HX300X_INT, &pin_status);

	if ( status == HAL_GPIO_STATUS_OK)
	{
		if(pin_status & 0x0001)
			return 1; // in ear
		else
			return 0; // out ear
	}

	return 0;



//方法二：读ps data 和高低阈值对比	
/*    
    uint16_t ps_data = 0;

    ps_data = hx300x_read_ps_data();
    //printf("HX300X  PS Data=%d ",ps_data);
    if(ps_data > H_THOD)
      {
           //in_ear_flag = 1;                       // In ear	
           //SEGGER_RTT_printf(0,"*************************************hx3002 In ear  ");
      }
    else if(ps_data < L_THOD)
     {
          //in_ear_flag = 0;                     // Out ear
          //SEGGER_RTT_printf(0,"=====================================hx3002 Out ear  ");
     }
*/
}

void hx300x_PowerDown(void)
{
    // Or disable EN of LDO
    TYHX_Write_REG(0x00, 0x05);	 		                                //bit1:PEN  关闭激光电源
}

void hx300x_PowerOn(void)
{
    // enable LDO
    TYHX_Write_REG(0x00, 0x0A);	 		                                //bit1:PEN  打开激光电源
}


uint16_t hx300x_read_ps_data(void)
{
    uint8_t ps_data_h = 0;
    uint8_t ps_data_l = 0;
    uint16_t ps_dataTemp1 = 0;
    
    ps_data_h = TYHX_Read_REG(0x08);
    ps_data_l = TYHX_Read_REG(0x07);
    
    ps_dataTemp1 = (ps_data_h << 8) + ps_data_l;  
    printf("HX300X  read ps data=%d  ",ps_dataTemp1);			
    return ps_dataTemp1;
}   

bool bsp_is_hx300x_using(void)
{
	uint8_t i = 0;
	bool ret = 0;

	for(i=0;i<5;i++)
	{
		uint8_t data = 0;
		
		slave_addr = HX300X_ID;
		if(HAL_I2C_STATUS_OK != HX300X_I2C_Read(0x00, &data, 1))
		{
			slave_addr = HX300X_ID_2;
			if(HAL_I2C_STATUS_OK == HX300X_I2C_Read(0x00, &data, 1))
			{
				ret = 1;
			}
		}
		else
		{
			ret = 1;
		}

		if(ret)
		{
			APPS_LOG_MSGID_I("psensor hx300x:iic checking reg00 = 0x%x, addr=0x%x", 2, data, slave_addr);
			break;			
		}
		else
		{
			slave_addr = HX300X_ID;
		}
	}

	return ret;
}

static void bsp_hx300x_callback(void *data)
{

#ifdef MTK_IN_EAR_FEATURE_ENABLE
	uint16_t *p_wear_status = (uint16_t *)pvPortMalloc(sizeof(uint16_t)); /* free by ui shell */
	
	*p_wear_status = hx300x_int_handle();
	
	ui_shell_remove_event(EVENT_GROUP_UI_SHELL_APP_INTERACTION, APPS_EVENTS_INTERACTION_UPDATE_IN_EAR_STA_EFFECT);
	ui_shell_send_event(true, EVENT_PRIORITY_HIGNEST, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
								 APPS_EVENTS_INTERACTION_UPDATE_IN_EAR_STA_EFFECT, (void *)p_wear_status, sizeof(bool),
								 NULL, 500);
#endif

    /* Clear interrupt flag to receive new interrupt */
    hal_eint_unmask(EINT_PSENSOR);						
}


static void bsp_hx300x_HW_config(void)
{
	//config power & reset	
	//TODO:

    /* Config i2c peripheral */
    hal_i2c_config_t i2c_config;
	
    i2c_config.frequency = HAL_I2C_FREQUENCY_100K;
    hal_i2c_status_t ret = hal_i2c_master_init(BSP_HX300X_I2C_PORT, &i2c_config);	
}


void bsp_component_hx300x_init(void)
{		
    hal_eint_config_t eint_config;
	hal_eint_status_t ret; 

	//bsp_hx300x_HW_config();
	
	ret = hal_eint_mask(EINT_PSENSOR);

	eint_config.debounce_time = 20;
	eint_config.trigger_mode  = HAL_EINT_EDGE_FALLING_AND_RISING;

	ret = hal_eint_init(EINT_PSENSOR, &eint_config);

	ret = hal_eint_register_callback((hal_eint_number_t)EINT_PSENSOR, (hal_eint_callback_t)bsp_hx300x_callback, NULL);
	APPS_LOG_MSGID_I("hx300x:hal_eint_register_callback ret = %d", 1, ret);

	hal_eint_unmask(EINT_PSENSOR);

	HX300x_init();

	if(0x01 == app_nvkey_get_hx300x_log_status())
		hx300x_spp_open_log();	

}


