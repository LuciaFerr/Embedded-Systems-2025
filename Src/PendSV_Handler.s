.syntax unified           /* sintaxe unificada ARM/Thumb para assembler */
.cpu cortex-m4            /* alvo: Cortex-M4 */
.fpu softvfp              /* ABI: softvfp (não usar instruções FPU) */
.thumb                    /* modo Thumb */

/* Exporta o símbolo para o linker e declara tipo função */
.global PendSV_Handler
.type PendSV_Handler, %function

PendSV_Handler:
    /* ---------------------------
       1) DESATIVA INTERRUPÇÕES
       --------------------------- */
    cpsid i                /* cpsid i  -> Clear PRIMASK.I : desativa IRQs (garante atomicidade) */

    /* ---------------------------
       2) OBTER PSP (stack da task atual)
       --------------------------- */
    mrs r0, psp            /* r0 = PSP (move register); PSP aponta para o topo da stack da task atual
                              (note: o hardware já empilhou R0-R3,R12,LR,PC,xPSR antes de entrar aqui) */

    /* ---------------------------
       3) GUARDAR R4-R7 na stack (primeira parte)
       --------------------------- */
    subs r0, #16           /* r0 = r0 - 16 ; reservar 16 bytes para R4..R7 */
    stmia r0!, {r4-r7}     /* store multiple (increment after) -> grava R4,R5,R6,R7 em memória e faz r0 += 16
                              => agora r0 aponta logo após onde escreveu R4..R7 */

    /* ---------------------------
       4) COPIAR R8-R11 para R4-R7 (truque de compatibilidade)
       --------------------------- */
    mov r4, r8             /* r4 = r8 */
    mov r5, r9             /* r5 = r9 */
    mov r6, r10            /* r6 = r10 */
    mov r7, r11            /* r7 = r11 */
    /* Agora r4..r7 contêm os valores originais de r8..r11 */

    /* ---------------------------
       5) GUARDAR (antigos) R8-R11 na stack (agora em r4-r7)
       --------------------------- */
    subs r0, #32           /* r0 = r0 - 32 ; reservar espaço extra (para manter layout esperado) */
    stmia r0!, {r4-r7}     /* grava os valores (que são as cópias de R8..R11) e incrementa r0 */

    /* ---------------------------
       6) AJUSTAR r0 para apontar ao bottom do contexto salvo
       --------------------------- */
    subs r0, #16           /* r0 = r0 - 16 ; ajustar para o início do bloco de contexto salvo
                              (assim r0 fica no SP "final" que vamos gravar no TCB) */

    /* ---------------------------
       7) SALVAR SP ATUAL NO TCB DA TASK ATUAL
       --------------------------- */
    ldr r2, =os_curr_task  /* r2 = endereço da variável global os_curr_task */
    ldr r1, [r2]           /* r1 = *os_curr_task  (ponteiro p/ TCB atual) */
    str r0, [r1]           /* * (TCB->saved_sp) = r0 ; grava SP salvo no TCB da tarefa atual */

    /* ---------------------------
       8) CARREGAR SP DA PRÓXIMA TASK (do seu TCB)
       --------------------------- */
    ldr r2, =os_next_task  /* r2 = endereço de os_next_task */
    ldr r1, [r2]           /* r1 = *os_next_task (ponteiro p/ TCB next) */
    ldr r0, [r1]           /* r0 = TCB_next->saved_sp ; r0 passa a ser o SP da próxima task */

    /* ---------------------------
       9) RESTAURAR R4-R7 DA NOVA TASK (primeira metade)
       --------------------------- */
    ldmia r0!, {r4-r7}     /* ldmia (load multiple) : carrega 4 palavras para r4..r7; r0 = r0 + 16 (Lê DA STACK)*/

    /* ---------------------------
       10) COPIAR PARA R8-R11 (restaurar os registos "altos")
       --------------------------- */
    mov r8, r4             /* r8 = r4 (que veio da stack) */
    mov r9, r5             /* r9 = r5 */
    mov r10, r6            /* r10 = r6 */
    mov r11, r7            /* r11 = r7 */

    /* ---------------------------
       11) RESTAURAR R4-R7 (segunda metade)
       --------------------------- */
    ldmia r0!, {r4-r7}     /* carrega os valores originais de r4..r7 ; r0 = r0 + 16 */

    /* ---------------------------
       12) ATUALIZAR PSP para a stack da nova task
       --------------------------- */
    msr psp, r0            /* PSP = r0 (write to system register); atualiza o Process Stack Pointer para o topo da stack da próxima task */

    /* ---------------------------
       13) PREPARAR RETORNO (EXC_RETURN) E REACTIVAR IRQs
       --------------------------- */
    ldr r0, =0xFFFFFFFD    /* r0 = EXC_RETURN (valor mágico): return to Thread mode, use PSP, no FPU lazy restore */
    cpsie i                /* reativa IRQs (PRIMASK.I = 0) */

    /* ---------------------------
       14) SAIR DA EXCEÇÃO -> hardware irá restaurar automaticamente R0-R3,R12,LR,PC,xPSR
       --------------------------- */
    bx r0                  /* branch to EXC_RETURN -> faz a saída da exceção; hardware faz o popping automático */

.size PendSV_Handler, .-PendSV_Handler
