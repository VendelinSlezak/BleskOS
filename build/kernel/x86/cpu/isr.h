#ifndef BUILD_KERNEL_X86_CPU_ISR_H
#define BUILD_KERNEL_X86_CPU_ISR_H


void isr0_handler_divide_by_zero(void);
void isr1_handler_debug(void);
void isr2_handler_nmi(void);
void isr3_handler_breakpoint(void);
void isr4_handler_overflow(void);
void isr5_handler_bound_range_exceeded(void);
void isr6_handler_invalid_opcode(void);
void isr7_handler_device_not_available(void);
void isr8_handler_double_fault(void);
void isr9_handler_coprocessor_segment_overrun(void);
void isr10_handler_invalid_tss(void);
void isr11_handler_segment_not_present(void);
void isr12_handler_stack_segment_fault(void);
void isr13_handler_general_protection_fault(void);
void isr14_handler_page_fault(void);
void isr16_handler_x87_fpu_error(void);
void isr17_handler_alignment_check(void);
void isr18_handler_machine_check(void);
void isr19_handler_simd_fpu_exception(void);

#endif /* BUILD_KERNEL_X86_CPU_ISR_H */
