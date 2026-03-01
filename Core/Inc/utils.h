/**
  ******************************************************************************
  * @file           : utils.h
  * @brief          : Header for filesystem_handler.c file.
  *                   This file contains generic macros and functions to help 
  *                   simplify the code base
  * @author         : AB
  * @date           : Feb 2026
  ******************************************************************************
*/

#ifndef UTILS_H
#define UTILS_H

#ifdef __cplusplus
 extern "C" {
#endif

/* ============================================================================
 * INCLUDES
 * ==========================================================================*/
#include <stdint.h>
#include "stm32l4xx_hal.h"

/* ============================================================================
 * PUBLIC MACROS & CONSTANTS
 * ==========================================================================*/
#define WAV_HEADER_SIZE 44
#define MAX_FILENAME_SIZE 32

 /* ============================================================================
 * PUBLIC TYPES
 * ==========================================================================*/
typedef struct wav_metadata {
    char path[MAX_FILENAME_SIZE]; 
    uint32_t file_size;
    uint32_t fmt_data_length;
    uint16_t fmt_type;
    uint16_t channels;
    uint32_t sample_rate;
    uint32_t bytes_per_s;
    uint16_t block_align;
    uint32_t bits_per_sample;
    uint32_t data_size;
} wav_metadata;

 /* ============================================================================
 * PUBLIC FUNCTION DECLARATIONS
 * ==========================================================================*/
void utils_init(UART_HandleTypeDef *uart);
void uart_printf(const char *fmt, ...);

/**
 * @brief  Recombines 4 little endian bytes into a single uint32
 *         Helper for parsing file headers.
 * @return void
 */
static inline uint32_t combine_32(const uint8_t *buf, size_t start_idx){
    return (uint32_t)(buf[start_idx+3] << 24 | buf[start_idx+2] << 16
        | buf[start_idx+1] << 8 | buf[start_idx]);
}

static inline uint16_t combine_16(const uint8_t *buf, size_t start_idx){
    return (uint16_t)(buf[start_idx+1] << 8 | buf[start_idx]);
}

#ifdef __cplusplus
}
#endif

#endif /* end FILESYSTEM_HANDLER_H */
