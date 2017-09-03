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
#include <linux/fb.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <string.h>
#include <unistd.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#ifdef __sh__
#include <linux/stmfb.h>
#endif

#include "nfr4x_common.h"
#include "nfr4x_utils.h"
#include "nfr4x_log.h"

#ifndef FBIO_BLIT
#define FBIO_SET_MANUAL_BLIT _IOW('F', 0x21, __u8)
#define FBIO_BLIT 0x22
#endif

#define ALPHA(x) (x >> 24) & 0xff;
#define RED(x)   (x >> 16) & 0xff;
#define GREEN(x) (x >> 8) & 0xff;
#define BLUE(x)   x & 0xff;

static int nfr4x_fb_fd = 0;
static unsigned char* nfr4x_fb_map = 0;
static struct fb_var_screeninfo nfr4x_var_screen_info;
static struct fb_fix_screeninfo nfr4x_fix_screen_info;
static int nfr4x_screen_size;

int nfr4x_read_screen_info()
{
	if (ioctl(nfr4x_fb_fd, FBIOGET_FSCREENINFO, &nfr4x_fix_screen_info) == -1) {
		nfr4x_log(LOG_ERROR, "%-33s: cannot read fixed information", __FUNCTION__);
		return nfr4x_ERROR;
	}

	if (ioctl(nfr4x_fb_fd, FBIOGET_VSCREENINFO, &nfr4x_var_screen_info) == -1) {
		nfr4x_log(LOG_ERROR, "%-33s: cannot read variable information", __FUNCTION__);
		return nfr4x_ERROR;
	}

	nfr4x_log(LOG_DEBUG, "%-33s: current mode is %dx%d, %dbpp, stride %d", __FUNCTION__,
		nfr4x_var_screen_info.xres, nfr4x_var_screen_info.yres, nfr4x_var_screen_info.bits_per_pixel, nfr4x_fix_screen_info.line_length);
	
	nfr4x_screen_size = nfr4x_fix_screen_info.smem_len;//nfr4x_var_screen_info.xres * nfr4x_var_screen_info.yres * nfr4x_var_screen_info.bits_per_pixel / 8;

#ifdef __sh__
	nfr4x_screen_size -= 1920*1080*4;
#endif

	return nfr4x_SUCCESS;
}

int nfr4x_set_screen_info(int width, int height, int bpp)
{
	nfr4x_var_screen_info.xres_virtual = nfr4x_var_screen_info.xres = width;
	nfr4x_var_screen_info.yres_virtual = nfr4x_var_screen_info.yres = height;
	nfr4x_var_screen_info.bits_per_pixel = bpp;
	nfr4x_var_screen_info.xoffset = nfr4x_var_screen_info.yoffset = 0;
	nfr4x_var_screen_info.height = 0;
	nfr4x_var_screen_info.width = 0;

#ifndef __sh__
	if (ioctl(nfr4x_fb_fd, FBIOPUT_VSCREENINFO, &nfr4x_var_screen_info) < 0) {
		nfr4x_log(LOG_ERROR, "%-33s: cannot set variable information", __FUNCTION__);
		return nfr4x_ERROR;
	}
#endif
	
	if ((nfr4x_var_screen_info.xres != width) && (nfr4x_var_screen_info.yres != height) && (nfr4x_var_screen_info.bits_per_pixel != bpp)) {
		nfr4x_log(LOG_ERROR, "%-33s: cannot set variable information: got %dx%dx%d instead of %dx%dx%d", __FUNCTION__,
			nfr4x_var_screen_info.xres, nfr4x_var_screen_info.yres, nfr4x_var_screen_info.bits_per_pixel, width, height, bpp);
		return nfr4x_ERROR;
	}
	
	if (ioctl(nfr4x_fb_fd, FBIOGET_FSCREENINFO, &nfr4x_fix_screen_info) == -1) {
		nfr4x_log(LOG_ERROR, "%-33s: cannot read fixed information", __FUNCTION__);
		return nfr4x_ERROR;
	}
	
	return nfr4x_SUCCESS;
}

int nfr4x_map_framebuffer()
{
#ifdef __sh__
	nfr4x_fb_map = (unsigned char *)mmap(0, nfr4x_screen_size, PROT_READ | PROT_WRITE, MAP_SHARED, nfr4x_fb_fd, 1920*1080*4);
#else
	nfr4x_fb_map = (unsigned char *)mmap(0, nfr4x_screen_size, PROT_READ | PROT_WRITE, MAP_SHARED, nfr4x_fb_fd, 0);
#endif
	if (nfr4x_fb_map == MAP_FAILED) {
		nfr4x_log(LOG_ERROR, "failed to map framebuffer device to memory");
		return nfr4x_ERROR;
	}
	
	nfr4x_log(LOG_DEBUG, "%-33s: the framebuffer device was mapped to memory successfully", __FUNCTION__);
	
	return nfr4x_SUCCESS;
}

static unsigned short red[256], green[256], blue[256], trans[256];
int nfr4x_make_palette()
{
	int r = 8, g = 8, b = 4, i;

	struct fb_cmap colormap;
	colormap.start=0;
	colormap.len=256;
	colormap.red=red;
	colormap.green = green;
	colormap.blue = blue;
	colormap.transp=trans;

	int rs = 256 / (r - 1);
	int gs = 256 / (g - 1);
	int bs = 256 / (b - 1);

	for (i = 0; i < 256; i++) {
		colormap.red[i]   = (rs * ((i / (g * b)) % r)) * 255;
		colormap.green[i] = (gs * ((i / b) % g)) * 255;
		colormap.blue[i]  = (bs * ((i) % b)) * 255;
	}

	nfr4x_log(LOG_DEBUG, "%-33s, set color palette disabled: FIXME !!", __FUNCTION__);
// FIXME
/*	
	if (ioctl(nfr4x_fb_fd, FBIOPUTCMAP, &colormap) == -1) {
		nfr4x_log(LOG_ERROR, "failed to set color palette");
		//return nfr4x_ERROR;
		return nfr4x_SUCCESS; // NEED TO BE FIXED FOR VU+ BOXES !!
	}
*/	
	return nfr4x_SUCCESS;
}

int nfr4x_set_manual_blit()
{
	nfr4x_log(LOG_DEBUG, "%-33s: set manual blit", __FUNCTION__);
	
#ifndef __sh__
	unsigned char tmp = 1;
	if (ioctl(nfr4x_fb_fd, FBIO_SET_MANUAL_BLIT, &tmp)) {
		nfr4x_log(LOG_ERROR, "failed to set manual blit");
		return nfr4x_ERROR;
	}
#endif
	
	return nfr4x_SUCCESS;
}

void nfr4x_blit()
{
#ifdef __sh__
	STMFBIO_BLT_DATA    bltData;
	memset(&bltData, 0, sizeof(STMFBIO_BLT_DATA));
	bltData.operation  = BLT_OP_COPY;
	bltData.srcOffset  = 1920*1080*4;
	bltData.srcPitch   = nfr4x_var_screen_info.xres * 4;
	bltData.dstOffset  = 0;
	bltData.dstPitch   = nfr4x_var_screen_info.xres * 4;
	bltData.src_top    = 0;
	bltData.src_left   = 0;
	bltData.src_right  = nfr4x_var_screen_info.xres;
	bltData.src_bottom = nfr4x_var_screen_info.yres;
	bltData.srcFormat  = SURF_BGRA8888;
	bltData.dstFormat  = SURF_BGRA8888;
	bltData.srcMemBase = STMFBGP_FRAMEBUFFER;
	bltData.dstMemBase = STMFBGP_FRAMEBUFFER;
	bltData.dst_top    = 0;
	bltData.dst_left   = 0;
	bltData.dst_right  = nfr4x_var_screen_info.xres;
	bltData.dst_bottom = nfr4x_var_screen_info.yres;
	if (ioctl(nfr4x_fb_fd, STMFBIO_BLT, &bltData ) < 0)
		nfr4x_log(LOG_WARNING, "%-33s: cannot blit the framebuffer", __FUNCTION__);
	if (ioctl(nfr4x_fb_fd, STMFBIO_SYNC_BLITTER) < 0)
		nfr4x_log(LOG_WARNING, "%-33s: cannot sync blit", __FUNCTION__);
#else
	if (ioctl(nfr4x_fb_fd, FBIO_BLIT) == -1)
		nfr4x_log(LOG_WARNING, "%-33s: cannot blit the framebuffer, __FUNCTION__");
#endif
}

int nfr4x_get_screen_width()
{
	return nfr4x_var_screen_info.xres;
}

int nfr4x_get_screen_height()
{
	return nfr4x_var_screen_info.yres;
}

int nfr4x_open_framebuffer()
{
	if (nfr4x_utils_file_exists(nfr4x_FB_DEVICE))
	    nfr4x_fb_fd = open(nfr4x_FB_DEVICE, O_RDWR);
	else
	    nfr4x_fb_fd = open(nfr4x_FB_DEVICE_FAILOVER, O_RDWR);	    
	if (nfr4x_fb_fd == -1) {
		nfr4x_log(LOG_ERROR, "%-33s: cannot open framebuffer device", __FUNCTION__);
		return nfr4x_ERROR;
	}
	nfr4x_log(LOG_DEBUG, "%-33s: the framebuffer device was opened successfully", __FUNCTION__);
	
	if (nfr4x_read_screen_info() == nfr4x_ERROR)
		return nfr4x_ERROR;
	
	if ((nfr4x_var_screen_info.xres != nfr4x_SCREEN_WIDTH)
		|| (nfr4x_var_screen_info.yres != nfr4x_SCREEN_HEIGHT)
		|| (nfr4x_var_screen_info.bits_per_pixel != nfr4x_SCREEN_BPP)) {
			
		if (nfr4x_set_screen_info(nfr4x_SCREEN_WIDTH, nfr4x_SCREEN_HEIGHT, nfr4x_SCREEN_BPP) == nfr4x_ERROR)
			return nfr4x_ERROR;
	
		if (nfr4x_read_screen_info() == nfr4x_ERROR)
			return nfr4x_ERROR;
	}
	
	if (nfr4x_map_framebuffer() == nfr4x_ERROR)
		return nfr4x_ERROR;
	
	if (nfr4x_make_palette() == nfr4x_ERROR)
		return nfr4x_ERROR;
	
	if (nfr4x_set_manual_blit() == nfr4x_ERROR)
		return nfr4x_ERROR;
	
	return nfr4x_SUCCESS;
}

void nfr4x_close_framebuffer()
{
	munmap(nfr4x_fb_map, nfr4x_screen_size);
	close(nfr4x_fb_fd);
}

void nfr4x_clear_screen()
{
	memset(nfr4x_fb_map, '\0', nfr4x_screen_size);
}

void nfr4x_draw_rect(int x, int y, int width, int height, int color)
{
	int i, j;
	long int location = 0;
	unsigned char alpha = ALPHA(color);
	unsigned char red = RED(color);
	unsigned char green = GREEN(color);
	unsigned char blue = BLUE(color);
	
	for (i = y; i < y + height; i++) {
		for (j = x; j < x + width; j++) {
			
			if (i < 0 || j < 0 || i > nfr4x_var_screen_info.yres || j > nfr4x_var_screen_info.xres)
				continue;

			location = ((j + nfr4x_var_screen_info.xoffset) * (nfr4x_var_screen_info.bits_per_pixel / 8)) +
				((i + nfr4x_var_screen_info.yoffset) * nfr4x_fix_screen_info.line_length);

			*(nfr4x_fb_map + location) = blue;
			*(nfr4x_fb_map + location + 1) = green;
			*(nfr4x_fb_map + location + 2) = red;
			*(nfr4x_fb_map + location + 3) = alpha;
		}
	}
}

static inline int nfr4x_is_point_inside_circle(int x, int y, int radius)
{
	if (((x - radius) * (x - radius)) + ((y - radius) * (y - radius)) < radius * radius)
		return 1;
	return 0;
}

void nfr4x_draw_rounded_rect(int x, int y, int width, int height, int color, int radius)
{
	int i, j;
	long int location = 0;
	unsigned char alpha = ALPHA(color);
	unsigned char red = RED(color);
	unsigned char green = GREEN(color);
	unsigned char blue = BLUE(color);
	
	for (i = y; i < y + height; i++) {
		for (j = x; j < x + width; j++) {
			if (i < 0 || j < 0 || i > nfr4x_var_screen_info.yres || j > nfr4x_var_screen_info.xres)
				continue;
			
			int relative_x = j - x;
			int relative_y = i - y;
			
			// top left corner
			if (relative_y < radius && relative_x < radius) {
				if (!nfr4x_is_point_inside_circle(relative_x, relative_y, radius)) {
					continue;
				}
			}
			
			// top right corner
			else if (relative_y < radius && width - relative_x < radius) {
				if (!nfr4x_is_point_inside_circle(width - relative_x, relative_y, radius)) {
					continue;
				}
			}
			
			// bottom left corner
			else if (height - relative_y < radius && relative_x < radius) {
				if (!nfr4x_is_point_inside_circle(relative_x, height - relative_y, radius)) {
					continue;
				}
			}

			// bottom right corner
			else if (height - relative_y < radius && width - relative_x < radius) {
				if (!nfr4x_is_point_inside_circle(width - relative_x, height - relative_y, radius)) {
					continue;
				}
			}

			location = ((j + nfr4x_var_screen_info.xoffset) * (nfr4x_var_screen_info.bits_per_pixel / 8)) +
				((i + nfr4x_var_screen_info.yoffset) * nfr4x_fix_screen_info.line_length);

			*(nfr4x_fb_map + location) = blue;
			*(nfr4x_fb_map + location + 1) = green;
			*(nfr4x_fb_map + location + 2) = red;
			*(nfr4x_fb_map + location + 3) = alpha;
		}
	}
}

static inline unsigned char nfr4x_blend_pixel(unsigned char background, unsigned char foreground, unsigned char foreground_alpha)
{
	return (foreground * (foreground_alpha / 255.0)) + (background * (1.0 - (foreground_alpha / 255.0)));
}

void nfr4x_draw_character(FT_Bitmap* bitmap, FT_Int x, FT_Int y, int color)
{
	int i, j, z = 0;
	long int location = 0;
	unsigned char red = RED(color);
	unsigned char green = GREEN(color);
	unsigned char blue = BLUE(color);
	
	for (i = y; i < y + bitmap->rows; i++) {
		for (j = x; j < x + bitmap->width; j++) {
			if (i < 0 || j < 0 || i > nfr4x_var_screen_info.yres || j > nfr4x_var_screen_info.xres) {
				z++;
				continue;
			}
			
			if (bitmap->buffer[z] != 0x00) {
				location = ((j + nfr4x_var_screen_info.xoffset) * (nfr4x_var_screen_info.bits_per_pixel / 8)) +
					((i + nfr4x_var_screen_info.yoffset) * nfr4x_fix_screen_info.line_length);
			
				if (*(nfr4x_fb_map + location + 3) == 0x00) {
					*(nfr4x_fb_map + location) = blue;
					*(nfr4x_fb_map + location + 1) = green;
					*(nfr4x_fb_map + location + 2) = red;
					*(nfr4x_fb_map + location + 3) = bitmap->buffer[z];
				}
				else {
					*(nfr4x_fb_map + location) = nfr4x_blend_pixel(*(nfr4x_fb_map + location), blue, bitmap->buffer[z]);
					*(nfr4x_fb_map + location + 1) = nfr4x_blend_pixel(*(nfr4x_fb_map + location + 1), green, bitmap->buffer[z]);
					*(nfr4x_fb_map + location + 2) = nfr4x_blend_pixel(*(nfr4x_fb_map + location + 2), red, bitmap->buffer[z]);
					if (bitmap->buffer[z] == 0xff)
						*(nfr4x_fb_map + location + 3) = bitmap->buffer[z];
				}
			}
			z++;
		}
	}
}
