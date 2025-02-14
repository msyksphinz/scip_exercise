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

struct BOOTINFO {
  char cyls, leds, vmode, reserve;
  short scrnx, scrny;
  char *vram;
};

void io_hlt(void);
void io_cli(void);
void io_out8(int port, int data);
int io_load_eflags(void);
void io_store_eflags(int eflags);

void init_palette(void);
void set_palette(int start, int end, unsigned char *rgb);
void boxfill8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1);
void init_screen(char *vram, int x, int y);
void putfont8_asc (char *vram, int xsize, int x, int y, char c, unsigned char *s);

void HariMain(void)
{
  int i;
  struct BOOTINFO *binfo = (struct BOOTINFO *)0x0ff0;
  int xsize = (*binfo).scrnx;
  int ysize = (*binfo).scrny;
  char *vram = (*binfo).vram;

  init_pallete();
  init_screen (vram, xsize, ysize);

  putfont8_asc (binfo->vram, binfo->scrnx, 8, 8, COL8_FFFFFF, "ABC 123");

  putfont8_asc (binfo->vram, binfo->scrnx, 31, 31, COL8_000000, "Haribote OS.");
  putfont8_asc (binfo->vram, binfo->scrnx, 30, 30, COL8_FFFFFF, "Haribote OS.");

  for (;;) {
    io_hlt();
  }
}


void putfont8_asc (char *vram, int xsize, int x, int y, char c, unsigned char *s)
{
  extern char hankaku[4096];

  for (; *s != 0x00;s++) {
	putfont8 (vram, xsize, x, y, c, hankaku + *s * 16);
	x += 8;
  }
  return;
}


void putfont8 (char *vram, int xsize, int x, int y, char c, char *font)
{
  for (int i = 0; i < 16; i++) {
	char d = font[i];
	char *p = vram + (y + i) * xsize + x;
	if ((d & 0x80) != 0) { p[0] = c; }
	if ((d & 0x40) != 0) { p[1] = c; }
	if ((d & 0x20) != 0) { p[2] = c; }
	if ((d & 0x10) != 0) { p[3] = c; }
	if ((d & 0x08) != 0) { p[4] = c; }
	if ((d & 0x04) != 0) { p[5] = c; }
	if ((d & 0x02) != 0) { p[6] = c; }
	if ((d & 0x01) != 0) { p[7] = c; }
  }
}


void init_pallete(void)
{
  static unsigned char table_rgb[16 * 3] = {
	0x00, 0x00, 0x00,   /*  0: Black       	*/
	0xff, 0x00, 0x00,   /*  1: Light Read  	*/
	0x00, 0xff, 0x00,   /*  2: Light Green 	*/
	0xff, 0xff, 0x00,   /*  3: Light Yellow */
	0x00, 0x00, 0xff,   /*  4: Light Blue   */
	0xff, 0x00, 0xff,   /*  5: Light Purple */
	0x00, 0xff, 0xff,   /*  6: Light Light Blue */
	0xff, 0xff, 0xff,   /*  7: White        */
	0xc6, 0xc6, 0xc6,   /*  8: Light Gray   */
	0x84, 0x00, 0x00,   /*  9: Dark Red     */
	0x00, 0x84, 0x00,   /* 10: Dark Green   */
	0x84, 0x84, 0x00,   /* 11: Dark Yellow  */
	0x00, 0x00, 0x84,   /* 12: Dark Blue    */
	0x84, 0x00, 0x84,   /* 13: Dark Purple  */
	0x00, 0x84, 0x84,   /* 14: Dark Light Blue */
	0x84, 0x84, 0x84    /* 15: Dark Gray    */
  };
  set_palette (0, 15, table_rgb);
  return;

}


void set_palette (int start, int end, unsigned char *rgb)
{
  int i, eflags;
  eflags = io_load_eflags ();
  io_cli ();                  /* clear interrupt flag */
  io_out8 (0x03c8, start);
  for (i = start; i <= end; i++) {
	io_out8(0x03c9, rgb[0] / 4);
	io_out8(0x03c9, rgb[1] / 4);
	io_out8(0x03c9, rgb[2] / 4);
	rgb += 3;
  }
  io_store_eflags (eflags);
  return;
}


void init_screen(char *vram, int x, int y)
{
	boxfill8(vram, x, COL8_008484,  0,     0,      x -  1, y - 29);
	boxfill8(vram, x, COL8_C6C6C6,  0,     y - 28, x -  1, y - 28);
	boxfill8(vram, x, COL8_FFFFFF,  0,     y - 27, x -  1, y - 27);
	boxfill8(vram, x, COL8_C6C6C6,  0,     y - 26, x -  1, y -  1);

	boxfill8(vram, x, COL8_FFFFFF,  3,     y - 24, 59,     y - 24);
	boxfill8(vram, x, COL8_FFFFFF,  2,     y - 24,  2,     y -  4);
	boxfill8(vram, x, COL8_848484,  3,     y -  4, 59,     y -  4);
	boxfill8(vram, x, COL8_848484, 59,     y - 23, 59,     y -  5);
	boxfill8(vram, x, COL8_000000,  2,     y -  3, 59,     y -  3);
	boxfill8(vram, x, COL8_000000, 60,     y - 24, 60,     y -  3);

	boxfill8(vram, x, COL8_848484, x - 47, y - 24, x -  4, y - 24);
	boxfill8(vram, x, COL8_848484, x - 47, y - 23, x - 47, y -  4);
	boxfill8(vram, x, COL8_FFFFFF, x - 47, y -  3, x -  4, y -  3);
	boxfill8(vram, x, COL8_FFFFFF, x -  3, y - 24, x -  3, y -  3);
	return;
}


void boxfill8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1)
{
  int x, y;
  for (y = y0; y <= y1; y++) {
	for (x = x0; x <= x1; x++) {
	  vram[y * xsize + x] = c;
	}
  }
  return;
}


