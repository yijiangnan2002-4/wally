/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2014        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "ffconf.h"
#include "diskio.h"		/* FatFs lower layer API */
#include <stdbool.h>

#ifdef MTK_FATFS_ON_SERIAL_NAND
#include "diskio_snand.h"
#endif

#ifdef HAL_RTC_MODULE_ENABLED
#include "hal_rtc.h"
#endif

//#if defined(MTK_FATFS_ON_SPI_SD) || defined(HAL_SD_MODULE_ENABLED)
#include "diskio_sd.h"
//#endif

DSTATUS Drive0_Stat = STA_NOINIT;	/* Physical drive status */
DSTATUS Drive1_Stat = STA_NOINIT;	/* Physical drive status */


/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status(
    BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
    if (pdrv) {
        return Drive1_Stat;
    } else {
        return Drive0_Stat;
    }
}


/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize(
    BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
    if(!pdrv) {
        return sd_disk_initialize(pdrv);
    } else {
#ifdef MTK_FATFS_ON_SERIAL_NAND
        return serial_nand_disk_initialize(pdrv);
#else
        return RES_PARERR;
#endif
   }


}


/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read(
    BYTE pdrv,		/* Physical drive nmuber to identify the drive */
    BYTE *buff,		/* Data buffer to store read data */
    DWORD sector,	/* Sector address in LBA */
    UINT count		/* Number of sectors to read */
)
{
    if(!pdrv) {
        return sd_disk_read(pdrv, buff, sector, count);
    } else {
#ifdef MTK_FATFS_ON_SERIAL_NAND
        return serial_nand_disk_read(pdrv, buff, sector, count);
#else
        return RES_PARERR;
#endif
   }
}


/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if _USE_WRITE
DRESULT disk_write(
    BYTE pdrv,			/* Physical drive nmuber to identify the drive */
    const BYTE *buff,	/* Data to be written */
    DWORD sector,		/* Sector address in LBA */
    UINT count			/* Number of sectors to write */
)
{
    if(!pdrv) {
        return sd_disk_write(pdrv, buff, sector, count);
    } else {
#ifdef MTK_FATFS_ON_SERIAL_NAND
        return serial_nand_disk_write(pdrv, buff, sector, count);
#else
        return RES_PARERR;
#endif
   }
}
#endif



/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

#if _USE_IOCTL
DRESULT disk_ioctl(
    BYTE pdrv,		/* Physical drive nmuber (0..) */
    BYTE cmd,		/* Control code */
    void *buff		/* Buffer to send/receive control data */
)
{
    if(!pdrv) {
        return sd_disk_ioctl(pdrv, cmd, buff);
    } else {
#ifdef MTK_FATFS_ON_SERIAL_NAND
        return serial_nand_disk_ioctl(pdrv, cmd, buff);
#else
        return RES_PARERR;
#endif
   }
}
#endif




#if !_FS_READONLY && !_FS_NORTC
DWORD get_fattime(void)
{
    DWORD cur_time;
    // get the current time
    /*   
        bit31:25 
        Year origin from the 1980 (0..127, e.g. 37 for 2017) 
        bit24:21 
        Month (1..12) 
        bit20:16 
        Day of the month (1..31) 
        bit15:11 
        Hour (0..23) 
        bit10:5 
        Minute (0..59) 
        bit4:0 Second / 2 (0..29, e.g. 25 for 50) 

        */

/*
typedef struct {
    uint8_t rtc_sec;              // Seconds after minutes   - [0,59]
    uint8_t rtc_min;             // Minutes after the hour   - [0,59]
    uint8_t rtc_hour;            // Hours after midnight     - [0,23]  
    uint8_t rtc_day;             // Day of the month          - [1,31]  
    uint8_t rtc_mon;            // Months                        - [1,12]  
    uint8_t rtc_week;           // Days in a week             - [0,6]   
    uint8_t rtc_year;            // Years                     - [0,127] 
    uint16_t rtc_milli_sec;    // Millisecond value, when in time API, this represents the read only register rtc_int_cnt
*/
#ifdef HAL_RTC_MODULE_ENABLED
    hal_rtc_status_t status;
    hal_rtc_time_t timer;
    status = hal_rtc_get_time(&timer);
    if (status == HAL_RTC_STATUS_OK) {
        cur_time = ((timer.rtc_sec))/2;
        cur_time |= ((timer.rtc_min)<<5);
        cur_time |= ((timer.rtc_hour)<<11);
        cur_time |= ((timer.rtc_day)<<16);
        cur_time |= ((timer.rtc_mon)<<21);
        cur_time |= ((timer.rtc_year)<<25);
    } else {
        cur_time = 0;
    }
#else
    cur_time = 0;
#endif

#if debug_rtc_time
    printf("sec = %x \r\n", (unsigned int)(timer.rtc_sec));
    printf("min = %x \r\n", (unsigned int)(timer.rtc_min));
    printf("hour = %x \r\n", (unsigned int)(timer.rtc_hour));
    printf("day = %x \r\n", (unsigned int)(timer.rtc_day));
    printf("mon = %x \r\n", (unsigned int)(timer.rtc_mon));
    printf("year = %x \r\n", (unsigned int)(timer.rtc_year));
    printf("cur_time = %x \r\n", (unsigned int)cur_time);
#endif
    return cur_time; //return the current time instead of 0 if current time is got
}
#endif


