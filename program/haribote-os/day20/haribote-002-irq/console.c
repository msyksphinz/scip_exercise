#include "console.h"
#include "fifo.h"
#include "file.h"

void console_task (struct SHEET *sheet, unsigned int memtotal)
{
  struct TIMER *timer;
  struct TASK *task = task_now ();
  int i, fifobuf[128];
  char s[30], cmdline[30];
  struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;
  struct CONSOLE cons;

  cons.sht = sheet;
  cons.cur_x = 8;
  cons.cur_y = 28;
  cons.cur_c = -1;

  *((int *) 0x0fec) = (int) &cons;
  
  int *fat = (int *)memman_alloc_4k (memman, 4 * 2880);
  file_readfat (fat, (unsigned char *)(ADR_DISKIMG + 0x000200));
  
  fifo32_init (&task->fifo, 128, fifobuf, task);
  timer = timer_alloc ();
  timer_init (timer, &task->fifo, 1);
  timer_settime (timer, 50);

  cons_putchar (&cons, '>', 1);

  for (;;) {
    io_cli();
    if (fifo32_status (&task->fifo) == 0) {
      task_sleep(task);
      io_sti ();
    } else {
      i = fifo32_get (&task->fifo);
      io_sti ();
      if (i <= 1) {   /* timer for cursor */
        if (i != 0) {
          timer_init (timer, &task->fifo, 0);
          if (cons.cur_c >= 0) {
            cons.cur_c = COL8_FFFFFF;
          }
        } else {
          timer_init (timer, &task->fifo, 1);
          if (cons.cur_c >= 0) {
            cons.cur_c = COL8_000000;
          }
        }
        timer_settime (timer, 50);
	  }
      if (i == 2) { // Cursor ON
        cons.cur_c = COL8_FFFFFF;
      }
      if (i == 3) { // Cursor OFF
        boxfill8 (sheet->buf, sheet->bxsize, COL8_000000, cons.cur_x, 28, cons.cur_x + 7, 43);
        cons.cur_c = -1;
      }
	  if (256 <= i && i <= 511) {  /* Keyboard data from Task-A */
		if (i == 8 + 256) {
		  // Backspace
		  if (cons.cur_x > 16) {
			putfonts8_asc_sht(sheet, cons.cur_x, cons.cur_y, COL8_FFFFFF, COL8_000000, " ", 1);
			cons.cur_x -= 8;
		  }
		} else if (i == 10 + 256) {
		  // Enter
          cons_putchar (&cons, ' ', 0);
          putfonts8_asc_sht (sheet, cons.cur_x, cons.cur_y, COL8_FFFFFF, COL8_000000, " ", 1);
          cmdline[cons.cur_x / 8 - 2] = 0;
          cons_newline (&cons);
          // Execute Command
          cons_runcmd (cmdline, &cons, fat, memtotal);
          cons_putchar (&cons, '>', 1);
		} else {
		  // Normal charactor
		  if (cons.cur_x < 240) {
			s[0] = i - 256;
			s[1] = 0;
            cmdline[cons.cur_x / 8 - 2] = i - 256;
			putfonts8_asc_sht(sheet, cons.cur_x, cons.cur_y, COL8_FFFFFF, COL8_000000, s, 1);
			cons.cur_x += 8;
		  }
		}
	  }
      if (cons.cur_c >= 0) {
        boxfill8 (sheet->buf, sheet->bxsize, cons.cur_c, cons.cur_x, cons.cur_y, cons.cur_x + 7, cons.cur_y + 15);
      }
	  sheet_refresh (sheet, cons.cur_x, cons.cur_y, cons.cur_x + 8, cons.cur_y + 16);
    }
  }
}


void cons_newline (struct CONSOLE *cons)
{
  int x, y;
  struct SHEET *sheet = cons->sht;
  if (cons->cur_y < 28 + 112) {
    cons->cur_y += 16;
  } else {  // scroll
    for (y = 28; y < 28 + 112; y++) {
      for (x = 8; x < 8 + 240; x++) {
    	sheet->buf[x + y * sheet->bxsize] = sheet->buf[x + (y + 16) * sheet->bxsize];
      }
    }
    for (y = 28 + 112; y < 28 + 128; y++) {
      for (x = 8; x < 8 + 240; x++) {
    	sheet->buf[x + y * sheet->bxsize] = COL8_000000;
      }
    }
    // show Prompt
    sheet_refresh (sheet, 8, 28, 8 + 240, 28 + 128);
  }
  cons->cur_x = 8;
  return;
}


void cons_runcmd (char *cmdline, struct CONSOLE *cons, int *fat, unsigned int memtotal)
{
  if (strcmp(cmdline, "mem") == 0) {
    cmd_mem (cons, memtotal);
  } else if (strcmp(cmdline, "cls") == 0) {
    cmd_cls (cons);
  } else if (strcmp (cmdline, "dir") == 0) {
    cmd_dir (cons);
  } else if (cmdline[0]=='t' && cmdline[1]=='y' && cmdline[2]=='p' && cmdline[3]=='e' && cmdline[4]==' ') {
    cmd_type (cons, fat, cmdline);
  } else if (cmdline[0] != 0) {
    if (cmd_app(cons, fat, cmdline) == 0) {
      // Not Command Line and Empty
      putfonts8_asc_sht (cons->sht, 8, cons->cur_y, COL8_FFFFFF, COL8_000000, "Bad command.", 12);
      cons_newline (cons);
      cons_newline (cons);
    }
  }
}

void cmd_mem (struct CONSOLE *cons, unsigned int memtotal)
{
  struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;
  char s[30];
  /* mem command */
  sprintf(s, "total %dMB", memtotal / (1024 * 1024));
  putfonts8_asc_sht (cons->sht, 8, cons->cur_y, COL8_FFFFFF, COL8_000000, s, 30);
  cons_newline(cons);
  sprintf(s, "free %dKB", memman_total(memman) / 1024);
  putfonts8_asc_sht (cons->sht, 8, cons->cur_y, COL8_FFFFFF, COL8_000000, s, 30);
  cons_newline(cons);
  cons_newline(cons);
  return;
}
 

void cmd_cls (struct CONSOLE *cons)
{
  /* cls command */
  struct SHEET *sheet = cons->sht;
  for (int y = 28; y < 28 + 128; y++) {
    for (int x = 8; x < 8 + 240; x++) {
      sheet->buf[x + y * cons->sht->bxsize] = COL8_000000;
    }
  }
  sheet_refresh (sheet, 8, 28, 8 + 240, 28 + 128);
  cons->cur_y = 28;
  return;
}


void cmd_dir (struct CONSOLE *cons)
{
  struct FILEINFO *finfo = (struct FILEINFO *)(ADR_DISKIMG + 0x002600);
  char s[30];
  /* dir command */
  for (int i = 0; i < 224; i++) {
    if (finfo[i].name[0] == 0x00) {
      break;
    }
    if (finfo[i].name[0] != 0xe5) {
      if ((finfo[i].type & 0x18) == 0) {
        sprintf (s, "filename.ext   %d", finfo[i].size);
        for (int j = 0; j < 8; j++) {
          s[j] = finfo[i].name[j];
        }
        s[ 9]  =finfo[i].ext[0];
        s[10]  =finfo[i].ext[1];
        s[11]  =finfo[i].ext[2];
        putfonts8_asc_sht (cons->sht, 8, cons->cur_y, COL8_FFFFFF, COL8_000000, s, 30);
        cons_newline (cons);
      }
    }
  }
  cons_newline (cons);
  return;
}


void cmd_type (struct CONSOLE *cons, int *fat, char *cmdline)
{
  struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;
  struct FILEINFO *finfo = file_search (cmdline + 5, (struct FILEINFO *) (ADR_DISKIMG + 0x002600), 224);
  if (finfo != 0) {
    // Find file
    char *p = (char *)memman_alloc_4k (memman, finfo->size);
    file_loadfile (finfo->clustno, finfo->size, p, fat, (char *) (ADR_DISKIMG + 0x003e00));
    cons->cur_x = 8;
    for (int i = 0; i < finfo->size; i++) {
      // output char
      cons_putchar (cons, p[i], 1);
    }
    memman_free_4k (memman, (int) p, finfo->size);
  } else {
    // When no file found
    putfonts8_asc_sht (cons->sht, 8, cons->cur_y, COL8_FFFFFF, COL8_000000, "File not found.", 15);
    cons_newline (cons);
  }
  cons_newline (cons);
  return;
}


void cmd_hlt (struct CONSOLE *cons, int *fat)
{
  struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
  struct FILEINFO *finfo = file_search ("HLT.HRB", (struct FILEINFO *) (ADR_DISKIMG + 0x002600), 224);
  struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *)ADR_GDT;

  if (finfo != 0) {
    // Find file
    char *p = (char *)memman_alloc_4k (memman, finfo->size);
    file_loadfile (finfo->clustno, finfo->size, p, fat, (char *) (ADR_DISKIMG + 0x003e00));
    set_segmdesc(gdt + 1003, finfo->size - 1, (int) p, AR_CODE32_ER);
    farcall (0, 1003 * 8);
    memman_free_4k (memman, (int) p, finfo->size);
  } else {
    // When no file found
    putfonts8_asc_sht (cons->sht, 8, cons->cur_y, COL8_FFFFFF, COL8_000000, "File not found.", 15);
    cons_newline (cons);
  }
  cons_newline (cons);
  return;
}


void cons_putchar (struct CONSOLE *cons, int chr, char move)
{
  char s[2];
  s[0] = chr;
  s[1] = 0;
  if (s[0] == 0x09) { // tab
    for (;;) {
      putfonts8_asc_sht (cons->sht, cons->cur_x, cons->cur_y, COL8_FFFFFF, COL8_000000, " ", 1);
      cons->cur_x += 8;
      if (cons->cur_x == 8 + 240) {
        cons_newline(cons);
      }
      if (((cons->cur_x - 8) & 0x1f) == 0) {
        break;
      }
    }
  } else if (s[0] == 0x0a) {  // enter
    cons_newline(cons);
  } else if (s[0] == 0x0d) {  // enter
    // Do nothing
  } else {
    putfonts8_asc_sht (cons->sht, cons->cur_x, cons->cur_y, COL8_FFFFFF, COL8_000000, s, 1);
    if (move != 0) {
      cons->cur_x += 8;
      if (cons->cur_x == 8 + 240) {
        cons_newline (cons);
      }
    }
  }
  return;
}


int cmd_app (struct CONSOLE *cons, int *fat, char *cmdline)
{
  struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;
  struct FILEINFO *finfo;
  struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *)ADR_GDT;
  char name [18], *p;
  int i;

  for (i = 0; i < 13; i++) {
    if (cmdline[i] <= ' ') {
      break;
    }
    name[i] = cmdline[i];
  }
  name[i] = 0;
  finfo = file_search (name, (struct FILEINFO *)(ADR_DISKIMG + 0x002600), 224);
  if (finfo == 0 && name[i-1] != '.') {
    name [i+0]  = '.';
    name [i+1]  = 'H';
    name [i+2]  = 'R';
    name [i+3]  = 'B';
    name [i+4]  = 0;
    finfo = file_search (name, (struct FILEINFO *)(ADR_DISKIMG + 0x002600), 224);
  }

  if (finfo != 0) {
    char *p = (char *)memman_alloc_4k (memman, finfo->size);
    file_loadfile (finfo->clustno, finfo->size, p, fat, (char *) (ADR_DISKIMG + 0x003e00));
    set_segmdesc(gdt + 1003, finfo->size - 1, (int) p, AR_CODE32_ER);
    farcall (0, 1003 * 8);
    memman_free_4k (memman, (int) p, finfo->size);
    cons_newline (cons);
    return 1;
  }

  return 0;
}

