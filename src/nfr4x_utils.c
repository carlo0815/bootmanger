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
#include <dirent.h>
#include <string.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <mntent.h>
#include <unistd.h>

#include "nfr4x_common.h"
#include "nfr4x_log.h"
#include "nfr4x_utils.h"
#include "nfr4x_branding.h"

#define nfr4x_FS_MAX 3
static const char *nfr4x_utils_fs_types[nfr4x_FS_MAX] = { "ext4", "ext3" };

int nfr4x_utils_dir_exists(const char* folder)
{
	DIR *fd = opendir(folder);
	if (fd) {
		closedir(fd);
		return 1;
	}
	return 0;
}

int nfr4x_utils_file_exists(const char* filename)
{
	struct stat st;
	int result = stat(filename, &st);
	return result == 0;
}

void nfr4x_utils_create_dir_tree()
{
	char tmp[255];
	if (!nfr4x_utils_dir_exists(nfr4x_MAIN_DIR))
		mkdir(nfr4x_MAIN_DIR, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	
	sprintf(tmp, "%s/.kernels", nfr4x_MAIN_DIR);
	if (!nfr4x_utils_dir_exists(tmp))
		mkdir(tmp, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
}

int nfr4x_utils_mount(const char* device, const char* mountpoint)
{
	int i;
	for (i = 0; i < nfr4x_FS_MAX; i++)
		if (mount(device, mountpoint, nfr4x_utils_fs_types[i], 0, NULL) == 0)
			return nfr4x_SUCCESS;
	
	return nfr4x_ERROR;
}

int nfr4x_utils_is_mounted(const char *mountpoint)
{
	FILE* mtab = NULL;
	struct mntent* part = NULL;
	int is_mounted = 0;
	
	if ((mtab = setmntent("/etc/mtab", "r")) != NULL) {
		while ((part = getmntent(mtab)) != NULL) {
			if (part->mnt_dir != NULL
				&& strcmp(part->mnt_dir, mountpoint) == 0) {
					
				is_mounted = 1;
			}
		}
		endmntent(mtab);
	}
	
	return is_mounted;
}

int nfr4x_utils_umount(const char* mountpoint)
{
	return umount(mountpoint) == 0 ? nfr4x_SUCCESS : nfr4x_ERROR;
}

void nfr4x_utils_remount_media(nfr4x_device_item *item)
{
	FILE* mtab = NULL;
	struct mntent* part = NULL;
	char media[255];
	char base[255];
	char vol[255];
	sprintf(media, "%s/%s/%s/media", nfr4x_MAIN_DIR, nfr4x_DATA_DIR, item->identifier);
	sprintf(base, "%s/%s/%s", nfr4x_MAIN_DIR, nfr4x_DATA_DIR, item->identifier);
	sprintf(vol, "%s/%s/%s/etc/init.d/volatile-media.sh", nfr4x_MAIN_DIR, nfr4x_DATA_DIR, item->identifier);
	
	if (nfr4x_utils_file_exists(vol)) {
		nfr4x_log(LOG_DEBUG, "%-33s:remount /media into %s", __FUNCTION__, media);
		if (!nfr4x_utils_is_mounted(media))
			if (mount("tmpfs", media, "tmpfs", 0, "size=64k") != 0)
				nfr4x_log(LOG_ERROR, "%-33s: cannot mount %s", __FUNCTION__, media);
	}		
	if ((mtab = setmntent("/etc/mtab", "r")) != NULL) {
		while ((part = getmntent(mtab)) != NULL) {
			if (part->mnt_dir != NULL
				&& strlen(part->mnt_dir) > 6
				&& memcmp(part->mnt_dir, "/media", 6) == 0) {
					char tmp[255];
					sprintf(tmp, "%s/%s", base, part->mnt_dir);
					
					if (nfr4x_utils_umount(part->mnt_dir) == nfr4x_ERROR)
						nfr4x_log(LOG_WARNING, "%-33s: cannot umount %s", __FUNCTION__, part->mnt_dir);
					
					if (!nfr4x_utils_dir_exists(tmp))
						mkdir(tmp, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

					if (nfr4x_utils_mount(part->mnt_fsname, tmp) == nfr4x_ERROR)
						nfr4x_log(LOG_WARNING, "%-33s: cannot mount %s", __FUNCTION__, tmp);
				}
		}
		endmntent(mtab);
	}

	if (nfr4x_utils_umount("/media") == nfr4x_ERROR)
		nfr4x_log(LOG_WARNING, "%-33s: cannot umount /media", __FUNCTION__);
}

int nfr4x_utils_find_and_mount()
{
	struct dirent *dir;
	DIR *fd = opendir(nfr4x_DEVICES_DIR);
	if (fd) {
		nfr4x_utils_create_dir_tree();
		
		while ((dir = readdir(fd)) != NULL) {
			if (((strlen(dir->d_name) == 7 || strlen(dir->d_name) == 9) && memcmp(dir->d_name, "mmc", 3) == 0) || ((strlen(dir->d_name) == 3 || strlen(dir->d_name) == 4) && memcmp(dir->d_name, "sd", 2) == 0)) {
				char device[255];
				sprintf(device, "%s/%s", nfr4x_DEVICES_DIR, dir->d_name);
				nfr4x_log(LOG_DEBUG, "%-33s: check device %s", __FUNCTION__, device);
				
				nfr4x_utils_umount(nfr4x_MAIN_DIR); // just force umount without check
				if (nfr4x_utils_mount(device, nfr4x_MAIN_DIR) == nfr4x_SUCCESS) {
					char datadir[255];
					sprintf(datadir, "%s/%s", nfr4x_MAIN_DIR, nfr4x_DATA_DIR);
					if (nfr4x_utils_dir_exists(datadir)) {
						nfr4x_log(LOG_DEBUG, "%-33s: found data on device %s", __FUNCTION__, device);
						closedir(fd);
						return nfr4x_SUCCESS;
					}
					
					if (nfr4x_utils_umount(nfr4x_MAIN_DIR) == nfr4x_ERROR)
						nfr4x_log(LOG_ERROR, "%-33s: cannot umount %s", __FUNCTION__, nfr4x_MAIN_DIR);
				}
			}
		}	
		closedir(fd);
	}
	return nfr4x_ERROR;
}

nfr4x_device_item *nfr4x_utils_get_images()
{
	struct dirent *dir;
	char datadir[255];
	DIR *fd;
	
	nfr4x_device_item *first = NULL;
	nfr4x_device_item *last = NULL;
	
	nfr4x_log(LOG_DEBUG, "%-33s: discover images", __FUNCTION__);
	
	nfr4x_device_item *item = nfr4x_branding_read_info("", "flash");
	if (item != NULL) {
		if (first == NULL)
			first = item;
		if (last != NULL)
			last->next = item;
		last = item;
	}

	sprintf(datadir, "%s/%s", nfr4x_MAIN_DIR, nfr4x_DATA_DIR);
	fd = opendir(datadir);
	if (fd) {
		while ((dir = readdir(fd)) != NULL) {
			if (strlen(dir->d_name) > 0 && dir->d_name[0] != '.') {
				char base_dir[255];
				sprintf(base_dir, "%s/%s", datadir, dir->d_name);

				if (!nfr4x_branding_is_compatible(base_dir)) {
					nfr4x_log(LOG_DEBUG ,"%-33s: skipping image %s", __FUNCTION__, base_dir);
					continue;
				}

				nfr4x_device_item *item = nfr4x_branding_read_info(base_dir, dir->d_name);
				if (item != NULL) {
					if (first == NULL)
						first = item;
					if (last != NULL)
						last->next = item;
					last = item;
				}
			}
		}
		closedir(fd);
	}
	return first;
}

void nfr4x_utils_free_items(nfr4x_device_item *items)
{
	nfr4x_device_item *tmp = items;
	while (tmp) {
		nfr4x_device_item *tmp2 = tmp;
		tmp = tmp->next;
			
		free(tmp2->label);
		free(tmp2->directory);
		free(tmp2->identifier);
		free(tmp2);
	}
}

void nfr4x_utils_update_background(nfr4x_device_item *item)
{
	char tmp[255];
	sprintf(tmp, "%s %s/usr/share/bootlogo.mvi", nfr4x_SHOWIFRAME_BIN, item->directory);
	system(tmp);
}

void nfr4x_utils_remove_nextboot()
{
	char tmp[255];
	sprintf(tmp, "%s/%s/.%s", nfr4x_MAIN_DIR, nfr4x_DATA_DIR, nfr4x_SETTINGS_NEXTBOOT);
	if(nfr4x_utils_file_exists(tmp)) {
		char cmd[255];
		sprintf(cmd, "rm -rf %s", tmp);
		system(cmd);
	}
}

int nfr4x_utils_gettimer()
{
	char tmp[255];
	sprintf(tmp, "%s/%s/.%s", nfr4x_MAIN_DIR, nfr4x_DATA_DIR, nfr4x_SETTINGS_TIMER);
	if(nfr4x_utils_file_exists(tmp)) {
		char *tmp = nfr4x_utils_read(nfr4x_SETTINGS_TIMER);
		if (tmp) {
			int ret = atoi(tmp);
			free(tmp);
			return ret;
		}
	}
	return nfr4x_DEFAULT_TIMER;
}

void nfr4x_utils_setrctype()
{
	char tmp[255];
	sprintf(tmp, "%s/%s/.%s", nfr4x_MAIN_DIR, nfr4x_DATA_DIR, nfr4x_SETTINGS_RCTYPE);
	if(nfr4x_utils_file_exists(tmp)) {
		char *tmp = nfr4x_utils_read(nfr4x_SETTINGS_RCTYPE);
		if (tmp) {
			int ret = atoi(tmp);
			free(tmp);
			if (ret) {
				char cmd[255];
				sprintf(cmd, "echo %d > /proc/stb/ir/rc/type", ret);
				system(cmd);
			}
		}
	}
}

void nfr4x_utils_save(const char* key, const char* value)
{
	char tmp[255];
	sprintf(tmp, "%s/%s/.%s", nfr4x_MAIN_DIR, nfr4x_DATA_DIR, key);
	FILE *fd = fopen(tmp, "w");
	if (fd) {
		fwrite(value, 1, strlen(value), fd);
		fclose(fd);
		sync();
	}
}

int nfr4x_utils_check_lock_menu()
{
	char tmp[255];
	sprintf(tmp, "%s/%s/.bootmenu.lock", nfr4x_MAIN_DIR, nfr4x_DATA_DIR);
	if (nfr4x_utils_file_exists(tmp)) {
		nfr4x_log(LOG_DEBUG ,"%-33s: bootmenu disabled!", __FUNCTION__);
		return 1;
	}
	
	return 0;
	
}

char* nfr4x_utils_read(const char *key)
{
	char tmp[255];
	sprintf(tmp, "%s/%s/.%s", nfr4x_MAIN_DIR, nfr4x_DATA_DIR, key);
	FILE *fd = fopen(tmp, "r");
	if (fd) {
		char line[1024];
		if (fgets(line, 1024, fd)) {
			strtok(line, "\n");
			char *ret = malloc(strlen(line) + 1);
			strcpy(ret, line);
			fclose(fd);
			return ret;
		}
		fclose(fd);
	}
	return NULL;
}

void nfr4x_utils_save_int(const char* key, int value)
{
	char tmp[255];
	sprintf(tmp, "%d", value);
	nfr4x_utils_save(key, tmp);
}

int nfr4x_utils_read_int(const char *key)
{
	int ret = 0;
	char *tmp = nfr4x_utils_read(key);
	if (tmp) {
		ret = atoi(tmp);
		free(tmp);
	}
	nfr4x_log(LOG_DEBUG, "%-33s: selected %d", __FUNCTION__, ret);
	return ret;
}

void nfr4x_utils_build_platform_wrapper(nfr4x_device_item *item)
{
	FILE *fp;
	char tmp[255];
	char cmd[512];
	
	sprintf(tmp, "%s/%s/%s/usr/bin/platform-util-wrapper.sh", nfr4x_MAIN_DIR, nfr4x_DATA_DIR, item->identifier);
 	fp = fopen(tmp,"w");
 	fprintf(fp,"%s","#!/bin/sh\n\n");
 	fprintf(fp,"%s","export PATH=/usr/local/bin:/usr/bin:/bin:/usr/local/sbin:/usr/sbin:/sbin\n");
 	fprintf(fp,"%s","/etc/init.d/vuplus-platform-util start\n");
	fprintf(fp,"%s","/etc/init.d/platform-util start\n");
	fprintf(fp,"%s","/etc/init.d/gigablue-platform-util start\n");
 	fclose(fp);
 	
 	sprintf(cmd, "chmod 0755 %s/%s/%s/usr/bin/platform-util-wrapper.sh", nfr4x_MAIN_DIR, nfr4x_DATA_DIR, item->identifier);
 	system(cmd);
}

void nfr4x_utils_init_system()
{
	nfr4x_log(LOG_DEBUG, "%-33s: mount /proc", __FUNCTION__);
	if (!nfr4x_utils_is_mounted("/proc"))
		if (mount("proc", "/proc", "proc", 0, NULL) != 0)
			nfr4x_log(LOG_ERROR, "%-33s: cannot mount /proc", __FUNCTION__);
	
	nfr4x_log(LOG_DEBUG, "%-33s: mount /sys", __FUNCTION__);
	if (!nfr4x_utils_is_mounted("/sys"))
		if (mount("sysfs", "/sys", "sysfs", 0, NULL) != 0)
			nfr4x_log(LOG_ERROR, "%-33s: cannot mount /sys", __FUNCTION__);
	
	nfr4x_log(LOG_DEBUG, "%-33s: mount /media", __FUNCTION__);
	if (!nfr4x_utils_is_mounted("/media"))
		if (mount("tmpfs", "/media", "tmpfs", 0, "size=64k") != 0)
			nfr4x_log(LOG_ERROR, "%-33s: cannot mount /media", __FUNCTION__);

	nfr4x_log(LOG_DEBUG, "%-33s: run volatile media", __FUNCTION__);
	system(nfr4x_VOLATILE_MEDIA_BIN);

	nfr4x_log(LOG_DEBUG, "%-33s: run mdev", __FUNCTION__);
	system(nfr4x_MDEV_BIN);
	
	// we really need this sleep?? :( - (wait for mdev to finalize)
	sleep(5);
}

/*
 **
 * by Meo.
 * We don't need to load drivers when we have not to show the bootmenu (force = 0).
 * So we split the original load_modules function to load drivers
 * only when needed.
 **
 */

void nfr4x_utils_prepare_destination(nfr4x_device_item *item)
{	
	nfr4x_log(LOG_DEBUG, "%-33s: prepare destination", __FUNCTION__);

	if (item != NULL && strcmp(item->identifier, "flash") != 0)
	{
		char dev[255];
		char proc[255];
		char sys[255];
		char nfr4x[255];
		char nfr4x_plugin[255];
		sprintf(dev, "%s/%s/%s/dev", nfr4x_MAIN_DIR, nfr4x_DATA_DIR, item->identifier);
		sprintf(proc, "%s/%s/%s/proc", nfr4x_MAIN_DIR, nfr4x_DATA_DIR, item->identifier);
		sprintf(sys, "%s/%s/%s/sys", nfr4x_MAIN_DIR, nfr4x_DATA_DIR, item->identifier);
		sprintf(nfr4x, "%s/%s/%s/%s", nfr4x_MAIN_DIR, nfr4x_DATA_DIR, item->identifier, nfr4x_MAIN_DIR);
		sprintf(nfr4x_plugin, "%s/%s/%s/%s", nfr4x_MAIN_DIR, nfr4x_DATA_DIR, item->identifier, nfr4x_PLUGIN_DIR);
		
		if (!nfr4x_utils_is_mounted(dev))
			if (mount("/dev", dev, NULL, MS_BIND, NULL) != 0)
				nfr4x_log(LOG_ERROR, "%-33s: cannot bind /dev to %s", __FUNCTION__, dev);
		
		if (!nfr4x_utils_is_mounted(proc))
			if (mount("/proc", proc, NULL, MS_BIND, NULL) != 0)
				nfr4x_log(LOG_ERROR, "%-33s: cannot bind /proc to %s", __FUNCTION__, proc);
		
		if (!nfr4x_utils_is_mounted(sys))
			if (mount("/sys", sys, NULL, MS_BIND, NULL) != 0)
				nfr4x_log(LOG_ERROR, "%-33s: cannot bind /sys to %s", __FUNCTION__, sys);

		if (!nfr4x_utils_dir_exists(nfr4x))
			mkdir(nfr4x, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

		if (!nfr4x_utils_is_mounted(nfr4x))
			if (mount(nfr4x_MAIN_DIR, nfr4x, NULL, MS_BIND, NULL) != 0)
				nfr4x_log(LOG_ERROR, "%-33s: cannot bind %s to %s", __FUNCTION__, nfr4x_MAIN_DIR, nfr4x);
				
		if (!nfr4x_utils_dir_exists(nfr4x_plugin))
			mkdir(nfr4x_plugin, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

		if (!nfr4x_utils_is_mounted(nfr4x_plugin))
			if (mount(nfr4x_PLUGIN_DIR, nfr4x_plugin, NULL, MS_BIND, NULL) != 0)
				nfr4x_log(LOG_ERROR, "%-33s: cannot bind %s to %s", __FUNCTION__, nfr4x_PLUGIN_DIR, nfr4x_plugin);
	}
	
}

void nfr4x_utils_load_modules(nfr4x_device_item *item)
{
	int i;
	
	nfr4x_log(LOG_DEBUG, "%-33s: load modules", __FUNCTION__);

	if (item == NULL || strcmp(item->identifier, "flash") == 0) {
		system(nfr4x_MODUTILS_BIN);
	}
	else {
		
		char cmd[512];
		
		sprintf(cmd, "%s %s/%s/%s %s", nfr4x_CHROOT_BIN, nfr4x_MAIN_DIR, nfr4x_DATA_DIR, item->identifier, nfr4x_MODUTILS_BIN);
		system(cmd);
	}
	
	for (i = 0; i < 200; i++) {
		if (nfr4x_utils_file_exists(nfr4x_VIDEO_DEVICE))
			break;
		
		usleep(10000);
	}

#ifdef __sh__
	nfr4x_log(LOG_DEBUG, "%-33s: load lirc", __FUNCTION__);
	if (item == NULL || strcmp(item->identifier, "flash") == 0) {
		system("/etc/init.d/populate-volatile.sh start");
		system("/etc/init.d/lircd start");
	}
	else {
		char cmd[255];
		sprintf(cmd, "%s %s/%s/%s /etc/init.d/populate-volatile.sh start", nfr4x_CHROOT_BIN, nfr4x_MAIN_DIR, nfr4x_DATA_DIR, item->identifier);
		system(cmd);
		sprintf(cmd, "%s %s/%s/%s /etc/init.d/lircd start", nfr4x_CHROOT_BIN, nfr4x_MAIN_DIR, nfr4x_DATA_DIR, item->identifier);
		system(cmd);
	}
#endif

}

/*
 **
 * by Meo.
 * OpenGles modules are loaded at the end of rcS
 * So we need additional stuffs and a different procedure.
 **
 */
void nfr4x_utils_load_modules_gl(nfr4x_device_item *item)
{	
	nfr4x_log(LOG_DEBUG, "%-33s: load platform-util", __FUNCTION__);
	
	int i;

	if (item == NULL || strcmp(item->identifier, "flash") == 0) 
	{
		system("/etc/init.d/mountall.sh start");
		system("/etc/init.d/modload.sh start");
		system("/etc/init.d/modutils.sh start");
		system("/etc/init.d/populate-volatile.sh start");
		system("/etc/init.d/bootmisc.sh start");
		system("/etc/init.d/vuplus-platform-util start");
		system("/etc/init.d/platform-util start");
		system("/etc/init.d/gigablue-platform-util start");		
	}

	else 
	{
		
		char tmp[255];
		char cmd[512];
			

		sprintf(tmp, "%s/%s/%s/etc/init.d/mountrun.sh", nfr4x_MAIN_DIR, nfr4x_DATA_DIR, item->identifier);
		if(nfr4x_utils_file_exists(tmp)) {
			sprintf(cmd, "%s %s/%s/%s /etc/init.d/mountrun.sh start", nfr4x_CHROOT_BIN, nfr4x_MAIN_DIR, nfr4x_DATA_DIR, item->identifier);
			system(cmd);
		}

		sprintf(cmd, "%s %s/%s/%s /etc/init.d/mountall.sh start", nfr4x_CHROOT_BIN, nfr4x_MAIN_DIR, nfr4x_DATA_DIR, item->identifier);
		system(cmd);
		
		sprintf(tmp, "%s/%s/%s/etc/init.d/modload.sh", nfr4x_MAIN_DIR, nfr4x_DATA_DIR, item->identifier);
		if(nfr4x_utils_file_exists(tmp)) {
			sprintf(cmd, "%s %s/%s/%s /etc/init.d/modload.sh start", nfr4x_CHROOT_BIN, nfr4x_MAIN_DIR, nfr4x_DATA_DIR, item->identifier);
			system(cmd);
		}
		
		sprintf(cmd, "%s %s/%s/%s /etc/init.d/modutils.sh start", nfr4x_CHROOT_BIN, nfr4x_MAIN_DIR, nfr4x_DATA_DIR, item->identifier);
		system(cmd);
		
		sprintf(cmd, "%s %s/%s/%s /etc/init.d/populate-volatile.sh start", nfr4x_CHROOT_BIN, nfr4x_MAIN_DIR, nfr4x_DATA_DIR, item->identifier);
		system(cmd);

		sprintf(cmd, "%s %s/%s/%s /etc/init.d/bootmisc.sh start", nfr4x_CHROOT_BIN, nfr4x_MAIN_DIR, nfr4x_DATA_DIR, item->identifier);
		system(cmd);

// prevent missing path in chroot
		nfr4x_utils_build_platform_wrapper(item);

		sprintf(cmd, "%s %s/%s/%s /usr/bin/platform-util-wrapper.sh", nfr4x_CHROOT_BIN, nfr4x_MAIN_DIR, nfr4x_DATA_DIR, item->identifier);
		system(cmd);
		
	}

	for (i = 0; i < 500; i++) {
		if (nfr4x_utils_file_exists(nfr4x_VIDEO_DEVICE))
			break;
		
		usleep(10000);
	}
}

void nfr4x_utils_backup_kernel(nfr4x_device_item *item)
{
	char cmd[512];

	if (!item)
		return;
	
	nfr4x_log(LOG_DEBUG, "%-33s: backup kernel for image '%s'", __FUNCTION__, item->identifier);
#ifdef nfr4x_DREAMBOX
	sprintf(cmd, "%s %s -nof %s/%s/.kernels/%s.bin", nfr4x_NANDDUMP_BIN, nfr4x_KERNEL_MTD, nfr4x_MAIN_DIR, nfr4x_DATA_DIR, item->identifier);
#elif defined(nfr4x_MMCBLK)
	nfr4x_log(LOG_DEBUG, "KERNEL_MTD:  '%s'", nfr4x_KERNEL_MTD);
	if (nfr4x_utils_file_exists(nfr4x_PROC_STB))
		sprintf(cmd, "%s if=%s of=%s/%s/.kernels/%s.bin", nfr4x_DD_BIN, nfr4x_KERNEL_MTD, nfr4x_MAIN_DIR, nfr4x_DATA_DIR, item->identifier);
#else
	sprintf(cmd, "%s %s -f %s/%s/.kernels/%s.bin", nfr4x_NANDDUMP_BIN, nfr4x_KERNEL_MTD, nfr4x_MAIN_DIR, nfr4x_DATA_DIR, item->identifier);
#endif
	system(cmd);
//	nfr4x_log(LOG_DEBUG, "nfr4x_utils_backup_kernel(): cmd: %s");
}

void nfr4x_utils_restore_kernel(nfr4x_device_item *item)
{
	char cmd[512];
	char filename[255];

	if (!item)
		return;
	
	sprintf(filename, "%s/%s/.kernels/%s.bin", nfr4x_MAIN_DIR, nfr4x_DATA_DIR, item->identifier);
	if (nfr4x_utils_file_exists(filename)) {
#ifndef nfr4x_MMCBLK
		nfr4x_log(LOG_DEBUG, "%-33s: erasing MTD", __FUNCTION__);
		sprintf(cmd, "%s %s 0 0", nfr4x_FLASHERASE_BIN, nfr4x_KERNEL_MTD);
		system(cmd);
#endif
	
		nfr4x_log(LOG_DEBUG, "%-33s: restore kernel for image '%s'", __FUNCTION__, item->identifier);
#ifdef nfr4x_DREAMBOX
		sprintf(cmd, "%s -mno %s %s/%s/.kernels/%s.bin", nfr4x_NANDWRITE_BIN, nfr4x_KERNEL_MTD, nfr4x_MAIN_DIR, nfr4x_DATA_DIR, item->identifier);
#elif defined(nfr4x_MMCBLK)
		if (nfr4x_utils_file_exists(nfr4x_PROC_STB))
			sprintf(cmd, "%s of=%s if=%s/%s/.kernels/%s.bin", nfr4x_DD_BIN, nfr4x_KERNEL_MTD, nfr4x_MAIN_DIR, nfr4x_DATA_DIR, item->identifier);
#else
		sprintf(cmd, "%s -pm %s %s/%s/.kernels/%s.bin", nfr4x_NANDWRITE_BIN, nfr4x_KERNEL_MTD, nfr4x_MAIN_DIR, nfr4x_DATA_DIR, item->identifier);
#endif
		system(cmd);
	}
}

void nfr4x_utils_reboot()
{
	nfr4x_utils_sysvinit(NULL, "6");
}

void nfr4x_utils_sysvinit(nfr4x_device_item *item, const char *args)
{
	if (item == NULL || strcmp(item->identifier, "flash") == 0) {
		execl(nfr4x_SYSVINIT_BIN, nfr4x_SYSVINIT_BIN, args, NULL);
	}
	else {
		char path[255];
		char udev[255];
		sprintf(path, "%s/%s/%s", nfr4x_MAIN_DIR, nfr4x_DATA_DIR, item->identifier);
		sprintf(udev, "%s/%s/%s/%s", nfr4x_MAIN_DIR, nfr4x_DATA_DIR, item->identifier, "/etc/init.d/udev");
		if (nfr4x_utils_file_exists(udev))
				system("/etc/init.d/mdev stop");
				
		execl(nfr4x_CHROOT_BIN, nfr4x_CHROOT_BIN, path, nfr4x_INIT_BIN, args, NULL);
	}
}
