/**
 * @file keypad.h
 * @author @Raghav3107
 * @brief 
 * @version 1.0.1
 * @date 03-10-2021
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#ifndef _KEYPAD_H_
#define _KEYPAD_H_

#define KEY_NOT_PRESSED       -1     

/*---------------------------------------------------------------*/
/**
 * @brief Set up the row and column dimensions, pins, and key values.
 * 
 * @param rowCount Number of keypad rows.
 * @param columnCount Number of keypad columns.
 * @param rowPinCons Array that stores the row connections (from top to bottom).
 * @param columnPinCons Array that stores the column connections (from left to right).
 * @param buttonValues Array that stores the values that should be returned for each key.
 */
void keypad_setup(int rowCount, int columnCount,
                  int *rowPinCons, int *columnPinCons,
                  int *buttonValues);


/*---------------------------------------------------------------*/

/**
 * @brief  Returns the first key pressed, or KeyToReturnWhenNOKeyPressed if no key is pressed.
 * @param KeyToReturnWhenNOKeyPressed Value to return when no key is pressed.
 * @return The value of the key that was pressed.  The value for each key 
 * is set up in the array that gets passed to keypad_setup's *buttonValues argument.  
 */
char scanForSingleKeyOnce(char KeyToReturnWhenNOKeyPressed);

/**
 * @brief  Returns the first key pressed, or  if no key is pressed.
 * @param KeyToReturnWhenNOKeyPressed Value to return when no key is pressed.
 * @param number of ticks to wait for input. use           TickType_t ticks = pdMS_TO_TICKS(milliseconds);
 * @return The value of the key that was pressed.  The value for each key 
 * is set up in the array that gets passed to keypad_setup's *buttonValues argument.  
 */
char scanForSingleKeyWithTimeOut(char KeyToReturnWhenNOKeyPressed,TickType_t timeout_ticks);


#endif /* _KEYPAD_H_ */