#ifndef __MAIN2SD_H
#define __MAIN2SD_H

struct FBUF
{
		char fbuf[128];
		struct FBUF *fpn;
};

extern struct FBUF *fsd;

#endif
