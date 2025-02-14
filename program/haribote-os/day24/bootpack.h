#ifndef __BOOTPACK_H__
#define __BOOTPACK_H__

#include "sheets.h"
#include "memtest.h"
#include "timer.h"
#include "graphic.h"
#include "keyboard.h"
#include "sprintf.h"
#include "naskfunc.h"
#include "mtask.h"


#define COL8_000000  0
#define COL8_FF0000  1
#define COL8_00FF00  2
#define COL8_FFFF00  3
#define COL8_0000FF  4
#define COL8_FF00FF  5
#define COL8_00FFFF  6
#define COL8_FFFFFF  7
#define COL8_C6C6C6  8
#define COL8_840000  9
#define COL8_008400 10
#define COL8_848400 11
#define COL8_000084 12
#define COL8_840084 13
#define COL8_008484 14
#define COL8_848484 15

#define PIC0_ICW1		0x0020
#define PIC0_OCW2		0x0020
#define PIC0_IMR		0x0021
#define PIC0_ICW2		0x0021
#define PIC0_ICW3		0x0021
#define PIC0_ICW4		0x0021
#define PIC1_ICW1		0x00a0
#define PIC1_OCW2		0x00a0
#define PIC1_IMR		0x00a1
#define PIC1_ICW2		0x00a1
#define PIC1_ICW3		0x00a1
#define PIC1_ICW4		0x00a1

#define ADR_BOOTINFO	0x00000ff0
#define ADR_DISKIMG     0x00100000

struct BOOTINFO {
  char cyls, leds, vmode, reserve;
  short scrnx, scrny;
  char *vram;
};

struct SEGMENT_DESCRIPTOR {
  short limit_low, base_low;
  char base_mid, access_right;
  char limit_high, base_high;
};


struct GATE_DESCRIPTOR {
  short offset_low, selector;
  char dw_count, access_right;
  short offset_high;
};


void io_hlt(void);
void io_cli(void);
void io_out8(int port, int data);
int io_load_eflags(void);
void io_store_eflags(int eflags);

#define ADR_IDT			0x0026f800
#define LIMIT_IDT		0x000007ff
#define ADR_GDT			0x00270000
#define LIMIT_GDT		0x0000ffff
#define ADR_BOTPAK		0x00280000
#define LIMIT_BOTPAK	0x0007ffff
#define AR_DATA32_RW	0x4092
#define AR_CODE32_ER	0x409a
#define AR_TSS32		0x0089
#define AR_INTGATE32	0x008e

#define PORT_KEYDAT          0x0060
#define PORT_KEYSTA          0x0064
#define PORT_KEYCMD          0x0064
#define KEYSTA_SEND_NOTREADY 0x02
#define KEYCMD_WRITE_MODE    0x60
#define KEYCMD_LED           0xed
#define KBC_MODE             0x47

void load_gdtr(int limit, int addr);
void load_idtr(int limit, int addr);

/* mouse.c */

#define KEYCMD_SENDTO_MOUSE 0xd4
#define MOUSECMD_ENABLE     0xf4

struct MOUSE_DEC {
  unsigned char buf[3], phase;
  int x, y, btn;
};

void enable_mouse (struct FIFO32 *fifo, int data0, struct MOUSE_DEC *mdec);
int mouse_decode(struct MOUSE_DEC *mdec, unsigned char dat);

#define MEMMAN_ADDR  0x003c0000

int keywin_off (struct SHEET *key_win, struct SHEET *sht_win, int cur_c, int cur_x);
int keywin_on (struct SHEET *key_win, struct SHEET *sht_win, int cur_c);

#endif // __BOOTPACK_H__
