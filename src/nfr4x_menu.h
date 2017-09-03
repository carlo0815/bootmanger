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

#ifndef _nfr4x_MENU_H_
#define _nfr4x_MENU_H_

int nfr4x_menu_count();
void nfr4x_menu_set(nfr4x_device_item *items);
nfr4x_device_item *nfr4x_menu_get_selected();
void nfr4x_menu_next();
void nfr4x_menu_prev();
void nfr4x_menu_set_selected(const char *identifier);
void nfr4x_menu_render();

#endif // _nfr4x_MENU_H_
