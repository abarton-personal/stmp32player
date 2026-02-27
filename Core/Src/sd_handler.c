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
 * @brief  Open the given file (if it exists) in read only mode then
 *         print the first N bytes
 * @return void
 */
void sd_head(const char* filename, int max_num_bytes, bool hexdump){
    
    FRESULT res;
    FIL fp;
    res = f_open(&fp, filename, FA_READ);
    if (res != FR_OK){
        char msg[32];
        snprintf(msg, sizeof(msg), "Could not open %s: %d\r\n", filename, res);
        HAL_UART_Transmit(uart, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
        return;
    }
    char buf[max_num_bytes];
    UINT bytes_read;
    res = f_read(&fp, buf, (max_num_bytes-1), &bytes_read);
    if (res != FR_OK){
        char msg[32];
        snprintf(msg, sizeof(msg), "Could not read %s: %d\r\n", filename, res);
        HAL_UART_Transmit(uart, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
        return;
    }
    
    // print ascii characters or raw hex values depending on hexdump arg
    if (hexdump){
        int offset = 0;
        while (offset < bytes_read){
            int line_len = (bytes_read - offset) > 16 ? 16 : (bytes_read - offset);
            char hex_line[48] = {0};
            char ascii_line[17] = {0};
            for (int i=0; i<line_len; i++){
                uint8_t byte = buf[offset + i];
                snprintf(hex_line + (i*3), sizeof(hex_line) - (i*3), "%02X ", byte);
                ascii_line[i] = (byte >= 32 && byte <= 126) ? byte : '.';
            }
            char line_msg[192];
            snprintf(line_msg, sizeof(line_msg), "%08X: %-48s %s\r\n", offset, hex_line, ascii_line);
            HAL_UART_Transmit(uart, (uint8_t*)line_msg, strlen(line_msg), HAL_MAX_DELAY);
            offset += line_len;
        }
    } else {
        HAL_UART_Transmit(uart, (uint8_t*)buf, bytes_read, HAL_MAX_DELAY);
    }
}


/* ============================================================================
 * PRIVATE FUNCTION DEFINITIONS
 * ==========================================================================*/
