#include "ch32v30x_it.h"

#define __IRQ       __attribute__((interrupt()))
#define __IRQ_WEAK  __attribute__((interrupt(), weak))
#define __IRQ_NAKED __attribute__((naked))


/**
 * FreeRTOS supports both non-vectored and vectored exception model.
 * For non-vectored exception, use `freertos_risc_v_trap_handler`,
 * this function will determine the type of current exception.
 * For vectored exception, use `freertos_risc_v_exception_handler`,
 * and use `freertos_risc_v_mtimer_interrupt_handler` for timer interrupt.
 * 
 */

/**
 * @brief M mode ecall handler
 * 
 */
__IRQ_NAKED void Ecall_M_Mode_Handler(void) {
    /* Use naked function to generate a short call, without saving stack. */
    asm("j freertos_risc_v_trap_handler");
}

/**
 * @brief SysTick interrupt handler
 * 
 * @return __IRQ_NAKED 
 */
__IRQ_NAKED void SysTick_Handler(void) {
    /* Use naked function to generate a short call, without causing stack unbalance. */
    asm volatile(
        "addi sp, sp, -4\n"     /* Push */
        "sw t0, 4(sp)\n"        /* Save t0 on stack */
        "li t0, 0xE000F004\n"   /* SysTick->SR */
        "sw zero, 0(t0)\n"      /* Write 0 to clear */
        "lw t0, 4(sp)\n"        /* Restore t0 from stack */
        "addi sp, sp, 4\n"      /* Pop */
        "j freertos_risc_v_mtimer_interrupt_handler"
    );
}
