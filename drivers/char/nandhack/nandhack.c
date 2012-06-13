/*
 * This modules maps additional mtd partions of the Motorola Milestone XT720
 *
 * Copyright (C) 2012 Munjeni (modified for HTC Aria)
 * Copyright (C) 2012 Mioze7Ae
 * Copyright (C) 2010 Janne Grunau
 * Copyright (C) 2010 Mike Baker
 *
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/err.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>

/* Note: Inverted sense because these are masks -- bits to be _removed_ */
#define MTD_RO (MTD_WRITEABLE)
#define MTD_RW 0
#define mtd_hack_part(psize, pstart, pname, pflags)	\
	.name 		= pname,				\
	.size 		= psize,				\
	.offset 	= pstart,				\
	.mask_flags 	= pflags,

struct mtd_partition part[] = {
	{mtd_hack_part(0x80000, 0x02400000, "hboot", MTD_RW)},		/* hboot */
	{mtd_hack_part(0xa0000, 0x1ff60000, "misc_rw", MTD_RW)},	/* duplicate misc partition and enable rw */ 
};

static int create_missing_flash_parts(struct device *dev, void *data) {
	struct mtd_info *mtd = NULL;

	printk(KERN_INFO "mtd-hack-rw: device %s\n", dev->init_name);

	mtd = dev_get_drvdata(dev);
	if (!mtd)
		return -1;

	printk(KERN_INFO "mtd-hack-rw: mtd name %s, type %d, size %llu\n",
		 mtd->name, mtd->type, mtd->size);
	add_mtd_partitions(mtd, part, sizeof(part)/sizeof(part[0]));	
	printk(KERN_INFO "mtd-hack-rw: mtd name %s, type %d, size %llu\n",
		 mtd->name, mtd->type, mtd->size);

	return 0;
}

static int __init mtd_init(void) {
	struct device_driver *devdrv;
	int err = 0;

	devdrv = driver_find("msm_nand", &platform_bus_type);
	printk(KERN_INFO "mtd-hack-rw: found driver %s modname %s\n", devdrv->name, devdrv->mod_name);

	err = driver_for_each_device(devdrv, NULL, NULL, create_missing_flash_parts);
	printk(KERN_INFO "mtd hack loaded");

	return 0;
}
 
module_init(mtd_init);

MODULE_LICENSE("GPL");

