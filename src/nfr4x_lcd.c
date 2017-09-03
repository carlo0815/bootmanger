/*
 *
 * Copyright (C) 2014 Impex-Sat Gmbh & Co.KG
 * Written by Sandro Cavazzoni <sandro@skanetwork.com>
 * All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <string.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "nfr4x_common.h"
#include "nfr4x_log.h"
#include "nfr4x_lcd.h"

#ifndef LCD_IOCTL_ASC_MODE
#define LCDSET					0x1000
#define LCD_IOCTL_ASC_MODE		(21|LCDSET)
#define	LCD_MODE_ASC			0
#define	LCD_MODE_BIN			1
#endif

#define RED(x)   (x >> 16) & 0xff;
#define GREEN(x) (x >> 8) & 0xff;
#define BLUE(x)   x & 0xff;

static int nfr4x_lcd_fd = -1;
static int nfr4x_lcd_width = 0;
static int nfr4x_lcd_height = 0;
static int nfr4x_lcd_stride = 0;
static int nfr4x_lcd_bpp = 0;
static unsigned char *nfr4x_lcd_buffer = NULL;


int nfr4x_lcd_read_value(const char *filename)
{
	int value = 0;
	FILE *fd = fopen(filename, "r");
	if (fd) {
		int tmp;
		if (fscanf(fd, "%x", &tmp) == 1)
			value = tmp;
		fclose(fd);
	}
	return value;
}

int nfr4x_lcd_open()
{
	nfr4x_lcd_fd = open("/dev/dbox/lcd0", O_RDWR);
	if (nfr4x_lcd_fd == -1)
		nfr4x_lcd_fd = open("/dev/lcd0", O_RDWR);
	if (nfr4x_lcd_fd == -1)
		nfr4x_lcd_fd = open("/dev/dbox/oled0", O_RDWR);
	if (nfr4x_lcd_fd == -1)
		nfr4x_lcd_fd = open("/dev/oled0", O_RDWR);
	if (nfr4x_lcd_fd == -1) {
		nfr4x_log(LOG_ERROR, "%-33s: cannot open lcd device", __FUNCTION__);
		return nfr4x_ERROR;
	}

#ifdef nfr4x_HAVE_TEXTLCD
	return nfr4x_SUCCESS;
#endif

	int tmp = LCD_MODE_BIN;
	if (ioctl(nfr4x_lcd_fd, LCD_IOCTL_ASC_MODE, &tmp)) {
		nfr4x_log(LOG_ERROR, "%-33s: failed to set lcd bin mode", __FUNCTION__);
#ifndef nfr4x_DREAMBOX
		return nfr4x_ERROR;
#endif
	}
	
	nfr4x_lcd_width = nfr4x_lcd_read_value(nfr4x_LCD_XRES);
	if (nfr4x_lcd_width == 0) {
		nfr4x_log(LOG_ERROR, "%-33s: cannot read lcd x resolution", __FUNCTION__);
		return nfr4x_ERROR;
	}
	
	nfr4x_lcd_height = nfr4x_lcd_read_value(nfr4x_LCD_YRES);
	if (nfr4x_lcd_height == 0) {
		nfr4x_log(LOG_ERROR, "%-33s: cannot read lcd y resolution", __FUNCTION__);
		return nfr4x_ERROR;
	}
	nfr4x_lcd_bpp = nfr4x_lcd_read_value(nfr4x_LCD_BPP);
	if (nfr4x_lcd_bpp == 0) {
		nfr4x_log(LOG_ERROR, "%-33s: cannot read lcd bpp", __FUNCTION__);
		return nfr4x_ERROR;
	}
	
	nfr4x_lcd_stride = nfr4x_lcd_width * (nfr4x_lcd_bpp / 8);
	nfr4x_lcd_buffer = malloc(nfr4x_lcd_height * nfr4x_lcd_stride);
	
	nfr4x_log(LOG_DEBUG, "%-33s: current lcd is %dx%d, %dbpp, stride %d", __FUNCTION__, nfr4x_lcd_width, nfr4x_lcd_height, nfr4x_lcd_bpp, nfr4x_lcd_stride);


	//vusolo4k and vuultimo4k need a brightness to enable lcd
	int fb =  open("/proc/stb/fp/oled_brightness", O_WRONLY|O_CREAT|O_TRUNC, 0666);
	if (fb) {
	    write(fb,"127",3);
	    close(fb);
	}

	return nfr4x_SUCCESS;
}

int nfr4x_lcd_get_width()
{
	return nfr4x_lcd_width;
}

int nfr4x_lcd_get_height()
{
	return nfr4x_lcd_height;
}

void nfr4x_lcd_clear()
{
	if (!nfr4x_lcd_buffer)
		return;
	
	memset(nfr4x_lcd_buffer, '\0', nfr4x_lcd_height * nfr4x_lcd_stride);
}

void nfr4x_lcd_update()
{
	if (!nfr4x_lcd_buffer)
		return;
	
	write(nfr4x_lcd_fd, nfr4x_lcd_buffer, nfr4x_lcd_height * nfr4x_lcd_stride);
}

void nfr4x_lcd_close()
{
	if (nfr4x_lcd_fd >= 0)
		close(nfr4x_lcd_fd);
	
	if (nfr4x_lcd_buffer)
		free(nfr4x_lcd_buffer);
}

void nfr4x_lcd_draw_character(FT_Bitmap* bitmap, FT_Int x, FT_Int y, int color)
{
	if (!nfr4x_lcd_buffer)
		return;
		
	int i, j, z = 0;
	long int location = 0;
	unsigned char red = RED(color);
	unsigned char green = GREEN(color);
	unsigned char blue = BLUE(color);
	
	red = (red >> 3) & 0x1f;
	green = (green >> 3) & 0x1f;
	blue = (blue >> 3) & 0x1f;
	
	for (i = y; i < y + bitmap->rows; i++) {
		for (j = x; j < x + bitmap->width; j++) {
			if (i < 0 || j < 0 || i > nfr4x_lcd_height || j > nfr4x_lcd_width) {
				z++;
				continue;
			}
			
			if (bitmap->buffer[z] != 0x00) {
				location = (j * (nfr4x_lcd_bpp / 8)) +
					(i * nfr4x_lcd_stride);
			
				if ( nfr4x_lcd_bpp == 32) {
					nfr4x_lcd_buffer[location] = RED(color);
					nfr4x_lcd_buffer[location + 1] = GREEN(color);
					nfr4x_lcd_buffer[location + 2] = BLUE(color) ;
					nfr4x_lcd_buffer[location + 3] = 0xff;
				} else {
					nfr4x_lcd_buffer[location] = red << 3 | green >> 2;
					nfr4x_lcd_buffer[location + 1] = green << 6 | blue << 1;
				}
			}
			// vusolo4k needs alpha channel
			if ( nfr4x_lcd_bpp == 32)
				nfr4x_lcd_buffer[location + 3] = 0xff;
			z++;
		}
	}
}

void nfr4x_lcd_write_text(const char* text)
{
	if(nfr4x_lcd_fd < 0)
		return;

	write(nfr4x_lcd_fd, text, strlen(text));
}
