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

#ifndef _nfr4x_LCD_H_
#define _nfr4x_LCD_H_

int nfr4x_lcd_open();
void nfr4x_lcd_close();
int nfr4x_lcd_get_width();
int nfr4x_lcd_get_height();

void nfr4x_lcd_clear();
void nfr4x_lcd_update();

void nfr4x_lcd_draw_character(FT_Bitmap* bitmap, FT_Int x, FT_Int y, int color);
void nfr4x_lcd_write_text(const char* text);

extern char nfr4x_vumodel[63];

#endif // _nfr4x_LCD_H_
