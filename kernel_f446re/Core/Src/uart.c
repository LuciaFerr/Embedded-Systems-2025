#include "uart.h"
#include "os.h"
#include "stm32f4xx.h"

void uart_init(void)
{
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;

    GPIOA->MODER &= ~((3 << (2*2)) | (3 << (3*2)));
    GPIOA->MODER |=  ((2 << (2*2)) | (2 << (3*2)));

    GPIOA->AFR[0] |= (7 << (2*4)) | (7 << (3*4));

    USART2->BRR = 0x8B;
    USART2->CR1 = USART_CR1_TE | USART_CR1_UE;
}

void uart_putc(char c)
{
    while (!(USART2->SR & USART_SR_TXE));
    USART2->DR = c;
}

void uart_print(const char *s)
{
    while (*s)
        uart_putc(*s++);
}

void uart_print_hex(uint32_t val)
{
    char hex[] = "0123456789ABCDEF";
    for (int i = 28; i >= 0; i -= 4)
        uart_putc(hex[(val >> i) & 0xF]);
}

void uart_print_psp(const char *task, uint32_t psp)
{
    os_enter_critical();

    uart_print(task);
    uart_print(" PSP = 0x");
    uart_print_hex(psp);
    uart_print("\r\n");

    os_exit_critical();
}
