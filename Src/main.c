#include <stdint.h>
#include "stm32f4xx.h"


/*
 * stackA[127]  ← xPSR
stackA[126]  ← PC inicial
stackA[125]  ← LR
stackA[124]  ← R12
stackA[123]  ← R3
stackA[122]  ← R2
stackA[121]  ← R1
stackA[120]  ← R0
stackA[119]  ← (talvez reservado)
...
stackA[112]  ← apontado por tcb->sp
 *
 *
 */

extern void PendSV_Handler(void);

typedef struct {
    uint32_t *sp;
} TCB_t;

uint32_t stackA[128]; //128 * 4bytes = 512 bytes por stack
uint32_t stackB[128];

TCB_t taskA;
TCB_t taskB;

TCB_t *os_curr_task;
TCB_t *os_next_task;

void taskA_func(void) {
    while (1) {
        GPIOA->ODR ^= (1<<5);  // LED verde. XOR Com 0001 0000
        for (volatile int i=0; i<50000; i++);
    }
}


void uart_send_char(char c) {
    while (!(USART2->SR & (1<<7)));  // TXE: transmit data register empty
    USART2->DR = c;
}

void uart_send_string(const char *s) {
    while (*s) uart_send_char(*s++);
}

void taskB_func(void) {
    while (1) {
        uart_send_string("Task B a correr!\r\n");
        for (volatile int i=0; i<300000; i++);
    }
}


void init_task(TCB_t *tcb, uint32_t *stack, void (*func)(void)) {

    stack += 128 - 16;       // espaço para stack frame automático

    stack[8]  = (uint32_t)func;  // PC
    stack[9]  = 0x01000000;      // xPSR (Thumb bit)

    tcb->sp = stack;
}

void trigger_context_switch(void) {
    SCB->ICSR = SCB_ICSR_PENDSVSET_Msk; //ativa Context Switch
}

int main(void) {

    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN; // Ativa o clock do GPIOA
    GPIOA->MODER |= (1 << 10); //SET PIN PA5 AS OUTPUT

    // Ativar clock para USART2 e GPIOA
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;

    // PA2 = alternate function AF7 (USART2_TX)
    // PA3 = alternate function AF7 (USART2_RX)
    GPIOA->MODER &= ~((3<<(2*2)) | (3<<(3*2))); // limpar bits
    GPIOA->MODER |=  ((2<<(2*2)) | (2<<(3*2))); // AF mode

    // AF7 para PA2 e PA3
    GPIOA->AFR[0] |= (7<<(2*4)) | (7<<(3*4));

    // Configuração USART2: 115200 8N1
    USART2->BRR = (uint16_t)(84000000/115200); // APB1 clock = 42MHz para F446? depende do clock init
    USART2->CR1 |= (1<<3) | (1<<2); // TE e RE
    USART2->CR1 |= (1<<13);         // UE = USART enable


    init_task(&taskA, stackA, taskA_func);
    init_task(&taskB, stackB, taskB_func);

    os_curr_task = &taskA;
    os_next_task = &taskB;

    __set_PSP((uint32_t)taskA.sp); //Colocar o PSP na stack da Task A
    __set_CONTROL(0x02);         // usar PSP em Thread mode
    __ISB();

    while (1) {
        trigger_context_switch(); // chama PendSV
        for (volatile int i=0; i<500000; i++); // delay
    }
}
