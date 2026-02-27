/**
  ******************************************************************************
  * @file           : sd_handler.c
  * @brief          : see sd_handler.h for description
  ******************************************************************************
*/

/* ============================================================================
 * INCLUDES
 * ==========================================================================*/
#include <string.h>
#include <stdio.h>
#include "sd_handler.h"
#include "utils.h"

/* ============================================================================
 * PRIVATE MACROS & CONSTANTS
 * ==========================================================================*/
#define ROOT_DIR "/"

/* ============================================================================
 * PRIVATE TYPES & ENUMERATIONS
 * ==========================================================================*/


/* ============================================================================
 * PRIVATE VARIABLES (Module-scope globals)
 * ==========================================================================*/
static FATFS fs;
static bool is_fs_mounted = false;

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
void sd_handler_init(){
    // mount the filesystem
    FRESULT res;
    res = f_mount(&fs, ROOT_DIR, 1);
    if (res != FR_OK) {
        uart_printf("Failed to mount filesystem: %d\r\n", res);
    } else {
        uart_printf("Filesystem mounted successfully\r\n", res);
        is_fs_mounted = true;
    }
}

/**
 * @brief  Read all filenames in root directory.
 * @return void
 */
void sd_ls(void){
    
    DIR dir;
    FILINFO fno;
    FRESULT res;

    if (!is_fs_mounted) return;

    // Open root directory
    res = f_opendir(&dir, ROOT_DIR);
    if (res == FR_OK) {
        while (1) {
            res = f_readdir(&dir, &fno);
            // Empty name means end of directory
            if (res != FR_OK || fno.fname[0] == 0) break;
            // Skip hidden and system files if you want
            if (fno.fattrib & (AM_HID | AM_SYS)) continue;

            uart_printf("%s %s\r\n",
                (fno.fattrib & AM_DIR) ? "[DIR] " : "[FILE]", fno.fname);
        }
        f_closedir(&dir);
    } else {
        uart_printf("opendir failed: %d\r\n", res);
    }
}

/**
 * @brief  Open the given file (if it exists) in read only mode then
 *         print the first N bytes
 * @return void
 */
void sd_head(const char* filename, int max_num_bytes, bool hexdump){
    
    if (!is_fs_mounted) return;
    
    FRESULT res;
    FIL fp;
    res = f_open(&fp, filename, FA_READ);
    if (res != FR_OK){
        uart_printf("Could not open %s: %d\r\n", filename, res);
        return;
    }
    char buf[max_num_bytes];
    UINT bytes_read;
    res = f_read(&fp, buf, (max_num_bytes-1), &bytes_read);
    if (res != FR_OK){
        uart_printf("Could not read %s: %d\r\n", filename, res);
        return;
    }
    
    // print filename header 
    uart_printf("\r\n%s:\r\n", filename);
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
            uart_printf("%08X: %-48s %s\r\n", offset, hex_line, ascii_line);
            offset += line_len;
        }
    } else {
        uart_printf("%s\r\n", buf);
    }
}


/* ============================================================================
 * PRIVATE FUNCTION DEFINITIONS
 * ==========================================================================*/
