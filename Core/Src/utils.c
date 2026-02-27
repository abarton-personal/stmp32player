/**
  ******************************************************************************
  * @file           : sd_handler.c
  * @brief          : see sd_handler.h for description
  ******************************************************************************
*/

/* ============================================================================
 * INCLUDES
 * ==========================================================================*/
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "utils.h"
#include "stm32l4xx_hal.h"

/* ============================================================================
 * PRIVATE MACROS & CONSTANTS
 * ==========================================================================*/
#define MAX_PRINTF_BUFFER_SIZE 128

/* ============================================================================
 * PRIVATE TYPES & ENUMERATIONS
 * ==========================================================================*/


/* ============================================================================
 * PRIVATE VARIABLES (Module-scope globals)
 * ==========================================================================*/
static UART_HandleTypeDef *uart = NULL;

 /* ============================================================================
 * PRIVATE FUNCTION PROTOTYPES
 * ==========================================================================*/


 /* ============================================================================
 * PUBLIC FUNCTIONS
 * ==========================================================================*/
/**
 * @brief  Initialize variables used by utils.
 * @param  rt - pointer to uart handle to be used for debug prints
 * @return void
 */
 void utils_init(UART_HandleTypeDef *rt){
    if (rt != NULL){
        uart = rt;
        uart_printf("SD handler initialized\r\n");
    }
 }


/**
 * @brief  Wrapper function to simplify uart transmit.
 *         Works just like printf.
 * @return void
 */
void uart_printf(const char *fmt, ...)
{
    if (uart == NULL) return;

    char buf[MAX_PRINTF_BUFFER_SIZE];
    va_list args;

    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    HAL_UART_Transmit(uart, (uint8_t*)buf, strlen(buf), HAL_MAX_DELAY);
}


/* ============================================================================
 * PRIVATE FUNCTION DEFINITIONS
 * ==========================================================================*/
