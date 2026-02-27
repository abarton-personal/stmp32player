/**
  ******************************************************************************
  * @file           : sd_handler.c
  * @brief          : see sd_handler.h for description
  ******************************************************************************
*/

/* ============================================================================
 * INCLUDES
 * ==========================================================================*/
#include "sd_handler.h"
#include <string.h>
#include <stdio.h>

/* ============================================================================
 * PRIVATE MACROS & CONSTANTS
 * ==========================================================================*/

/* ============================================================================
 * PRIVATE TYPES & ENUMERATIONS
 * ==========================================================================*/


/* ============================================================================
 * PRIVATE VARIABLES (Module-scope globals)
 * ==========================================================================*/
static UART_HandleTypeDef* uart = NULL;


 /* ============================================================================
 * PRIVATE FUNCTION PROTOTYPES
 * ==========================================================================*/



 /* ============================================================================
 * PUBLIC FUNCTIONS
 * ==========================================================================*/
/**
 * @brief  Initialize variables used in the sd_hander module
 * @return void
 */
void sd_handler_init(UART_HandleTypeDef* rt){
    if (rt != NULL){
        uart = rt;
        char msg[32];
        snprintf(msg, sizeof(msg), "SD handler initialized\r\n");
        HAL_UART_Transmit(uart, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
    }
}

/**
 * @brief  Mount filesystem and read all filenames in root directory.
 * @return void
 */
void sd_ls(void){
    FATFS fs;
    DIR dir;
    FILINFO fno;
    FRESULT res;

    //mount
    res = f_mount(&fs, "/", 1);
    if (res != FR_OK) {
        char msg[32];
        snprintf(msg, sizeof(msg), "Mount failed: %d\r\n", res);
        HAL_UART_Transmit(uart, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
    } else {
        // Open root directory
        res = f_opendir(&dir, "/");
        if (res == FR_OK) {
            while (1) {
                res = f_readdir(&dir, &fno);
                // Empty name means end of directory
                if (res != FR_OK || fno.fname[0] == 0) break;
                // Skip hidden and system files if you want
                if (fno.fattrib & (AM_HID | AM_SYS)) continue;

                char msg[64];
                snprintf(msg, sizeof(msg), "%s %s\r\n",
                    (fno.fattrib & AM_DIR) ? "[DIR] " : "[FILE]", fno.fname);
                HAL_UART_Transmit(uart, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
            }
            f_closedir(&dir);
        } else {
            char msg[32];
            snprintf(msg, sizeof(msg), "opendir failed: %d\r\n", res);
            HAL_UART_Transmit(uart, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
        }

        f_mount(NULL, "/", 1);
    }
}


/* ============================================================================
 * PRIVATE FUNCTION DEFINITIONS
 * ==========================================================================*/
