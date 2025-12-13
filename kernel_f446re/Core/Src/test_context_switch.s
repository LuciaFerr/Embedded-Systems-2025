.syntax unified
.cpu cortex-m4
.fpu softvfp
.thumb

.global PendSV_Handler
.type PendSV_Handler, %function
PendSV_Handler:
    /* Disable interrupts for atomic operation */
    cpsid i

    /*
    Save current task's registers R4-R11 to its stack
    */

    /* Get current Process Stack Pointer */
    push {r4-r11}
    mrs r0, psp


    /* Make space for R8-R11 and save R4-R7 */
    //subs r0, #16
    //stmia r0!, {r4-r7}

    /* Copy R8-R11 to R4-R7 for saving (M0 compatibility) */
    //mov r4, r8
    //mov r5, r9
    //mov r6, r10
    //mov r7, r11

    /* Save R8-R11 (now in R4-R7) to stack */
    //subs r0, #32
    //stmia r0!, {r4-r7}

    /* Adjust R0 to point to bottom of saved context */
    //subs r0, #16

    /* Save current task's SP to its task control block */
    ldr r2, =os_curr_task
    ldr r1, [r2]
    str r0, [r1]

    /* Load next task's SP from its task control block */
    ldr r2, =os_next_task
    ldr r1, [r2]
    ldr r0, [r1]

    /*
    Restore next task's registers R4-R11 from its stack
    */

    /* Restore R4-R7 */
    //ldmia r0!, {r4-r7}

    /* Copy to R8-R11 */
    //mov r8, r4
    //mov r9, r5
    //mov r10, r6
    //mov r11, r7

    /* Restore original R4-R7 */
    //ldmia r0!, {r4-r7}

    /* Update Process Stack Pointer */
    msr psp, r0
    pop {r4-r11}

    /* EXC_RETURN value: Thread mode + PSP + No FPU */
    ldr r0, =0xFFFFFFFD

    /* Re-enable interrupts */
    cpsie i

    /* Return from exception - triggers hardware context restore */
    bx r0

//.size PendSV_Handler, .-PendSV_Handler
