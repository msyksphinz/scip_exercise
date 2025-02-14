#ifndef __CONSOLE_H__
#define __CONSOLE_H__

#include "sheets.h"

struct CONSOLE {
  struct SHEET *sht;
  int cur_x, cur_y, cur_c;
};

void console_task (struct SHEET *sheet, unsigned int memtotal);
void cons_newline (struct CONSOLE *cons);

void cons_runcmd (char *cmdline, struct CONSOLE *cons, int *fat, unsigned int memtotal);
void cmd_mem (struct CONSOLE *cons, unsigned int memtotal);
void cmd_cls (struct CONSOLE *cons);
void cmd_dir (struct CONSOLE *cons);
void cmd_type (struct CONSOLE *cons, int *fat, char *cmdline);
void cmd_hlt (struct CONSOLE *cons, int *fat);
void cons_putchar (struct CONSOLE *cons, int chr, char move);

#endif // __CONSOLE_H__

