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
#include <stdbool.h>

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
FATFS fs;
static bool module_initialized = false;

 /* ============================================================================
 * PRIVATE FUNCTION PROTOTYPES
 * ==========================================================================*/



 /* ============================================================================
 * PUBLIC FUNCTIONS
 * ==========================================================================*/
/**
 * @brief  Initialize variables used in the sd_hander module
 * @return 0 on success, 1 on failure
 */
void sd_handler_init(UART_HandleTypeDef* rt){
    // store uart handle
    if (rt != NULL){
        uart = rt;
        char msg[32];
        snprintf(msg, sizeof(msg), "SD handler initialized\r\n");
        HAL_UART_Transmit(uart, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
    }
    // mount the filesystem
    FRESULT res;
    res = f_mount(&fs, "/", 1);
    if (res != FR_OK) {
        char msg[64];
        snprintf(msg, sizeof(msg), "Failed to mount filesystem: %d\r\n", res);
        HAL_UART_Transmit(uart, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
    } else {
        char msg[64];
        snprintf(msg, sizeof(msg), "Filesystem mounted successfully\r\n");
        HAL_UART_Transmit(uart, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
        module_initialized = true;
    }
}

/**
 * @brief  Mount filesystem and read all filenames in root directory.
 * @return void
 */
void sd_ls(void){
    
    DIR dir;
    FILINFO fno;
    FRESULT res;

    if (!module_initialized){
        return;
    }

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
    // f_mount(NULL, "/", 1);
}

/**
 * @brief  Print the first N bytes of the given filename (if it exists)
 * @return void
 */
void sd_head(const char* filename, int max_num_bytes){
    
    FRESULT res;
    FIL fp;
    res = f_open(&fp, filename, FA_READ);
    if (res != FR_OK){
        char msg[32];
        snprintf(msg, sizeof(msg), "could not open %s: %d\r\n", filename, res);
        HAL_UART_Transmit(uart, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
        return;
    }
    char buf[max_num_bytes];
    UINT bytes_read;
    res = f_read(&fp, buf, (max_num_bytes-1), &bytes_read);
    if (res != FR_OK){
        char msg[32];
        snprintf(msg, sizeof(msg), "could not read %s: %d\r\n", filename, res);
        HAL_UART_Transmit(uart, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
        return;
    }
    
    HAL_UART_Transmit(uart, (uint8_t*)buf, bytes_read, HAL_MAX_DELAY);
    
}

/* ============================================================================
 * PRIVATE FUNCTION DEFINITIONS
 * ==========================================================================*/
