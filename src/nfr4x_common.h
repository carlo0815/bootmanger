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

#ifndef _nfr4x_COMMON_H_
#define _nfr4x_COMMON_H_

#define nfr4x_SUCCESS 0
#define nfr4x_ERROR -1

#define nfr4x_DEVICES_DIR "/dev"
#define nfr4x_FB_DEVICE "/dev/fb/0"
#define nfr4x_FB_DEVICE_FAILOVER "/dev/fb0"
//#define nfr4x_INPUT_DEVICE "/dev/input/event0"
#define nfr4x_VIDEO_DEVICE "/dev/dvb/adapter0/video0"
#define nfr4x_LCD_DEVICE "/dev/dbox/lcd0"
#define nfr4x_PROC_STB "/proc/stb"
#define nfr4x_LCD_XRES "/proc/stb/lcd/xres"
#define nfr4x_LCD_YRES "/proc/stb/lcd/yres"
#define nfr4x_LCD_BPP "/proc/stb/lcd/bpp"
#define nfr4x_KERNEL_MTD "/dev/mtd2"
#define nfr4x_MAIN_DIR "/nfr4x"
#define nfr4x_DATA_DIR "open-multiboot"
#define nfr4x_PLUGIN_DIR "/usr/lib/enigma2/python/Plugins/Extensions/NFR4XBoot"
#define nfr4x_SCREEN_WIDTH 1920
#define nfr4x_SCREEN_HEIGHT 1080
#define nfr4x_SCREEN_BPP 32
#define nfr4x_APP_NAME "NFR4XBoot"
#define nfr4x_APP_VERION "1.0"
#define nfr4x_DISPLAY_NAME "NFR4XBoot"
#ifndef nfr4x_DEFAULT_TIMER
#define nfr4x_DEFAULT_TIMER 5
#endif
#define nfr4x_SHOWIFRAME_BIN "/usr/bin/showiframe"
#define nfr4x_VOLATILE_MEDIA_BIN "/etc/init.d/volatile-media.sh"
#define nfr4x_MDEV_BIN "/etc/init.d/mdev"
#define nfr4x_MODUTILS_BIN "/etc/init.d/modutils.sh"
#define nfr4x_INIT_BIN "/sbin/init"
#define nfr4x_SYSVINIT_BIN "/sbin/init.sysvinit"
#define nfr4x_CHROOT_BIN "/usr/sbin/chroot"
#define nfr4x_NANDDUMP_BIN "/usr/sbin/nanddump"
#define nfr4x_NANDWRITE_BIN "/usr/sbin/nandwrite"
#define nfr4x_FLASHERASE_BIN "/usr/sbin/flash_erase"
#define nfr4x_PYTHON_BIN "/usr/bin/python"
#define nfr4x_BRANDING_HELPER_BIN "/sbin/NFR4XBoot-branding-helper.py"
#define nfr4x_DD_BIN "/bin/dd"

#define nfr4x_MENU_ITEM_RADIUS 10
#define nfr4x_MENU_ITEM_HEIGHT 80
#define nfr4x_MENU_ITEM_WIDTH 900
#ifndef nfr4x_MENU_ITEM_COLOR
#define nfr4x_MENU_ITEM_COLOR 0xFA202020
#endif
#ifndef nfr4x_MENU_ITEM_SELECTED_COLOR
#define nfr4x_MENU_ITEM_SELECTED_COLOR 0xFA404040
#endif
#ifndef nfr4x_MENU_ITEM_TEXT_COLOR
#define nfr4x_MENU_ITEM_TEXT_COLOR 0xFFFFFFFF
#endif
#define nfr4x_MENU_ITEM_TEXT_BOTTON_MARGIN 24
#define nfr4x_MENU_ITEM_TEXT_FONT_SIZE 26
#define nfr4x_MENU_BOX_RADIUS 10
#define nfr4x_MENU_BOX_MARGIN 6
#ifndef nfr4x_MENU_BOX_COLOR
#define nfr4x_MENU_BOX_COLOR 0xE0202020
#endif
#define nfr4x_MENU_MAX_ITEMS 8
#define nfr4x_MENU_ARROWS_SIZE 120
#ifndef nfr4x_MENU_ARROWS_COLOR
#define nfr4x_MENU_ARROWS_COLOR 0xFA202020
#endif

#define nfr4x_LCD_LOGO_X 0.1 // 10% of display width
#define nfr4x_LCD_LOGO_Y 0.1 // like the X axis (same margin)
#define nfr4x_LCD_LOGO_SIZE 0.1 // 10% of display width
#define nfr4x_LCD_LOGO_COLOR 0xffffffff

#define nfr4x_LCD_TITLE_X 0.3 // 30% of display width
#define nfr4x_LCD_TITLE_Y 0.19 // 19% of display width (keep proportion with x axis)
#define nfr4x_LCD_TITLE_SIZE 0.05 // 5% of display width
#define nfr4x_LCD_TITLE_COLOR 0xffffffff

#define nfr4x_LCD_SELECTION_Y 0.75 // 75% of display height
#define nfr4x_LCD_SELECTION_SIZE 0.07 // 7% of display width
#define nfr4x_LCD_SELECTION_COLOR 0xffffffff

#define nfr4x_HEADER_X 20
#define nfr4x_HEADER_Y 40
#define nfr4x_HEADER_FONT_SIZE 24
#define nfr4x_HEADER_COLOR 0xFFFFFFFF

#define nfr4x_TIMER_RIGHT_MARGIN 20
#define nfr4x_TIMER_Y 50
#define nfr4x_TIMER_FONT_SIZE 34
#define nfr4x_TIMER_COLOR 0xFFFFFFFF

#endif // _nfr4x_COMMON_H_
