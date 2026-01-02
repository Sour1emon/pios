#ifndef _TIMER_H
#define _TIMER_H

unsigned long time_since_boot();
void timer_init(void);
void handle_timer_irq(void);

#endif
