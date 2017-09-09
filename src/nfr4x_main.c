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
#include <sys/types.h>
#include <linux/input.h>
#include <unistd.h>
#include <fcntl.h>

#include "nfr4x_common.h"
#include "nfr4x_log.h"
#include "nfr4x_freetype.h"
#include "nfr4x_framebuffer.h"
#include "nfr4x_lcd.h"
#include "nfr4x_input.h"
#include "nfr4x_utils.h"
#include "nfr4x_menu.h"

static int nfr4x_timer_enabled;
static int nfr4x_current_timer;
static int nfr4x_timer;
char nfr4x_vumodel[63];

void nfr4x_draw_header()
{
	char tmp[255];
	sprintf(tmp, "%s %s", nfr4x_DISPLAY_NAME, nfr4x_APP_VERION);
	nfr4x_render_text(tmp,
		nfr4x_HEADER_X + 45,
		nfr4x_HEADER_Y,
		400,
		nfr4x_HEADER_COLOR,
		nfr4x_HEADER_FONT_SIZE,
		nfr4x_TEXT_ALIGN_LEFT);
}

void nfr4x_draw_lcd()
{
	char tmp[255];
	sprintf(tmp, "%s %s", nfr4x_DISPLAY_NAME, nfr4x_APP_VERION);
	
	int logo_x = nfr4x_lcd_get_width() * nfr4x_LCD_LOGO_X;
	int logo_y = nfr4x_lcd_get_height() * nfr4x_LCD_LOGO_Y;
	int logo_size = nfr4x_lcd_get_width() * nfr4x_LCD_LOGO_SIZE;
	
	int title_x = nfr4x_lcd_get_width() * nfr4x_LCD_TITLE_X;
	int title_y = nfr4x_lcd_get_height() * nfr4x_LCD_TITLE_Y;
	int title_size = nfr4x_lcd_get_width() * nfr4x_LCD_TITLE_SIZE;

	if (! strcmp(nfr4x_vumodel,""))
	nfr4x_render_lcd_symbol(nfr4x_SYMBOL_LOGO,
		logo_x,
		logo_y,
		0,
		nfr4x_LCD_LOGO_COLOR,
		logo_size,
		nfr4x_TEXT_ALIGN_LEFT);
	else {
		if (! strcmp(nfr4x_vumodel,"duo2"))
			title_y += 2;

		sprintf(tmp, "VU+ %s %s", nfr4x_DISPLAY_NAME, nfr4x_APP_VERION);
		title_x = logo_x;
	}
	nfr4x_render_lcd_text(tmp,
		title_x,
		title_y,
		0,
		nfr4x_LCD_TITLE_COLOR,
		title_size,
		nfr4x_TEXT_ALIGN_LEFT);
}

void nfr4x_draw_timer()
{
	if (nfr4x_timer_enabled) {
		char tmp[255];
		sprintf(tmp, "%d", nfr4x_current_timer);
		nfr4x_render_text(tmp,
			nfr4x_get_screen_width() - (400 + nfr4x_TIMER_RIGHT_MARGIN),
			nfr4x_TIMER_Y,
			400,
			nfr4x_TIMER_COLOR,
			nfr4x_TIMER_FONT_SIZE,
			nfr4x_TEXT_ALIGN_RIGHT);
	}
}

void nfr4x_refresh_gui()
{
	nfr4x_clear_screen();
	nfr4x_lcd_clear();
	
	nfr4x_draw_lcd();
	nfr4x_draw_header();
	nfr4x_draw_timer();
	nfr4x_menu_render();
	
	nfr4x_blit();
	nfr4x_lcd_update();
}

int nfr4x_show_menu()
{
	struct timeval start, end;
	
	if (nfr4x_open_framebuffer() == nfr4x_ERROR)
		return nfr4x_ERROR;
	
	if (nfr4x_init_freetype() == nfr4x_ERROR)
		return nfr4x_ERROR;
	
	if (nfr4x_input_open() == nfr4x_ERROR)
		return nfr4x_ERROR;
	
	nfr4x_lcd_open();
	
	nfr4x_timer_enabled = 1;
	nfr4x_timer = nfr4x_utils_gettimer();
	nfr4x_current_timer = nfr4x_timer;
	gettimeofday(&start, NULL);
	
	nfr4x_refresh_gui();
	
	for(;;) {
		usleep(20000);
		int need_refresh_gui = 0;
		int code = nfr4x_input_get_code();
		if (code == KEY_OK)
			break;
		else if (code == KEY_UP) {
			nfr4x_menu_prev();
			nfr4x_utils_update_background(nfr4x_menu_get_selected());
			need_refresh_gui = 1;
			nfr4x_timer_enabled = 0;
		}
		else if (code == KEY_DOWN) {
			nfr4x_menu_next();
			nfr4x_utils_update_background(nfr4x_menu_get_selected());
			need_refresh_gui = 1;
			nfr4x_timer_enabled = 0;
		}
		
		if (nfr4x_timer_enabled) {
			long mtime, seconds, useconds;
			gettimeofday(&end, NULL);
			
			seconds  = end.tv_sec  - start.tv_sec;
			useconds = end.tv_usec - start.tv_usec;

			mtime = ((seconds) * 1000 + useconds/1000.0) + 0.5;
			int last_value = nfr4x_current_timer;
			nfr4x_current_timer = nfr4x_timer - (mtime / 1000);
			
			if (nfr4x_current_timer != last_value)
				need_refresh_gui = 1;
		}
		
		if (need_refresh_gui)
			nfr4x_refresh_gui();
		
		
		if (nfr4x_current_timer == 0)
			break;
	}

	nfr4x_clear_screen();
	nfr4x_blit();
	
	nfr4x_lcd_clear();
	nfr4x_lcd_update();
	
	nfr4x_lcd_close();
	nfr4x_input_close();
	nfr4x_deinit_freetype();
	nfr4x_close_framebuffer();
	
	return nfr4x_SUCCESS;
}

int main(int argc, char *argv[]) 
{
	int is_rebooting = 0;

	if (argc > 1 && getppid() > 1) {
		nfr4x_utils_sysvinit(NULL, argv[1]);
	}
	else {
		nfr4x_vumodel[0] = '\0';

		nfr4x_utils_init_system();
		nfr4x_device_item *item = NULL;
		nfr4x_device_item *items = NULL;
		char *selected = NULL;
		char *nextboot = NULL;
		if (nfr4x_utils_find_and_mount() == nfr4x_SUCCESS) {
			items = nfr4x_utils_get_images();
			nfr4x_menu_set(items);
			selected = nfr4x_utils_read(nfr4x_SETTINGS_SELECTED);
			if (!selected) {
				selected = malloc(6);
				strcpy(selected, "flash");
			}
			nfr4x_menu_set_selected(selected);
			item = nfr4x_menu_get_selected();
		}
/*
 * by Meo. load_modules moved !
 */
		nfr4x_utils_prepare_destination(item);

		int lock_menu = nfr4x_utils_check_lock_menu();
		int force = nfr4x_utils_read_int(nfr4x_SETTINGS_FORCE);
		if (!force && items) 
		{
			nfr4x_log(LOG_DEBUG, "%-33s: preparing environment...", __FUNCTION__);
			if (!lock_menu) {
				nfr4x_log(LOG_DEBUG, "%-33s: loading modules...", __FUNCTION__);
				nfr4x_utils_load_modules(item);
				if (!nfr4x_utils_file_exists(nfr4x_VIDEO_DEVICE)) {
					nfr4x_utils_load_modules_vugl(item);
				}
				nfr4x_utils_setrctype();
			}
			nfr4x_utils_update_background(item);
			nfr4x_utils_backup_kernel(item);

			nextboot = nfr4x_utils_read(nfr4x_SETTINGS_NEXTBOOT);
			if (nextboot) {
				nfr4x_menu_set_selected(nextboot);
				nfr4x_utils_remove_nextboot();
				item = nfr4x_menu_get_selected();
				nfr4x_utils_update_background(item);
				free(nextboot);
			}
			
			if (!lock_menu) {
				nfr4x_log(LOG_DEBUG, "%-33s: menu enabled", __FUNCTION__);
				FILE *fvu = fopen("/proc/stb/info/vumodel", "r");
				if (fvu) {
					char tmp[63];
					if (fscanf(fvu, "%s", &tmp) == 1) {
						strcpy(nfr4x_vumodel, tmp);
					}
					fclose(fvu);
				}
				nfr4x_log(LOG_DEBUG, "%-33s: boxmodel: %s", __FUNCTION__, nfr4x_vumodel);
				nfr4x_show_menu();
			} else {
				nfr4x_log(LOG_DEBUG, "%-33s: menu disabled", __FUNCTION__);
			}
		}
		else {
			nfr4x_log(LOG_DEBUG, "%-33s: nfr4x_utils_save_int(nfr4x_SETTINGS_FORCE, 0)", __FUNCTION__);
			nfr4x_utils_save_int(nfr4x_SETTINGS_FORCE, 0);
		}

		item = nfr4x_menu_get_selected();
		if ((item && selected && strcmp(selected, item->identifier)) != 0 || (item && strstr(item->identifier, "vti") && !force)) {
			nfr4x_utils_restore_kernel(item);
			nfr4x_utils_save(nfr4x_SETTINGS_SELECTED, item->identifier);
			nfr4x_utils_save_int(nfr4x_SETTINGS_FORCE, 1);
			nfr4x_utils_umount(nfr4x_MAIN_DIR);
			nfr4x_utils_reboot();
			is_rebooting = 1;
		}
		
		if (!is_rebooting) {
			if (item != NULL && strcmp(item->identifier, "flash") != 0)
				nfr4x_utils_remount_media(item);
			nfr4x_utils_umount(nfr4x_MAIN_DIR);
			nfr4x_utils_sysvinit(item, NULL);
		}

		if (items) nfr4x_utils_free_items(items);
		if (selected) free(selected);
	}

	return nfr4x_SUCCESS;
}

