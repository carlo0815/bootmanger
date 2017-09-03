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

#ifndef _nfr4x_UTILS_H_
#define _nfr4x_UTILS_H_

typedef struct nfr4x_device_item
{
	char *label;
	char *directory;
	char *identifier;
	struct nfr4x_device_item *next;
} nfr4x_device_item;

#define nfr4x_SETTINGS_SELECTED "selected"
#define nfr4x_SETTINGS_FORCE "force"
#define nfr4x_SETTINGS_NEXTBOOT "nextboot"
#define nfr4x_SETTINGS_TIMER "timer"
#define nfr4x_SETTINGS_RCTYPE "rctype"

int nfr4x_utils_find_and_mount();
nfr4x_device_item *nfr4x_utils_get_images();
void nfr4x_utils_update_background(nfr4x_device_item *item);
void nfr4x_utils_free_items(nfr4x_device_item *items);

void nfr4x_utils_save(const char* key, const char* value);
char* nfr4x_utils_read(const char *key);
void nfr4x_utils_save_int(const char* key, int value);
int nfr4x_utils_read_int(const char *key);
int nfr4x_utils_check_lock_menu();
void nfr4x_utils_build_vu_wrapper(nfr4x_device_item *item);

void nfr4x_utils_remove_nextboot();
int nfr4x_utils_gettimer();
void nfr4x_utils_setrctype();

void nfr4x_utils_init_system();
void nfr4x_utils_prepare_destination(nfr4x_device_item *item);
void nfr4x_utils_load_modules(nfr4x_device_item *item);
void nfr4x_utils_load_modules_vugl(nfr4x_device_item *item);

void nfr4x_utils_backup_kernel(nfr4x_device_item *item);
void nfr4x_utils_restore_kernel(nfr4x_device_item *item);

void nfr4x_utils_remount_media(nfr4x_device_item *item);

void nfr4x_utils_reboot();
void nfr4x_utils_sysvinit(nfr4x_device_item *item, const char *args);

#endif // _nfr4x_UTILS_H_
