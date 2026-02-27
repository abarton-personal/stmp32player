/* USER CODE BEGIN Header */
/**
 ******************************************************************************
  * @file    user_diskio.c
  * @brief   This file includes a diskio driver skeleton to be completed by the user.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
 /* USER CODE END Header */

#ifdef USE_OBSOLETE_USER_CODE_SECTION_0
/*
 * Warning: the user section 0 is no more in use (starting from CubeMx version 4.16.0)
 * To be suppressed in the future.
 * Kept to ensure backward compatibility with previous CubeMx versions when
 * migrating projects.
 * User code previously added there should be copied in the new user sections before
 * the section contents can be deleted.
 */
/* USER CODE BEGIN 0 */
/* USER CODE END 0 */
#endif

/* USER CODE BEGIN DECL */

#include <string.h>
#include "ff_gen_drv.h"
#include "stm32l4xx_hal.h"
#include "main.h"

//extern SPI_HandleTypeDef hspi2;

static volatile DSTATUS Stat = STA_NOINIT;
static int SD_Type = 0;

static void CS_Select(void)   { HAL_GPIO_WritePin(SD_CS_GPIO_Port, SD_CS_Pin, GPIO_PIN_RESET); }
static void CS_Deselect(void) { HAL_GPIO_WritePin(SD_CS_GPIO_Port, SD_CS_Pin, GPIO_PIN_SET);   }

static uint8_t SPI_TxRx(uint8_t data) {
    uint8_t rx;
    HAL_SPI_TransmitReceive(&hspi2, &data, &rx, 1, HAL_MAX_DELAY);
    return rx;
}

static void SPI_Tx(const uint8_t *buf, uint16_t len) {
    HAL_SPI_Transmit(&hspi2, (uint8_t *)buf, len, HAL_MAX_DELAY);
}

static void SPI_Rx(uint8_t *buf, uint16_t len) {
    memset(buf, 0xFF, len);
    HAL_SPI_Receive(&hspi2, buf, len, HAL_MAX_DELAY);
}

static uint8_t SD_SendCmd(uint8_t cmd, uint32_t arg) {
    uint8_t crc = 0x01;
    if (cmd == 0) crc = 0x95;
    if (cmd == 8) crc = 0x87;

    uint8_t pkt[6] = {
        0x40 | cmd,
        (arg >> 24) & 0xFF,
        (arg >> 16) & 0xFF,
        (arg >>  8) & 0xFF,
        (arg >>  0) & 0xFF,
        crc
    };

    SPI_TxRx(0xFF);
    SPI_Tx(pkt, 6);

    uint8_t res;
    for (int i = 0; i < 8; i++) {
        res = SPI_TxRx(0xFF);
        if (!(res & 0x80)) break;
    }
    return res;
}

static DSTATUS SD_SPI_Init(void) {
    uint8_t res, buf[4];

    CS_Deselect();
    for (int i = 0; i < 10; i++) SPI_TxRx(0xFF);

    CS_Select();
    res = SD_SendCmd(0, 0);
    CS_Deselect();
    if (res != 0x01) return STA_NOINIT;

    CS_Select();
    res = SD_SendCmd(8, 0x000001AA);
    if (res == 0x01) {
        for (int i = 0; i < 4; i++) buf[i] = SPI_TxRx(0xFF);
        CS_Deselect();

        if (buf[2] == 0x01 && buf[3] == 0xAA) {
            uint32_t timeout = HAL_GetTick();
            do {
                CS_Select(); SD_SendCmd(55, 0);              CS_Deselect();
                CS_Select(); res = SD_SendCmd(41, 0x40000000); CS_Deselect();
            } while (res != 0x00 && (HAL_GetTick() - timeout) < 2000);

            if (res != 0x00) return STA_NOINIT;

            CS_Select();
            SD_SendCmd(58, 0);
            for (int i = 0; i < 4; i++) buf[i] = SPI_TxRx(0xFF);
            CS_Deselect();
            SD_Type = (buf[0] & 0x40) ? 3 : 2;
        }
    } else {
        CS_Deselect();
        uint32_t timeout = HAL_GetTick();
        do {
            CS_Select(); SD_SendCmd(55, 0);       CS_Deselect();
            CS_Select(); res = SD_SendCmd(41, 0); CS_Deselect();
        } while (res != 0x00 && (HAL_GetTick() - timeout) < 2000);
        SD_Type = (res == 0x00) ? 1 : 0;
        if (SD_Type == 0) return STA_NOINIT;
    }

    return 0;
}

static DRESULT SD_ReadBlock(uint8_t *buf, uint32_t sector) {
    if (SD_Type != 3) sector *= 512;

    CS_Select();
    if (SD_SendCmd(17, sector) != 0x00) { CS_Deselect(); return RES_ERROR; }

    uint32_t timeout = HAL_GetTick();
    while (SPI_TxRx(0xFF) != 0xFE) {
        if (HAL_GetTick() - timeout > 500) { CS_Deselect(); return RES_ERROR; }
    }

    SPI_Rx(buf, 512);
    SPI_TxRx(0xFF);
    SPI_TxRx(0xFF);

    CS_Deselect();
    SPI_TxRx(0xFF);
    return RES_OK;
}

/* USER CODE END DECL */
/* USER CODE END DECL */

/* Private function prototypes -----------------------------------------------*/
DSTATUS USER_initialize (BYTE pdrv);
DSTATUS USER_status (BYTE pdrv);
DRESULT USER_read (BYTE pdrv, BYTE *buff, DWORD sector, UINT count);
#if _USE_WRITE == 1
  DRESULT USER_write (BYTE pdrv, const BYTE *buff, DWORD sector, UINT count);
#endif /* _USE_WRITE == 1 */
#if _USE_IOCTL == 1
  DRESULT USER_ioctl (BYTE pdrv, BYTE cmd, void *buff);
#endif /* _USE_IOCTL == 1 */

Diskio_drvTypeDef  USER_Driver =
{
  USER_initialize,
  USER_status,
  USER_read,
#if  _USE_WRITE
  USER_write,
#endif  /* _USE_WRITE == 1 */
#if  _USE_IOCTL == 1
  USER_ioctl,
#endif /* _USE_IOCTL == 1 */
};

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Initializes a Drive
  * @param  pdrv: Physical drive number (0..)
  * @retval DSTATUS: Operation status
  */
DSTATUS USER_initialize (
	BYTE pdrv           /* Physical drive nmuber to identify the drive */
)
{
	/* USER CODE BEGIN INIT */
	Stat = SD_SPI_Init();
	return Stat;
	/* USER CODE END INIT */
}

/**
  * @brief  Gets Disk Status
  * @param  pdrv: Physical drive number (0..)
  * @retval DSTATUS: Operation status
  */
DSTATUS USER_status (
	BYTE pdrv       /* Physical drive number to identify the drive */
)
{
	/* USER CODE BEGIN STATUS */
	Stat = SD_Type ? 0 : STA_NOINIT;
	return Stat;
	/* USER CODE END STATUS */
}

/**
  * @brief  Reads Sector(s)
  * @param  pdrv: Physical drive number (0..)
  * @param  *buff: Data buffer to store read data
  * @param  sector: Sector address (LBA)
  * @param  count: Number of sectors to read (1..128)
  * @retval DRESULT: Operation result
  */
DRESULT USER_read (
	BYTE pdrv,      /* Physical drive nmuber to identify the drive */
	BYTE *buff,     /* Data buffer to store read data */
	DWORD sector,   /* Sector address in LBA */
	UINT count      /* Number of sectors to read */
)
{
	/* USER CODE BEGIN READ */
	for (UINT i = 0; i < count; i++) {
		DRESULT res = SD_ReadBlock(buff + (i * 512), sector + i);
		if (res != RES_OK) return res;
	}
	return RES_OK;
	/* USER CODE END READ */
}

/**
  * @brief  Writes Sector(s)
  * @param  pdrv: Physical drive number (0..)
  * @param  *buff: Data to be written
  * @param  sector: Sector address (LBA)
  * @param  count: Number of sectors to write (1..128)
  * @retval DRESULT: Operation result
  */
#if _USE_WRITE == 1
DRESULT USER_write (
	BYTE pdrv,          /* Physical drive nmuber to identify the drive */
	const BYTE *buff,   /* Data to be written */
	DWORD sector,       /* Sector address in LBA */
	UINT count          /* Number of sectors to write */
)
{
	/* USER CODE BEGIN WRITE */
	return RES_WRPRT;
	/* USER CODE END WRITE */
}
#endif /* _USE_WRITE == 1 */

/**
  * @brief  I/O control operation
  * @param  pdrv: Physical drive number (0..)
  * @param  cmd: Control code
  * @param  *buff: Buffer to send/receive control data
  * @retval DRESULT: Operation result
  */
#if _USE_IOCTL == 1
DRESULT USER_ioctl (
	BYTE pdrv,      /* Physical drive nmuber (0..) */
	BYTE cmd,       /* Control code */
	void *buff      /* Buffer to send/receive control data */
)
{
	/* USER CODE BEGIN IOCTL */
	switch (cmd) {
		case CTRL_SYNC:       return RES_OK;
		case GET_SECTOR_SIZE: *(WORD*)buff = 512; return RES_OK;
		case GET_BLOCK_SIZE:  *(DWORD*)buff = 1;  return RES_OK;
		default:              return RES_PARERR;
	}
	/* USER CODE END IOCTL */
}
#endif /* _USE_IOCTL == 1 */

