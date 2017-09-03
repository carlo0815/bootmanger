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

#include "nfr4x_common.h"
#include "nfr4x_log.h"
#include "nfr4x_freetype.h"
#include "nfr4x_framebuffer.h"
#include "nfr4x_lcd.h"
#include "nfr4x_utils.h"
#include "nfr4x_menu.h"

static nfr4x_device_item *nfr4x_device_items = NULL;
static int nfr4x_menu_offset = 0;
static int nfr4x_menu_selected = 0;

void nfr4x_menu_set(nfr4x_device_item *items)
{
	nfr4x_device_items = items;
}

nfr4x_device_item *nfr4x_menu_get_last()
{
	nfr4x_device_item *tmp = nfr4x_device_items;
	while (tmp) {
		if (!tmp->next)
			return tmp;
		
		tmp = tmp->next;
	}
	return NULL;
}

int nfr4x_menu_count()
{
	int count = 0;
	nfr4x_device_item *tmp = nfr4x_device_items;
	while (tmp) {
		count++;
		tmp = tmp->next;
	}
	return count;
}

nfr4x_device_item *nfr4x_menu_get(int position) {
	nfr4x_device_item *tmp = nfr4x_device_items;
	int count = 0;
	while (tmp) {
		if (count == position)
			return tmp;
		tmp = tmp->next;
		count++;
	}
	return NULL;
}

nfr4x_device_item *nfr4x_menu_get_selected()
{
	return nfr4x_menu_get(nfr4x_menu_selected);
}

void nfr4x_menu_set_selected(const char *identifier)
{
	nfr4x_device_item *tmp = nfr4x_device_items;
	int count = 0;
	while (tmp) {
		if (strcmp(tmp->identifier, identifier) == 0) {
			nfr4x_menu_selected = count;
			break;
		}
		tmp = tmp->next;
		count++;
	}
}

void nfr4x_menu_next()
{
	int position = nfr4x_menu_selected;
	position++;
	if (position >= nfr4x_menu_count())
		position = nfr4x_menu_count() - 1;
	nfr4x_menu_selected = position;
	
	if (position >= nfr4x_menu_offset + nfr4x_MENU_MAX_ITEMS)
		nfr4x_menu_offset = position - nfr4x_MENU_MAX_ITEMS + 1;
	if (nfr4x_menu_offset < 0)
		nfr4x_menu_offset = 0;
}

void nfr4x_menu_prev()
{
	int position = nfr4x_menu_selected;
	position--;
	if (position < 0)
		position = 0;
	nfr4x_menu_selected = position;
	
	if (position < nfr4x_menu_offset)
		nfr4x_menu_offset = position;
}

void nfr4x_menu_render()
{
	int i;
	int count = nfr4x_menu_count();
	int visible_count = count < nfr4x_MENU_MAX_ITEMS ? count : nfr4x_MENU_MAX_ITEMS;
	int screen_width = nfr4x_get_screen_width();
	int screen_height = nfr4x_get_screen_height();
	int box_width = nfr4x_MENU_ITEM_WIDTH + (nfr4x_MENU_BOX_MARGIN * 2);
	int box_height = (nfr4x_MENU_ITEM_HEIGHT * visible_count) + (nfr4x_MENU_BOX_MARGIN * (visible_count + 1));
	int box_x = (screen_width - box_width) / 2;
	int box_y = (screen_height - box_height) / 2;
	
	nfr4x_draw_rounded_rect(box_x, box_y, box_width, box_height, nfr4x_MENU_BOX_COLOR, nfr4x_MENU_BOX_RADIUS);
	
	if (nfr4x_menu_offset > 0) {
		nfr4x_render_symbol(nfr4x_SYMBOL_ARROW_UP,
			box_x + nfr4x_MENU_BOX_MARGIN,
			box_y - 70,
			nfr4x_MENU_ITEM_WIDTH,
			nfr4x_MENU_ARROWS_COLOR,
			nfr4x_MENU_ARROWS_SIZE,
			nfr4x_TEXT_ALIGN_CENTER);
	}
		
	for (i = nfr4x_menu_offset; i < visible_count + nfr4x_menu_offset; i++) {
		nfr4x_device_item *item = nfr4x_menu_get(i);
		int color = nfr4x_MENU_ITEM_COLOR;
		if (i == nfr4x_menu_selected) {
#ifdef nfr4x_HAVE_TEXTLCD
			nfr4x_lcd_write_text(item->label);
#else
			int selection_y = nfr4x_lcd_get_height() * nfr4x_LCD_SELECTION_Y;
			int selection_size = nfr4x_lcd_get_width() * nfr4x_LCD_SELECTION_SIZE;
			
			nfr4x_render_lcd_text(item->label,
				0,
				selection_y,
				nfr4x_lcd_get_width(),
				nfr4x_LCD_SELECTION_COLOR,
				selection_size,
				nfr4x_TEXT_ALIGN_CENTER);
#endif
			
			color = nfr4x_MENU_ITEM_SELECTED_COLOR;
		}
		
		nfr4x_draw_rounded_rect(box_x + nfr4x_MENU_BOX_MARGIN,
			box_y + nfr4x_MENU_BOX_MARGIN,
			nfr4x_MENU_ITEM_WIDTH,
			nfr4x_MENU_ITEM_HEIGHT,
			color,
			nfr4x_MENU_ITEM_RADIUS);
			
		nfr4x_render_text(item->label,
			box_x + nfr4x_MENU_BOX_MARGIN,
			box_y + nfr4x_MENU_BOX_MARGIN + nfr4x_MENU_ITEM_HEIGHT - nfr4x_MENU_ITEM_TEXT_BOTTON_MARGIN,
			nfr4x_MENU_ITEM_WIDTH,
			nfr4x_MENU_ITEM_TEXT_COLOR,
			nfr4x_MENU_ITEM_TEXT_FONT_SIZE,
			nfr4x_TEXT_ALIGN_CENTER);
			
		box_y += nfr4x_MENU_ITEM_HEIGHT + nfr4x_MENU_BOX_MARGIN;
	}
	
	if (nfr4x_menu_offset + nfr4x_MENU_MAX_ITEMS < count) {
		nfr4x_render_symbol(nfr4x_SYMBOL_ARROW_DOWN,
			box_x + nfr4x_MENU_BOX_MARGIN,
			box_y + 20,
			nfr4x_MENU_ITEM_WIDTH,
			nfr4x_MENU_ARROWS_COLOR,
			nfr4x_MENU_ARROWS_SIZE,
			nfr4x_TEXT_ALIGN_CENTER);
	}
}
