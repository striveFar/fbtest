/*
   Simple framebuffer testing program

   Set the screen to a given color. For use in developing Linux
   display drivers.

   Copyright (c) 2014, Jumpnow Technologies, LLC
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

   1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.

   2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
   HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
   TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

   Ideas taken from the Yocto Project psplash program.
   */

#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <limits.h>
#include <linux/fb.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

struct fb_config {
	int fd;
	int width;
	int height;
	int height_virtual;
	int bpp;
	int stride;
	int red_offset;
	int red_length;
	int green_offset;
	int green_length;
	int blue_offset;
	int blue_length;
	int transp_offset;
	int transp_length;
	int buffer_num;
	char *data;
	char *base;
};

static void fbdev_blank(int _fp, bool blank)
{
	int ret;
	ret = ioctl(_fp, FBIOBLANK, blank ? FB_BLANK_POWERDOWN : FB_BLANK_UNBLANK);
	if (ret < 0)
		perror("ioctl(): blank");
}

void dump_vscreeninfo(struct fb_var_screeninfo *fvsi) {
	printf("======= FB VAR SCREENINFO =======\n");
	printf("xres: %d\n", fvsi->xres);
	printf("yres: %d\n", fvsi->yres);
	printf("yres_virtual: %d\n", fvsi->yres_virtual);
	printf("buffer number: %d\n", fvsi->yres_virtual / fvsi->yres);
	printf("bpp : %d\n", fvsi->bits_per_pixel);
	printf("red bits    :\n");
	printf("    offset   : %d\n", fvsi->red.offset);
	printf("    length   : %d\n", fvsi->red.length);
	printf("    msb_right: %d\n", fvsi->red.msb_right);
	printf("green bits  :\n");
	printf("    offset   : %d\n", fvsi->green.offset);
	printf("    length   : %d\n", fvsi->green.length);
	printf("    msb_right: %d\n", fvsi->green.msb_right);
	printf("blue bits   :\n");
	printf("    offset   : %d\n", fvsi->blue.offset);
	printf("    length   : %d\n", fvsi->blue.length);
	printf("    msb_right: %d\n", fvsi->blue.msb_right);
	printf("transp bits :\n");
	printf("    offset   : %d\n", fvsi->transp.offset);
	printf("    length   : %d\n", fvsi->transp.length);
	printf("    msb_right: %d\n", fvsi->transp.msb_right);
	printf("=================================\n");
}

void dump_fscreeninfo(struct fb_fix_screeninfo *ffsi) {
	printf("======= FB FIX SCREENINFO =======\n");
	printf("id          : %s\n", ffsi->id);
	printf("smem_start  : 0x%08lX\n", ffsi->smem_start);
	printf("smem_len    : %u\n", ffsi->smem_len);
	printf("line_length : %u\n", ffsi->line_length);
	printf("=================================\n");
}

void usage(const char *argv_0) {
	printf("\nUsage %s: [-r<red>] [-g<green>] [-b<blue>]\n", argv_0);
	printf("  All colors default to 0xff\n");
	exit(1);
}

/**** RGB888颜色定义 ****/
typedef struct rgb888_type {
	unsigned char blue;
	unsigned char green;
	unsigned char red;
} __attribute__((packed)) rgb888_t;

/********************************************************************
 * 函数名称： set_background_color
 * 功能描述： 将LCD背景颜色设置为指定的颜色
 * 输入参数： 颜色
 * 返 回 值： 无
 ********************************************************************/
static void set_background_color(struct fb_config *fbp, unsigned int color) {
	int size = fbp->height * fbp->width;
	//计算出像素点个数
	int j;
	int bpp = fbp->bpp;

	switch (bpp) {
	case 16: {
			 // RGB565
			 unsigned short *base = (unsigned short *)fbp->base;
			 unsigned short rgb565_color = ((color & 0xF800UL)) |
				 ((color & 0x07E0UL)) |
				 ((color & 0x001FUL)); //得到RGB565颜色

			 /* 向每一个像素点填充颜色 */
			 for (j = 0; j < size; j++)
				 base[j] = rgb565_color;
			printf("bpp 16 color.\n");

		 } break;

	case 24: {
			 // RGB888
			 rgb888_t *base = (rgb888_t *)fbp->base;
			 rgb888_t rgb888_color = {
				 .blue = color & 0xFFUL,
				 .green = (color << 8) & 0xFFUL,
				 .red = (color << 16) & 0xFFUL,
			 };

			 for (j = 0; j < size; j++)
				 base[j] = rgb888_color;

		 } break;

	default:
		 fprintf(stderr, "can't surport %dbpp\n", bpp);
		 break;
	}
}

int main(int argc, char **argv) {
	int fd;
	struct fb_var_screeninfo fvsi;
	struct fb_fix_screeninfo ffsi;
	struct fb_config fb;
	int red = 0xff;
	int green = 0xff;
	int blue = 0xff;
	int opt;

	while ((opt = getopt(argc, argv, "r:g:b:")) != -1) {
		switch (opt) {
		case 'r':
			red = 0xff & strtol(optarg, NULL, 0);
			printf("red %d\n", red);
			break;

		case 'g':
			green = 0xff & strtol(optarg, NULL, 0);
			printf("green %d\n", green);
			break;

		case 'b':
			blue = 0xff & strtol(optarg, NULL, 0);
			printf("blue %d\n", blue);
			break;

		default:
			usage(argv[0]);
			break;
		}
	}

	memset(&fb, 0, sizeof(fb));

	if ((fd = open("/dev/fb0", O_RDWR)) < 0) {
		perror("open");
		exit(1);
	}

	if (ioctl(fd, FBIOGET_VSCREENINFO, &fvsi) < 0) {
		perror("ioctl(FBIOGET_VSCREENINFO)");
		close(fd);
		exit(1);
	}

	dump_vscreeninfo(&fvsi);

	if (ioctl(fd, FBIOGET_FSCREENINFO, &ffsi) < 0) {
		perror("ioctl(FBIOGET_FSCREENINFO)");
		close(fd);
		exit(1);
	}

	dump_fscreeninfo(&ffsi);

	fb.fd = fd;
	fb.width = fvsi.xres;
	fb.height = fvsi.yres;
	fb.height_virtual = fvsi.yres_virtual;
	fb.bpp = fvsi.bits_per_pixel;
	fb.stride = ffsi.line_length;
	fb.red_offset = fvsi.red.offset;
	fb.red_length = fvsi.red.length;
	fb.green_offset = fvsi.green.offset;
	fb.green_length = fvsi.green.length;
	fb.blue_offset = fvsi.blue.offset;
	fb.blue_length = fvsi.blue.length;
	fb.transp_offset = fvsi.transp.offset;
	fb.transp_length = fvsi.transp.length;
	fb.buffer_num = fb.height_virtual / fb.height;

	fb.base = (char *)mmap((caddr_t)NULL, ffsi.smem_len, PROT_READ | PROT_WRITE,
			       MAP_SHARED, fd, 0);

	if (fb.base == (char *)-1) {
		perror("mmap");
		close(fd);
		exit(1);
	}

	/* 背光关闭 */
	fbdev_blank(fd, true);

	/* 刷背景 */
	set_background_color(&fb, (red << fb.red_offset | green << fb.green_offset | blue << fb.blue_offset));

	/* 背光开启 */
	fbdev_blank(fd, false);

	/* 退出 */
	munmap(fb.base, ffsi.smem_len);

	/* 取消映射 */
	close(fd);

	return 0;
}
