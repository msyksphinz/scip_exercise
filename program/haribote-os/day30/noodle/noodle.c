#include "apilib.h"
#include "../haribote/sprintf.h"

void HariMain(void)
{
  char *buf, s[12];
  int win, timer, sec = 0, min = 0, hou = 0;
  api_initmalloc();
  buf = api_malloc(150 * 50);
  win = api_openwin(buf, 150, 50, -1, "noodle");
  timer = api_alloctimer();
  api_inittimer(timer, 128);
  for (;;) {
	sprintf(s, "%d:%d:%d", hou, min, sec);
	api_boxfilwin (win, 28, 27, 115, 41, 7 /* White */);
	api_putstrwin (win, 28, 27, 0 /* Black */, 11, s);
	api_settimer (timer, 100); /* 1 second */
	if (api_getkey(1) != 128) {
	  break;
	}
	sec++;
	if (sec == 60) {
	  sec = 0;
	  min++;
	  if (min == 60) {
		min = 0;
		hou++;
	  }
	}
  }
  api_end();
}
