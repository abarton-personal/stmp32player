/**
  ******************************************************************************
  * @file           : filesystem_handler.h
  * @brief          : Header for filesystem_handler.c file.
  *                   This file is a utility wrapper for access to files on 
  *                   the SD card
  * @author         : AB
  * @date           : Feb 2026
  ******************************************************************************
*/

#ifndef FILESYSTEM_HANDLER_H
#define FILESYSTEM_HANDLER_H

#ifdef __cplusplus
 extern "C" {
#endif

/* ============================================================================
 * INCLUDES
 * ==========================================================================*/
#include <stdbool.h>
#include "fatfs.h"
#include "utils.h"

/* ============================================================================
 * PUBLIC MACROS & CONSTANTS
 * ==========================================================================*/


 /* ============================================================================
 * PUBLIC TYPES
 * ==========================================================================*/


 /* ============================================================================
 * PUBLIC FUNCTION DECLARATIONS
 * ==========================================================================*/

void filesystem_handler_init();
void sd_ls(void);
void sd_head(const char* filename, int max_num_bytes, bool hexdump);
int get_wav_metadata(const char *filename, wav_header *wh);

#ifdef __cplusplus
}
#endif

#endif /* end FILESYSTEM_HANDLER_H */
