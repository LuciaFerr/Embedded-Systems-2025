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
typedef uint8_t task_prio_t;

void os_init(void);
void os_systick(void);
bool os_task_init(void (*handler)(void), os_stack_t *p_stack, uint32_t stack_size, task_prio_t prio );
void os_start(uint32_t systick_ticks);
void os_delay(uint32_t ticks);
uint32_t os_get_psp(void);
void os_enter_critical(void);
void os_exit_critical(void);


#endif /* INC_OS_H_ */
