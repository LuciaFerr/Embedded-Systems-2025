/*
 * os.c
 *
 *  Created on: Nov 20, 2025
 *      Author: M Hassaan Khalid
 */


#include "os.h"

typedef enum {
	OS_TASK_STATUS_READY = 1,
	OS_TASK_STATUS_DELAYED
} os_task_status_t;

typedef uint8_t task_prio_t; // 0 = higher priority

typedef struct {
	/* The stack pointer (sp) has to be the first element as it is located
	   at the same address as the structure itself (which makes it possible
	   to locate it safely from assembly implementation of PendSV_Handler).
	   The compiler might add padding between other structure elements. */
	volatile uint32_t sp;
	void (*handler)(void);
	volatile os_task_status_t status;
	volatile uint32_t delay_ticks;
	volatile task_prio_t priority;
} os_task_t;

static struct {
	os_task_t tasks[OS_CONFIG_MAX_TASKS];
	volatile uint32_t current_task;
	uint32_t size;
} m_task_table;

volatile os_task_t *os_curr_task;
volatile os_task_t *os_next_task;

volatile uint32_t tick_debug = 0;



static void task_finished(void)
{
	/* This function is called when some task handler returns. */
	volatile uint32_t i = 0;
	while (1)
		i++;
}

void os_init(void)
{
    /* Disable FPU completely */
    SCB->CPACR &= ~(0xF << 20);
    __DSB();
    __ISB();

	memset(&m_task_table, 0, sizeof(m_task_table));
}


/* helper to prepare canonical Cortex-M initial stack frame */
static void prepare_initial_stack(os_stack_t *p_stack, uint32_t stack_size, void (*handler)(void), uint32_t *out_sp)
{
    uint32_t *stk = &p_stack[stack_size]; /* one-past-end, points to the top of the stack */

    /* 8-byte align */
    stk = (uint32_t *)((uint32_t)stk & ~0x7UL);

    /* hardware-saved frame: xPSR, PC, LR, R12, R3, R2, R1, R0 */
    *(--stk) = 0x01000000UL;         /* xPSR */
   // *(--stk) = (uint32_t)handler;  // PC
    *(--stk) = ((uint32_t)handler) | 0x1;
    //*(--stk) = 0xFFFFFFFDUL;         // LR = EXC_RETURN (use PSP)
    *(--stk) = 0x00000000UL;
    *(--stk) = 0;                    /* R12 */
    *(--stk) = 0;                    /* R3  */
    *(--stk) = 0;                    /* R2  */
    *(--stk) = 0;                    /* R1  */
    *(--stk) = 0;                    /* R0  */

    /* reserve R4-R11 */
    for (int i = 0; i < 8; ++i) *(--stk) = 0;

    *out_sp = (uint32_t)stk;

    if (stk < &p_stack[0] || stk > &p_stack[stack_size]) {
        __BKPT(0);   // breakpoint de emergência
    }

}

bool os_task_init(void (*handler)(void), os_stack_t *p_stack, uint32_t stack_size, task_prio_t prio)
{
    if (m_task_table.size >= OS_CONFIG_MAX_TASKS - 1)
        return false;

    os_task_t *p_task = &m_task_table.tasks[m_task_table.size];
    p_task->handler = handler;
    p_task->status = OS_TASK_STATUS_READY;
    p_task->delay_ticks = 0;
    p_task->priority = prio;

    uint32_t initial_sp;
    prepare_initial_stack(p_stack, stack_size, handler, &initial_sp);
    p_task->sp = initial_sp;

    m_task_table.size++;

    return true;
}

void os_start(uint32_t systick_ticks)
{

    m_task_table.current_task = 0; //starts the first task
    os_curr_task = &m_task_table.tasks[0];
    //os_next_task = os_curr_task;

  //  os_first_switch = 1;


    /* prioridades (MUITO IMPORTANTE) */
    NVIC_SetPriority(SysTick_IRQn, 0xE0);
    NVIC_SetPriority(PendSV_IRQn, 0xF0);

    /* --- MUDAR PARA PSP --- */
    __set_PSP(os_curr_task->sp+8*4);
    __set_CONTROL(0x02);   // Thread mode, PSP
    __ISB(); //Instruction Synchronization Barrier

  //  __disable_irq();          //  ISTO É A CHAVE

     os_start_systick();       // SysTick começa a contar
                                // mas NÃO pode gerar IRQ

    // entrar no kernel
    __asm volatile ("svc 0");

    while (1);


}





void os_systick(void)
{

	tick_debug++;

	  for (int i = 0; i < m_task_table.size; i++)
	    {
	        os_task_t *t = &m_task_table.tasks[i];

	        if (t->status == OS_TASK_STATUS_DELAYED)
	        {
	            if (--t->delay_ticks == 0)
	            {
	                t->status = OS_TASK_STATUS_READY;
	            }
	        }
	    }

    /* request context switch */
    SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
}

void os_start_systick(void)
{
	    SysTick->LOAD = (SystemCoreClock / 1000) - 1;
	    SysTick->VAL  = 0;
	    SysTick->CTRL =
	          SysTick_CTRL_CLKSOURCE_Msk
	        | SysTick_CTRL_TICKINT_Msk
	        | SysTick_CTRL_ENABLE_Msk;
}

/*

void os_schedule(void)
{
    int start = m_task_table.current_task;

    for (int i = 1; i <= m_task_table.size; i++)
    {
        int idx = (start + i) % m_task_table.size;

        if (m_task_table.tasks[idx].status == OS_TASK_STATUS_READY)
        {
            m_task_table.current_task = idx;
            os_next_task = &m_task_table.tasks[idx];
            os_curr_task = os_next_task;
            return;
        }
    }

    // se ninguém estiver READY, continua na mesma
    os_next_task = os_curr_task;
}
*/

void os_schedule(void)
{
    os_task_t *best = NULL;

    for (int i = 0; i < m_task_table.size; i++)
    {
        os_task_t *t = &m_task_table.tasks[i];

        if (t->status != OS_TASK_STATUS_READY)
            continue;

        if (!best || t->priority < best->priority)
            best = t;
    }

    if (best){
        os_next_task = best;
    	os_curr_task = os_next_task;
    }else
        os_next_task = os_curr_task; // fallback
}


void os_delay(uint32_t ticks)
{
    __disable_irq();

    os_curr_task->delay_ticks = ticks;
    os_curr_task->status = OS_TASK_STATUS_DELAYED;

    __enable_irq();

    // forces context switch
    SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
}



