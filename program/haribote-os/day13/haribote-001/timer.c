#include "bootpack.h"
#include "timer.h"

#define TIMER_FLAGS_ALLOC  1  // allocated
#define TIMER_FLAGS_USING  2  // timer is using

#define PIT_CTRL 0x0043
#define PIT_CNT0 0x0040

struct TIMERCTL timerctl;

void init_pit(void)
{
  io_out8 (PIT_CTRL, 0x34);
  io_out8 (PIT_CNT0, 0x9c);
  io_out8 (PIT_CNT0, 0x2e);
  timerctl.count = 0;
  int i;
  for (i = 0; i < MAX_TIMER; i++) {
	timerctl.timer[i].flags = 0;  // unused
  }
  
  return;
}


struct TIMER *timer_alloc (void)
{
  int i;
  for (i = 0; i < MAX_TIMER; i++) {
	if (timerctl.timer[i].flags == 0) {
	  timerctl.timer[i].flags = TIMER_FLAGS_ALLOC;
	  return &timerctl.timer[i];
	}
  }
  return 0;
}


void timer_free (struct TIMER *timer)
{
  timer->flags = 0;
  return;
}


void timer_init (struct TIMER *timer, struct FIFO8 *fifo, unsigned char data)
{
  timer->fifo = fifo;
  timer->data = data;

  return;
}


void timer_settime (struct TIMER *timer, unsigned int timeout)
{
  timer->timeout = timeout;
  timer->flags = TIMER_FLAGS_USING;

  return;
}


void inthandler20(int *esp)
{
  io_out8 (PIC0_OCW2, 0x60);
  timerctl.count ++;
  int i;
  for (i = 0; i < MAX_TIMER; i++) {
	if (timerctl.timer[i].flags == TIMER_FLAGS_USING) {
	  timerctl.timer[i].timeout --;
	  if (timerctl.timer[i].timeout == 0) {
		timerctl.timer[i].flags = TIMER_FLAGS_ALLOC;
		fifo8_put(timerctl.timer[i].fifo, timerctl.timer[i].data);
	  }
    }
  }
  
  return;
}



               
