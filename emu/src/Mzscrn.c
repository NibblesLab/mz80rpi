//----------------------------------------------------------------------------
// File:MZscrn.c
//
// mz80rpi:VRAM/CRT Emulation Module based on MZ700WIN
// (c) Nibbles Lab./Oh!Ishi 2017
//----------------------------------------------------------------------------

#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <linux/fb.h>
#include <linux/fs.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <stdbool.h>
#include "mz80rpi.h"
//
#include "z80.h"
//
#include "mzmain.h"
#include "MZhw.h"
#include "mzscrn.h"

// フレームバッファ
#define FB_NAME "/dev/fb0"
static char *fbptr;
static int fd_fb;
static int ssize;
#define YOFFSET (640*30)
int bpp, pitch;

/*
 * 表示画面の初期化
 */
int mz_screen_init(void)
{
//	FILE *fd;
	struct fb_var_screeninfo vinfo;
	struct fb_fix_screeninfo finfo;

	// フレームバッファのオープン
	fd_fb = open(FB_NAME, O_RDWR);
	if (!fd_fb)
	{
		perror("Open Framebuffer device");
		return -1;
	}

	// スクリーン情報取得
	if (ioctl(fd_fb, FBIOGET_FSCREENINFO, &finfo))
	{
		perror("FB fixed info");
		return -1;
	}
	if (ioctl(fd_fb, FBIOGET_VSCREENINFO, &vinfo))
	{
		perror("FB variable info");
		return -1;
	}

	bpp = vinfo.bits_per_pixel;
	pitch = finfo.line_length;
//	printf("X = %d, Y = %d, %d(bpp)\n", vinfo.xres, vinfo.yres, bpp);
	ssize = vinfo.xres * vinfo.yres * bpp / 8;

	// メモリへマッピング
	fbptr = (char *)mmap(0, ssize, PROT_READ|PROT_WRITE, MAP_SHARED, fd_fb, 0);
	if ((int)fbptr == -1)
	{
		perror("Mapping FB");
		return -1;
	}

	// 画面クリア
	for(int ptr = 0; ptr < ssize; ptr++)
	{
		*(fbptr + ptr) = 0;
	}

	// カーソル点滅抑止
	system("setterm -cursor off > /dev/tty0");

	return 0;
}

/*
 * 画面関係の終了処理
 */
void mz_screen_finish(void)
{
	munmap(fbptr, ssize);
	close(fd_fb);
}

/*
 * フォントデータを描画用に展開
 */
int font_load(const char *fontfile)
{
	FILE *fdfont;
	int dcode, line, bit;
	int ch;
	uint16_t c_bright, c_dark, color;

	c_bright = 0x07e0;	// GREEN
	c_dark = 0x0000;

	// CG-ROMデータ
	fdfont = fopen(fontfile, "r");
	if(fdfont == NULL)
	{
		perror("Open font file");
		return -1;
	}

	for(dcode = 0; dcode < 256; dcode++)
	{
		for(line = 0; line < 8; line++)
		{
			ch = fgetc(fdfont);
			for(bit = 0; bit < 8; bit++)
			{
				color = (ch & 0x80) == 0 ? c_dark : c_bright;
				font[dcode][line][bit] = color;
				if(dcode < 128)
				{
					pcg8000_font[dcode][line][bit] = color;
				}
				ch <<= 1;
			}
		}
	}

	fclose(fdfont);
	return 0;
}
/**********************/
/* redraw the screen */
/*********************/
void update_scrn(void)
{
	unsigned char ch;
//	struct timespec h_start, h_end, h_split;
	uint32_t ptr = YOFFSET;
//	int32_t hwait;
//	int32_t htime = 62000;	// 64us
	uint16_t blankdat[8] = {0, 0, 0, 0, 0, 0, 0, 0};

	hw700.retrace=1;
	
	hw700.cursor_cou++;
	if(hw700.cursor_cou>=60)													/* for japan.1/60 */
	{
		hw700.cursor_cou=0;
	}

	for(int cy = 0; cy < 1000; cy+=40)
	{
		for(int cl = 0; cl < 8; cl++)
		{
//			clock_gettime(CLOCK_MONOTONIC_RAW, &h_start);
			for(int cx = 0; cx < 40; cx++)
			{
				ch = mem[VID_START + cx + cy];
				if(hw700.vgate == 0)
				{
					memcpy(fbptr + ptr, blankdat, sizeof(uint16_t) * 8);
				}
				else
				{
					memcpy(fbptr + ptr, (hw700.pcg8000_mode == 0) ? font[ch][cl] : pcg8000_font[ch][cl], sizeof(uint16_t) * 8);
				}
				ptr += 16;
			}
//			clock_gettime(CLOCK_MONOTONIC_RAW, &h_split);
//			do
//			{
//				clock_gettime(CLOCK_MONOTONIC_RAW, &h_end);
//				if(h_end.tv_nsec < h_start.tv_nsec)
//				{
//					hwait = htime - (1000000000 + h_end.tv_nsec - h_start.tv_nsec);
//				}
//				else
//				{
//					hwait = htime - (h_end.tv_nsec - h_start.tv_nsec);
//				}
//			}
//			while(hwait > 0);
		}
	}

//	printf("H-start = %ld, H-end = %ld\n", h_start.tv_nsec, h_end.tv_nsec);
//	printf("H-disp  = %ld\n", h_split.tv_nsec);
//	printf("V-start = %ld, H-end = %ld\n", v_start.tv_nsec, v_end.tv_nsec);
//	printf("V-blank = %ld\n", v_wait.tv_nsec);
}
