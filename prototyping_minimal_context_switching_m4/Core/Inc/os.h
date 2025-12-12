/*
 * os.h
 *
 *  Created on: Nov 20, 2025
 *      Author: M Hassaan Khalid
 */

#ifndef INC_OS_H_
#define INC_OS_H_

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "os_config.h"


typedef uint32_t os_stack_t;

void os_init(void);
bool os_task_init(void (*handler)(void), os_stack_t *p_stack, uint32_t stack_size);
bool os_start(uint32_t systick_ticks);
//void PendSV_Handler(void);


#endif /* INC_OS_H_ */
