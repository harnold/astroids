#ifndef TIMER_H
#define TIMER_H

int timer_init(float ticks_per_sec);
void timer_exit(void);
float timer_ticks_per_sec(void);
unsigned int timer_get_ticks(void);
unsigned int timer_get_ticks_delta(void);
float timer_get_time(void);
float timer_get_time_delta(void);
void timer_sleep(float seconds);
void timer_wait(float time);

#endif
