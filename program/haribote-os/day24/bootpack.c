#include "bootpack.h"
#include "dsctbl.h"
#include "int.h"
#include "graphic.h"

extern struct TIMERCTL timerctl;

#define EFLAGS_AC_BIT      0x00040000
#define CR0_CACHE_DISABLE  0x60000000

extern void console_task (struct SHEET *sheet, unsigned int memtotal);

void HariMain(void)
{
  int i;
  int mmx = -1, mmy = -1;
  struct SHEET *sht = 0, *key_win;
  struct BOOTINFO *binfo = (struct BOOTINFO *)0x0ff0;
  int xsize = (*binfo).scrnx;
  int ysize = (*binfo).scrny;
  int mx = xsize/2;
  int my = ysize/2;
  int fifobuf[128], keycmd_buf[32];
  struct MOUSE_DEC mdec;
  unsigned char s[32];
  unsigned int memtotal;
  struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;
  int cursor_x, cursor_c;
  int key_shift = 0, key_leds = (binfo->leds >> 4) & 7, keycmd_wait = -1;
  
  struct SHTCTL *shtctl;
  struct SHEET *sht_back, *sht_mouse, *sht_win, *sht_cons;
  unsigned char *buf_back, buf_mouse[256], *buf_win, *buf_cons;
  struct TIMER *timer;
  struct FIFO32 fifo, keycmd;
  struct CONSOLE *cons;
  
  struct TASK *task_a, *task_cons;
  
  static char keytable0[0x80] = {
      0,   0,   '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '^', 0,  0,
      'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '@', '[', 0,   0, 'A', 'S',
      'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', ':', 0,   0,   ']', 'Z', 'X', 'C', 'V',
      'B', 'N', 'M', ',', '.', '/', 0,   '*', 0,   ' ', 0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   '7', '8', '9', '-', '4', '5', '6', '+', '1',
      '2', '3', '0', '.', 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0x5c, 0,  0,   0,   0,   0,   0,   0,   0,   0,   0x5c, 0,  0
  };
  static char keytable1[0x80] = {
      0,   0,   '!', 0x22, '#', '$', '%', '&', 0x27, '(', ')', '~', '=', '~', 0x08, 0,
      'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '`', '{', 0,   0, 'A', 'S',
      'D', 'F', 'G', 'H', 'J', 'K', 'L', '+', '*', 0,   0,   '}', 'Z', 'X', 'C', 'V',
      'B', 'N', 'M', '<', '>', '?', 0,   '*', 0,   ' ', 0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   '7', '8', '9', '-', '4', '5', '6', '+', '1',
      '2', '3', '0', '.', 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   '_', 0,   0,   0,   0,   0,   0,   0,   0,   0,   '|', 0,   0
  };

  init_gdtidt ();
  init_pic ();
  io_sti ();

  fifo32_init(&fifo, 32, fifobuf, 0);
  fifo32_init(&keycmd, 32, keycmd_buf, 0);
  
  init_pit();
  io_out8(PIC0_IMR, 0xf8); /* Allow PIT and Keyboard (11111000) */
  io_out8(PIC1_IMR, 0xef); /* Allow Mouse (11101111) */
  init_keyboard (&fifo, 256);
  enable_mouse (&fifo, 512, &mdec);

  timer = timer_alloc();
  timer_init(timer, &fifo, 1);
  timer_settime(timer, 50);
  
  memtotal = memtest(0x00400000, 0xbfffffff);
  memman_init (memman);
  memman_free (memman, 0x00001000, 0x009e000);   /* 0x00001000 - 0x0009efff */
  memman_free (memman, 0x00400000, memtotal - 0x00400000);

  init_pallete();
  shtctl = shtctl_init (memman, binfo->vram, binfo->scrnx, binfo->scrny);
  sht_back  = sheet_alloc(shtctl);
  sht_mouse = sheet_alloc(shtctl);
  sht_win   = sheet_alloc(shtctl);
  buf_back = (unsigned char *)memman_alloc_4k (memman, binfo->scrnx * binfo->scrny);
  buf_win  = (unsigned char *)memman_alloc_4k (memman, 160 * 52);
  sheet_setbuf (sht_back,  buf_back, binfo->scrnx, binfo->scrny, -1);
  sheet_setbuf (sht_mouse, buf_mouse, 16, 16, 99);
  sheet_setbuf (sht_win,   buf_win,   160, 52, -1);
  init_screen (buf_back, xsize, ysize);
  init_mouse_cursor8 (buf_mouse, 99);
  make_window8(buf_win, 160, 52, "task_a", 1);
  
  // sprintf (s, "(%d, %d)", mx, my);
  // putfonts8_asc (buf_back, binfo->scrnx, 0, 0, COL8_FFFFFF, s);
  // sprintf (s, "Memory %dMB, free : %dKB", 
  //          memtotal / (1024 * 1024), memman_total(memman) / 1024);
  // putfonts8_asc(buf_back, binfo->scrnx, 0, 32, COL8_FFFFFF, s);

  make_textbox8 (sht_win, 8, 28, 144, 16, COL8_FFFFFF);
  cursor_x = 8;
  cursor_c = COL8_FFFFFF;
  
  //=====================
  // Task Settings
  //=====================
  task_a = task_init(memman);
  fifo.task = task_a;
  task_run (task_a, 1, 0);
  
  /* console sheet */
  sht_cons = sheet_alloc(shtctl);
  buf_cons = (unsigned char *)memman_alloc_4k(memman, 256 * 165);
  sheet_setbuf (sht_cons, buf_cons, 256, 165, -1);
  make_window8 (buf_cons, 256, 165, "console", 0);
  make_textbox8 (sht_cons, 8, 28, 240, 128, COL8_000000);
  task_cons = task_alloc ();
  task_cons->tss.esp = memman_alloc_4k (memman, 64 * 1024) + 64 * 1024 - 12;
  task_cons->tss.eip = (int)&console_task;
  task_cons->tss.es = 1 * 8;
  task_cons->tss.cs = 2 * 8;
  task_cons->tss.ss = 1 * 8;
  task_cons->tss.ds = 1 * 8;
  task_cons->tss.fs = 1 * 8;
  task_cons->tss.gs = 1 * 8;
  *((int *) (task_cons->tss.esp + 4)) = (int) sht_cons;
  *((int *) (task_cons->tss.esp + 8)) = (int) memtotal;
  task_run (task_cons, 1, 0);   /* level = 2, priority = 2 */

  //=======================
  // Sheet Setting
  //=======================

  *((int *) 0x0fe4) = (int) shtctl;
  
  sheet_slide (sht_back,  0,   0);
  sheet_slide (sht_mouse, mx, my);
  sheet_slide (sht_cons,  32,  4);
  sheet_slide (sht_win,   64, 56);
  sheet_updown (sht_back,  0);
  sheet_updown (sht_cons,  1);
  sheet_updown (sht_win,   2);
  sheet_updown (sht_mouse, 3);

  sheet_refresh (sht_back, 0, 0, binfo->scrnx, 48);

  fifo32_put (&keycmd, KEYCMD_LED);
  fifo32_put (&keycmd, key_leds);

  key_win = sht_win;
  sht_cons->task = task_cons;
  sht_cons->flags |= 0x20;
  
  for (;;) {
	if (fifo32_status(&keycmd) > 0 && keycmd_wait < 0) {
	  keycmd_wait = fifo32_get(&keycmd);
	  wait_KBC_sendready ();
	  io_out8(PORT_KEYDAT, keycmd_wait);
	}
	io_cli();
	if (fifo32_status(&fifo) == 0) {
      task_sleep(task_a);
	  io_sti();
	} else {
      i = fifo32_get(&fifo);
      io_sti();
	  if (key_win->flags == 0) {   // input Window is closed
		key_win = shtctl->sheets[shtctl->top - 1];
		cursor_c = keywin_on (key_win, sht_win, cursor_c);
	  }
      if (256 <= i && i <= 511) {  // Keyboard Data
		// sprintf (s, "%x", i - 256);
        // putfonts8_asc_sht (sht_back, 0, 16, COL8_FFFFFF, COL8_008484, s, 2);
		if (i < 0x80 + 256) {
		  if (key_shift == 0) {
			s[0] = keytable0[i - 256];
		  } else {
			s[0] = keytable1[i - 256];
		  }
		} else {
		  s[0] = 0;
		}
		if ('A' <= s[0] && s[0] <= 'Z') {
		  if (((key_leds & 4) == 0 && key_shift == 0) ||
			  ((key_leds & 4) != 0 && key_shift != 0)) {
			s[0] += 0x20;
		  }
		}
		if (s[0] != 0) {
		  if (key_win == sht_win) { /* To Task-A */
			if (cursor_x < 128) {
			  s[1] = 0;
			  putfonts8_asc_sht(sht_win, cursor_x, 28, COL8_000000, COL8_FFFFFF, s, 1);
			  cursor_x += 8;
			}
		  } else { /* To Console */
			fifo32_put(&key_win->task->fifo, s[0] + 256);
		  }
		}
        if (i == 256 + 0x0e) {  // Backspace
		  if (key_win == sht_win) { // To Task-A
			if (cursor_x > 8) {
			  putfonts8_asc_sht(sht_win, cursor_x, 28, COL8_000000, COL8_FFFFFF, " ", 1);
			  cursor_x -= 8;
			}
		  } else {
		  	fifo32_put(&task_cons->fifo, 8 + 256);
		  }
        }
		if (i == 256 + 0x0f) { // Tab
		  cursor_c = keywin_off (key_win, sht_win, cursor_c, cursor_x);
		  int j = key_win->height - 1;
		  if (j == 0) {
			j = shtctl->top - 1;
		  }
		  key_win = shtctl->sheets[j];
		  cursor_c = keywin_on (key_win, sht_win, cursor_c);
		}
		if (i == 256 + 0x1c) {  // Enter
		  if (key_win != sht_win) {
			fifo32_put (&task_cons->fifo, 10 + 256);
		  }
		}
		if (i == 256 + 0x2a) {  // Left Shift ON
		  key_shift |= 1;
		}
		if (i == 256 + 0x36) {  // Right Shift ON
		  key_shift |= 2;
		}
		if (i == 256 + 0xaa) {  // Left Shift OFF
		  key_shift &= ~1;
		}
		if (i == 256 + 0xb6) {  // Right Shift ON
		  key_shift &= ~2;
		}
		if (i == 256 + 0x3a) {  // CapsLock
		  key_leds ^= 4;
		  fifo32_put(&keycmd, KEYCMD_LED);
		  fifo32_put(&keycmd, key_leds);
		}
		if (i == 256 + 0x45) {  // NumLock
		  key_leds ^= 2;
		  fifo32_put(&keycmd, KEYCMD_LED);
		  fifo32_put(&keycmd, key_leds);
		}
		if (i == 256 + 0x45) {  // ScrollLock
		  key_leds ^= 1;
		  fifo32_put(&keycmd, KEYCMD_LED);
		  fifo32_put(&keycmd, key_leds);
		}
		if (i == 256 + 0xfa) {  // KeyBoard receive Data
		  keycmd_wait = -1;
		}
		if (i == 256 + 0xfe) {  // KeyBoard failed to receive Data
		  wait_KBC_sendready ();
		  io_out8(PORT_KEYDAT, keycmd_wait);
		}
        if (i == 256 + 0x3b && key_shift != 0 && task_cons->tss.ss0 != 0) {   /* Shift+F1 */
          cons = (struct CONSOLE *) *((int *) 0x0fec);
          cons_putstr0 (cons, "\nBreak(key) : \n");
          io_cli ();
          task_cons->tss.eax = (int) &(task_cons->tss.esp0);
          task_cons->tss.eip = (int) asm_end_app;
          io_sti ();
        }
		if (i == 256 + 0x57 && shtctl->top > 2) {   /* F11 */
		  sheet_updown (shtctl->sheets[1], shtctl->top-1);
		}
        // Redraw cursor
        if (cursor_c >= 0) { 
          boxfill8 (sht_win->buf, sht_win->bxsize, cursor_c, cursor_x, 28, cursor_x + 7, 43);
        }
        sheet_refresh(sht_win, cursor_x, 28, cursor_x + 8, 44);
	  } else if (512 <= i && i <= 767) {  // Mouse Data
        if (mouse_decode(&mdec, i-512) != 0) {
          // sprintf (s, "[lcr %d %d]", mdec.x, mdec.y);
          // if ((mdec.btn & 0x01) != 0) { s[1] = 'L'; }
          // if ((mdec.btn & 0x02) != 0) { s[3] = 'R'; }
          // if ((mdec.btn & 0x04) != 0) { s[2] = 'C'; }
          // putfonts8_asc_sht (sht_back, 32, 16, COL8_FFFFFF, COL8_008484, s, 15);
          
          mx += mdec.x;
          my += mdec.y;
          if (mx < 0) { mx = 0; } if (mx > binfo->scrnx - 1) { mx = binfo->scrnx - 1; }
          if (my < 0) { my = 0; } if (my > binfo->scrny - 1) { my = binfo->scrny - 1; }

          // sprintf(s, "(%d, %d)", mx, my);
          // putfonts8_asc_sht (sht_back, 0, 0, COL8_FFFFFF, COL8_008484, s, 10);

          sheet_slide (sht_mouse, mx, my);
          if ((mdec.btn & 0x01) != 0) {
			/* Left Mouse Button is down */
			if (mmx < 0) {
			  /* Normal Mode */
			  for (int j = shtctl->top - 1; j > 0; j--) {
				sht = shtctl->sheets[j];
				int x = mx - sht->vx0;
				int y = my - sht->vy0;
				if (0 <= x && x < sht->bxsize && 0 <= y && y < sht->bysize) {
				  if (sht->buf[y * sht->bxsize + x] != sht->col_inv) {
					sheet_updown (sht, shtctl->top - 1);
					if (sht != key_win) {
					  cursor_c = keywin_off (key_win, sht_win, cursor_c, cursor_x);
					  key_win = sht;
					  cursor_c = keywin_on (key_win, sht_win, cursor_c);
					}
					if (3 <= x && x < sht->bxsize - 3 && 3 <= y && y < 21) {
					  mmx = mx;
					  mmy = my;
					}
					if (sht->bxsize - 21 <= x && x < sht->bxsize - 5 && 5 <= y && y < 19) {
					  if (sht->flags & 0x10 != 0) {
						cons = (struct CONSOLE *) *((int *) 0xfec);
						cons_putstr0 (cons, "\nBreak(mouse) :\n");
						io_cli ();
						task_cons->tss.eax = (int) & (task_cons->tss.esp0);
						task_cons->tss.eip = (int) asm_end_app;
						io_sti ();
					  }
					}
					break;
				  }
				}
			  }
			} else {
			  int x = mx - mmx;
			  int y = my - mmy;
			  sheet_slide (sht, sht->vx0 + x, sht->vy0 + y);
			  mmx = mx;
			  mmy = my;
			}
		  } else {
			mmx = -1;
          }
        }
      } else if (i <= 1) {
        if (i != 0) {
          timer_init(timer, &fifo, 0);
          if (cursor_c >= 0) {
            cursor_c = COL8_000000;
          }
        } else {
          timer_init(timer, &fifo, 1);
          if (cursor_c >= 0) {
            cursor_c = COL8_FFFFFF;
          }
        }
        timer_settime(timer, 50);
        if (cursor_c >= 0) {
          boxfill8 (sht_win->buf, sht_win->bxsize, cursor_c, cursor_x, 28, cursor_x + 7, 43);
          sheet_refresh(sht_win, cursor_x, 28, cursor_x + 8, 44);
        }
      }
    }
  }
}


int keywin_off (struct SHEET *key_win, struct SHEET *sht_win, int cur_c, int cur_x)
{
  change_wtitle8(key_win, 0);
  if (key_win == sht_win) {
	cur_c = -1; /* Remove Cursor */
	boxfill8(sht_win->buf, sht_win->bxsize, COL8_FFFFFF, cur_x, 28, cur_x + 7, 43);
  } else {
	if ((key_win->flags & 0x20) != 0) {
	  fifo32_put(&key_win->task->fifo, 3); /* Cursor OFF on Console */
	}
  }
  return cur_c;
}


int keywin_on (struct SHEET *key_win, struct SHEET *sht_win, int cur_c)
{
  change_wtitle8(key_win, 1);
  if (key_win == sht_win) {
	cur_c = COL8_000000; /* Show Cursor */
  } else {
	if ((key_win->flags & 0x20) != 0) {
	  fifo32_put(&key_win->task->fifo, 2); /* Consor ON on Console */
	}
  }
  return cur_c;
}

