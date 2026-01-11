.syntax unified
.cpu cortex-m4
.thumb

.extern os_schedule
.extern os_curr_task
.extern os_next_task
.extern os_first_switch
.extern os_start_systick
.extern scheduler_started


.global PendSV_Handler
.type PendSV_Handler, %function


PendSV_Handler:

     //Saves EXC_RETURN (LR) in MSP

    push    {lr}

     //Saves Context of the current task (PSP)

    mrs     r0, psp       //saves psp of the task
    stmdb   r0!, {r4-r11} //stores r4-r11 in the task stack

    ldr     r1, =os_curr_task
    ldr     r1, [r1]
    str     r0, [r1]          // os_curr_task->sp = PSP (stores psp in r0)


     //Calls shceduler in C

    bl      os_schedule

	//Restore context of the next task

    ldr     r1, =os_next_task
    ldr     r1, [r1]
    ldr     r0, [r1]          // r0 = next_task->sp

    ldmia   r0!, {r4-r11}   //loads r4-r11(from the task stack) to the cpu registers
    msr     psp, r0			//updates psp


     //Exit Exception (Returns to task)

    pop     {lr}
    bx      lr


