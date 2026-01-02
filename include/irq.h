#ifndef _IRQ_H
#define _IRQ_H

void enable_interrupt_controller(void);

void show_invalid_entry_message(int type, unsigned long esr, unsigned long elr,
                                unsigned long far, unsigned long fp,
                                unsigned long lr);

void print_stack_trace(unsigned long fp, unsigned long lr, unsigned long elr);

void irq_vector_init(void);
void enable_irq(void);
void disable_irq(void);
#endif
