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


//PendSV_Handler:
    /* --- Save context of current task --- */
//    mov   r3, lr

 //   mrs   r0, psp              /* r0 = PSP */
  //  stmdb r0!, {r4-r11}        /* save R4–R11 on task stack */

 //   ldr   r1, =os_curr_task
   // ldr   r1, [r1]
    //str   r0, [r1]             /* os_curr_task->sp = PSP */

    /* --- Call scheduler (C) --- */
  //  bl    os_schedule

   // mov   lr, r3

    /* --- Restore context of next task --- */
   // ldr   r1, =os_next_task
    //ldr   r1, [r1]
    //ldr   r0, [r1]             /* r0 = next_task->sp */

    //ldmia r0!, {r4-r11}        /* restore R4–R11 */
    //msr   psp, r0
       /* update PSP */

    /* --- Return from exception --- */
    //bx    lr

/*
PendSV_Handler:
    //* ---------------------------------------
    // * Guardar EXC_RETURN no MSP
    // * ---------------------------------------
    push    {lr}

    // obter PSP
    mrs     r0, psp

    // verificar primeiro switch
    ldr     r2, =os_first_switch
    ldr     r2, [r2]
    cbnz    r2, skip_save

    // salvar R4–R11
    stmdb   r0!, {r4-r11}

    // guardar SP na task atual
    ldr     r1, =os_curr_task
    ldr     r1, [r1]
    str     r0, [r1]

skip_save:
    // chamar scheduler
    bl      os_schedule

    // limpar flag
    ldr     r2, =os_first_switch
    movs    r1, #0
    str     r1, [r2]

    //* restaurar SP da próxima task
    ldr     r1, =os_next_task
    ldr     r1, [r1]
    ldr     r0, [r1]

    //* restaurar R4–R11
    ldmia   r0!, {r4-r11}
    msr     psp, r0

    //* ---------------------------------------
    // * Restaurar EXC_RETURN
    // * ---------------------------------------
    pop     {lr}

    // exception return -
    bx      lr
*/

/*
PendSV_Handler:
    // ------------------------------------------------
     // Guardar EXC_RETURN (LR) no MSP
     // ------------------------------------------------
    push    {lr}

    // obter PSP atual
    mrs     r0, psp

    // verificar se é o primeiro switch
    ldr     r2, =os_first_switch
    ldr     r3, [r2]
    cbnz    r3, first_switch

    // ------------------------------------------------
     // Switch normal: salvar contexto da task atual
     // ------------------------------------------------
    stmdb   r0!, {r4-r11}

    ldr     r1, =os_curr_task
    ldr     r1, [r1]
    str     r0, [r1]          // os_curr_task->sp = PSP

    // chamar scheduler
    bl      os_schedule
    b       restore_context

first_switch:
    // ------------------------------------------------
    //  Primeiro switch: não salvar contexto
    // ------------------------------------------------

    // marcar que já não é o primeiro
    movs    r3, #0
    str     r3, [r2]

    // ativar SysTick agora que o kernel está estável
    bl      os_start_systick

restore_context:
    // ------------------------------------------------
    // Restaurar contexto da próxima task
    // ------------------------------------------------
    ldr     r1, =os_next_task
    ldr     r1, [r1]
    ldr     r0, [r1]          // r0 = next_task->sp

    ldmia   r0!, {r4-r11}
    msr     psp, r0

    // ------------------------------------------------
    // Restaurar EXC_RETURN e sair da exceção
    // ------------------------------------------------
    pop     {lr}
    bx      lr                //exception return
*/


PendSV_Handler:

     // Guardar EXC_RETURN (LR) no MSP

    push    {lr}


     // Salvar contexto da task atual (PSP)

    mrs     r0, psp
    stmdb   r0!, {r4-r11}

    ldr     r1, =os_curr_task
    ldr     r1, [r1]
    str     r0, [r1]          // os_curr_task->sp = PSP


     // Chamar scheduler em C

    bl      os_schedule


     // Restaurar contexto da próxima task

    ldr     r1, =os_next_task
    ldr     r1, [r1]
    ldr     r0, [r1]          // r0 = next_task->sp

    ldmia   r0!, {r4-r11}
    msr     psp, r0


     //Sair da exceção (volta à task)

    pop     {lr}
    bx      lr


/*
PendSV_Handler:

     * Guardar EXC_RETURN (LR) no MSP

    push    {lr}


     // Salvar contexto da task atual (PSP)

    mrs     r0, psp
    stmdb   r0!, {r4-r11}

    ldr     r1, =os_curr_task
    ldr     r1, [r1]
    str     r0, [r1]          // os_curr_task->sp = PSP


     // Iniciar SysTick apenas uma vez

    ldr     r2, =scheduler_started
    ldr     r3, [r2]
    cmp     r3, #0
    bne     1f

    movs    r3, #1
    str     r3, [r2]
    bl      os_start_systick

1:

     // Chamar scheduler em C

    bl      os_schedule


     // Restaurar contexto da próxima task

    ldr     r1, =os_next_task
    ldr     r1, [r1]          // r1 = os_next_task
    ldr     r0, [r1]          // r0 = next_task->sp

    ldmia   r0!, {r4-r11}
    msr     psp, r0


     // Sair da exceção (volta à task)

    pop     {lr}
    bx      lr

*/
