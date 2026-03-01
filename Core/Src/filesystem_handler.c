/**
  ******************************************************************************
  * @file           : filesystem_handler.c
  * @brief          : see filesystem_handler.h for description
  ******************************************************************************
*/

/* ============================================================================
 * INCLUDES
 * ==========================================================================*/
#include <string.h>
#include <stdio.h>
#include "filesystem_handler.h"

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
static void parse_wav_file(const uint8_t *buf, wav_header *wh);

 /* ============================================================================
 * PUBLIC FUNCTIONS
 * ==========================================================================*/
/**
 * @brief  Initialize variables used in the sd_hander module
 * @return 0 on success, 1 on failure
 */
void filesystem_handler_init(){
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

/**
 * @brief  Open the given file (if it exists) in read only mode,
 *         then parse the metadata
 *         TODO: verify that it's really a wav file
 * @param  filename path and name of the wav file
 * @param  wh wav_header struct containing the file's metadata
 * @return 0 on success, 1 if failed to parse
 */
int get_wav_metadata(const char *filename, wav_header *wh){
    if (!is_fs_mounted) return 1;
    
    FRESULT res;
    FIL fp;
    res = f_open(&fp, filename, FA_READ);
    if (res != FR_OK){
        uart_printf("[get_wav_metadata] - ERROR: Could not open %s: %d\r\n", filename, res);
        return 1;
    }
    uint8_t buf[WAV_HEADER_SIZE];
    UINT bytes_read;
    res = f_read(&fp, buf, WAV_HEADER_SIZE, &bytes_read);
    if (res != FR_OK){
        uart_printf("[get_wav_metadata] - ERROR: Could not read %s: %d\r\n", filename, res);
        return 1;
    }

    parse_wav_file(buf, wh);
    return 0;
}


/* ============================================================================
 * PRIVATE FUNCTION DEFINITIONS
 * ==========================================================================*/

 /**
 * @brief  Parse the header of a wav file and pack a 
 *         wav_header struct
 * @param  buf - buffer containing at least the first 44 raw bytes of the file
 * @param  wh - pointer to wav_header struct to store the metadata
 * @return void
 */
static void parse_wav_file(const uint8_t *buf, wav_header *wh){
    wh->file_size = combine_32(buf, 4);
    wh->fmt_data_length = combine_32(buf, 16);
    wh->fmt_type = combine_16(buf, 20);
    wh->channels = combine_16(buf, 22);
    wh->sample_rate = combine_32(buf, 24);
    wh->bytes_per_s = combine_32(buf, 28);
    wh->block_align = combine_16(buf, 32);
    wh->bits_per_sample = combine_16(buf, 34);
    wh->data_size = combine_32(buf, 40);
    uart_printf("WAV header:\n");
    uart_printf(" - file_size: %d\n", wh->file_size);
    uart_printf(" - fmt_data_length: %d\n", wh->fmt_data_length);
    uart_printf(" - fmt_type: %d\n", wh->fmt_type);
    uart_printf(" - channels: %d\n", wh->channels);
    uart_printf(" - sample_rate: %d\n", wh->sample_rate);
    uart_printf(" - bytes_per_s: %d\n", wh->bytes_per_s);
    uart_printf(" - block_align: %d\n", wh->block_align);
    uart_printf(" - bits_per_sample: %d\n", wh->bits_per_sample);
    uart_printf(" - data_size: %d\n", wh->data_size);
}