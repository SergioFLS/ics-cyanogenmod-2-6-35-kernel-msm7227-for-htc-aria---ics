/* linux/arch/arm/mach-msm/devices.c
 *
 * Copyright (C) 2008 Google, Inc.
 * Copyright (C) 2007-2009 HTC Corporation.
 * Author: Thomas Tsai <thomas_tsai@htc.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/kernel.h>
#include <linux/platform_device.h>

#include <linux/dma-mapping.h>
#include <mach/msm_iomap.h>
#include <mach/dma.h>
#include "gpio_chip.h"
#include "devices.h"
#include <mach/board.h>
#include <mach/board_htc.h>
#include <mach/msm_hsusb.h>
#include <linux/usb/android_composite.h>

#include <asm/mach/flash.h>
#include <asm/setup.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/partitions.h>
#include <linux/delay.h>
#include <linux/android_pmem.h>
#include <mach/msm_rpcrouter.h>
#include <mach/msm_iomap.h>
#include <asm/mach/mmc.h>
#include <linux/msm_kgsl.h>
#include <mach/dal_axi.h>
#include "proc_comm.h"
#ifdef CONFIG_MACH_PHOTON
#include <mach/htc_acoustic_wince.h>
#endif

static char *df_serialno = "000000000000";
static char *board_sn;

#define MFG_GPIO_TABLE_MAX_SIZE        0x400
static unsigned char mfg_gpio_table[MFG_GPIO_TABLE_MAX_SIZE];

#ifndef CONFIG_ARCH_MSM7X30
struct platform_device *devices[] __initdata = {
	&msm_device_nand,
	&msm_device_smd,
	/* &msm_device_i2c, */
};

void __init msm_add_devices(void)
{
	platform_add_devices(devices, ARRAY_SIZE(devices));
}
#endif

static struct android_pmem_platform_data pmem_pdata = {
	.name = "pmem",
	.allocator_type = PMEM_ALLOCATORTYPE_ALLORNOTHING,
	.cached = 1,
};

static struct android_pmem_platform_data pmem_adsp_pdata = {
	.name = "pmem_adsp",
	.allocator_type = PMEM_ALLOCATORTYPE_BUDDYBESTFIT,
#if defined(CONFIG_ARCH_MSM7227)
	.cached = 1,
#else
	.cached = 0,
#endif
};

static struct android_pmem_platform_data pmem_camera_pdata = {
	.name = "pmem_camera",
	.allocator_type = PMEM_ALLOCATORTYPE_BUDDYBESTFIT,
	.cached = 0,
};

static struct platform_device pmem_device = {
	.name = "android_pmem",
	.id = 0,
	.dev = { .platform_data = &pmem_pdata },
};

static struct platform_device pmem_adsp_device = {
	.name = "android_pmem",
	.id = 1,
	.dev = { .platform_data = &pmem_adsp_pdata },
};

static struct platform_device pmem_camera_device = {
	.name = "android_pmem",
	.id = 4,
	.dev = { .platform_data = &pmem_camera_pdata },
};

static struct resource ram_console_resource[] = {
	{
		.flags	= IORESOURCE_MEM,
	}
};

static struct platform_device ram_console_device = {
	.name = "ram_console",
	.id = -1,
	.num_resources  = ARRAY_SIZE(ram_console_resource),
	.resource       = ram_console_resource,
};

#ifdef CONFIG_MSM_CAMERA_7X30
static struct resource msm_vpe_resources[] = {
       {
               .start  = 0xAD200000,
               .end    = 0xAD200000 + SZ_1M - 1,
               .flags  = IORESOURCE_MEM,
       },
       {
               .start  = INT_VPE,
               .end    = INT_VPE,
               .flags  = IORESOURCE_IRQ,
       },
};

static struct platform_device msm_vpe_device = {
       .name = "msm_vpe",
       .id   = 0,
       .num_resources = ARRAY_SIZE(msm_vpe_resources),
       .resource = msm_vpe_resources,
};
#endif

#if defined(CONFIG_MSM_HW3D)
static struct resource resources_hw3d[] = {
	{
		.start	= 0xA0000000,
		.end	= 0xA00fffff,
		.flags	= IORESOURCE_MEM,
		.name	= "regs",
	},
	{
		.flags	= IORESOURCE_MEM,
		.name	= "smi",
	},
	{
		.flags	= IORESOURCE_MEM,
		.name	= "ebi",
	},
	{
		.start	= INT_GRAPHICS,
		.end	= INT_GRAPHICS,
		.flags	= IORESOURCE_IRQ,
		.name	= "gfx",
	},
};

static struct platform_device hw3d_device = {
	.name		= "msm_hw3d",
	.id		= 0,
	.num_resources	= ARRAY_SIZE(resources_hw3d),
	.resource	= resources_hw3d,
};
#endif

#if defined(CONFIG_MSM_KGSL) && !defined(CONFIG_ARCH_MSM8X60)
static struct resource kgsl_3d0_resources[] = {
	{
		.name   = KGSL_3D0_REG_MEMORY,
		.start  = MSM_GPU_REG_PHYS,
		.end    = MSM_GPU_REG_PHYS + MSM_GPU_REG_SIZE - 1,
		.flags  = IORESOURCE_MEM,
	},
	{
		.name   = "kgsl_phys_memory",
		.flags  = IORESOURCE_MEM,
	},
	{
		.name   = KGSL_3D0_IRQ,
		.start  = INT_GRAPHICS,
		.end    = INT_GRAPHICS,
		.flags  = IORESOURCE_IRQ,
	},
};

static struct kgsl_device_platform_data kgsl_3d0_pdata = {
	.pwr_data = {
		.pwrlevel = {
			{
				.gpu_freq = 0,
				.bus_freq = 160000000,
			},
		},
		.init_level = 0,
		.num_levels = 1,
		.set_grp_async = NULL,
		.idle_timeout = HZ/2,
	},
	.clk = {
		.name = {
			.clk = "grp_clk",
			.pclk = "grp_pclk",
		},
	},
	.imem_clk_name = {
		.clk = "imem_clk",
	}
};

static struct platform_device kgsl_3d0_device = {
	.name		= "kgsl-3d0",
	.id		= 0,
	.resource	= kgsl_3d0_resources,
	.num_resources	= ARRAY_SIZE(kgsl_3d0_resources),
	.dev = {
		.platform_data = &kgsl_3d0_pdata,
	},
};

#if !defined(CONFIG_ARCH_MSM7X30)
#define PWR_RAIL_GRP_CLK               8
static int kgsl_power_rail_mode(int follow_clk)
{
	int mode = follow_clk ? 0 : 1;
	int rail_id = PWR_RAIL_GRP_CLK;

	return msm_proc_comm(PCOM_CLKCTL_RPC_RAIL_CONTROL, &rail_id, &mode);
}

static int kgsl_power(bool on)
{
	int cmd;
	int rail_id = PWR_RAIL_GRP_CLK;

	cmd = on ? PCOM_CLKCTL_RPC_RAIL_ENABLE : PCOM_CLKCTL_RPC_RAIL_DISABLE;
	return msm_proc_comm(cmd, &rail_id, NULL);
}
#endif /* !CONFIG_ARCH_MSM7X30 */

#endif /* CONFIG_MSM_KGSL */

void __init msm_add_mem_devices(struct msm_pmem_setting *setting)
{
	if (setting->pmem_size) {
		pmem_pdata.start = setting->pmem_start;
		pmem_pdata.size = setting->pmem_size;
		platform_device_register(&pmem_device);
	}

	if (setting->pmem_adsp_size) {
		pmem_adsp_pdata.start = setting->pmem_adsp_start;
		pmem_adsp_pdata.size = setting->pmem_adsp_size;
		platform_device_register(&pmem_adsp_device);
	}

	if (setting->pmem_camera_size) {
		pmem_camera_pdata.start = setting->pmem_camera_start;
		pmem_camera_pdata.size = setting->pmem_camera_size;
		platform_device_register(&pmem_camera_device);
	}

	if (setting->ram_console_size) {
		ram_console_resource[0].start = setting->ram_console_start;
		ram_console_resource[0].end = setting->ram_console_start
			+ setting->ram_console_size - 1;
		platform_device_register(&ram_console_device);
	}

#if defined(CONFIG_MSM_KGSL) && !defined(CONFIG_ARCH_MSM8X60)
	if (setting->kgsl_size) {
		kgsl_3d0_resources[1].start = setting->kgsl_start;
		kgsl_3d0_resources[1].end   = setting->kgsl_start +
                                              setting->kgsl_size - 1;

#if !defined(CONFIG_ARCH_MSM7X30)
		/*
		 * Due to 7x30 gpu hw bug, we have to apply clk
		 * first then power on gpu, thus we move power on
		 * into kgsl driver.
		 */
		kgsl_power_rail_mode(0);
		kgsl_power(true);
#endif

		platform_device_register(&kgsl_3d0_device);
	}
#endif /* CONFIG_MSM_KGSL */

#ifdef CONFIG_MSM_CAMERA_7X30
		platform_device_register(&msm_vpe_device);
#endif
}

#define PM_LIBPROG      0x30000061
#if (CONFIG_MSM_AMSS_VERSION == 6220) || (CONFIG_MSM_AMSS_VERSION == 6225)
#define PM_LIBVERS      0xfb837d0b
#else
#define PM_LIBVERS      0x10001
#endif

#if 1
static struct platform_device *msm_serial_devices[] __initdata = {
#ifndef CONFIG_ARCH_MSM8X60
	&msm_device_uart1,
	&msm_device_uart2,
	&msm_device_uart3,
	#ifdef CONFIG_SERIAL_MSM_HS
	&msm_device_uart_dm1,
	&msm_device_uart_dm2,
	#endif
#endif
};

int __init msm_add_serial_devices(unsigned num)
{
	if (num > MSM_SERIAL_NUM)
		return -EINVAL;

	return platform_device_register(msm_serial_devices[num]);
}
#endif

#define ATAG_SMI 0x4d534D71
/* setup calls mach->fixup, then parse_tags, parse_cmdline
 * We need to setup meminfo in mach->fixup, so this function
 * will need to traverse each tag to find smi tag.
 */
int __init parse_tag_smi(const struct tag *tags)
{
	int smi_sz = 0, find = 0;
	struct tag *t = (struct tag *)tags;

	for (; t->hdr.size; t = tag_next(t)) {
		if (t->hdr.tag == ATAG_SMI) {
			printk(KERN_DEBUG "find the smi tag\n");
			find = 1;
			break;
		}
	}
	if (!find)
		return -1;

	printk(KERN_DEBUG "parse_tag_smi: smi size = %d\n", t->u.mem.size);
	smi_sz = t->u.mem.size;
	return smi_sz;
}
__tagtable(ATAG_SMI, parse_tag_smi);


#define ATAG_HWID 0x4d534D72
int __init parse_tag_hwid(const struct tag *tags)
{
	int hwid = 0, find = 0;
	struct tag *t = (struct tag *)tags;

	for (; t->hdr.size; t = tag_next(t)) {
		if (t->hdr.tag == ATAG_HWID) {
			printk(KERN_DEBUG "find the hwid tag\n");
			find = 1;
			break;
		}
	}

	if (find)
		hwid = t->u.revision.rev;
	printk(KERN_DEBUG "parse_tag_hwid: hwid = 0x%x\n", hwid);
	return hwid;
}
__tagtable(ATAG_HWID, parse_tag_hwid);

static char *keycap_tag = NULL;
static int __init board_keycaps_tag(char *get_keypads)
{
	if(strlen(get_keypads))
		keycap_tag = get_keypads;
	else
		keycap_tag = NULL;
	return 1;
}
__setup("androidboot.keycaps=", board_keycaps_tag);

void board_get_keycaps_tag(char **ret_data)
{
	*ret_data = keycap_tag;
}
EXPORT_SYMBOL(board_get_keycaps_tag);

static char *cid_tag = NULL;
static int __init board_set_cid_tag(char *get_hboot_cid)
{
	if(strlen(get_hboot_cid))
		cid_tag = get_hboot_cid;
	else
		cid_tag = NULL;
	return 1;
}
__setup("androidboot.cid=", board_set_cid_tag);

void board_get_cid_tag(char **ret_data)
{
	*ret_data = cid_tag;
}
EXPORT_SYMBOL(board_get_cid_tag);

static char *carrier_tag = NULL;
static int __init board_set_carrier_tag(char *get_hboot_carrier)
{
	if(strlen(get_hboot_carrier))
		carrier_tag = get_hboot_carrier;
	else
		carrier_tag = NULL;
	return 1;
}
__setup("androidboot.carrier=", board_set_carrier_tag);

void board_get_carrier_tag(char **ret_data)
{
	*ret_data = carrier_tag;
}
EXPORT_SYMBOL(board_get_carrier_tag);

/* G-Sensor calibration value */
#define ATAG_GS         0x5441001d

unsigned int gs_kvalue;
EXPORT_SYMBOL(gs_kvalue);

static int __init parse_tag_gs_calibration(const struct tag *tag)
{
	gs_kvalue = tag->u.revision.rev;
	printk(KERN_DEBUG "%s: gs_kvalue = 0x%x\n", __func__, gs_kvalue);
	return 0;
}

__tagtable(ATAG_GS, parse_tag_gs_calibration);

/* Proximity sensor calibration values */
#define ATAG_PS         0x5441001c

unsigned int ps_kparam1;
EXPORT_SYMBOL(ps_kparam1);

unsigned int ps_kparam2;
EXPORT_SYMBOL(ps_kparam2);

static int __init parse_tag_ps_calibration(const struct tag *tag)
{
	ps_kparam1 = tag->u.serialnr.low;
	ps_kparam2 = tag->u.serialnr.high;

	printk(KERN_INFO "%s: ps_kparam1 = 0x%x, ps_kparam2 = 0x%x\n",
		__func__, ps_kparam1, ps_kparam2);

	return 0;
}

__tagtable(ATAG_PS, parse_tag_ps_calibration);

unsigned int als_kadc;
EXPORT_SYMBOL(als_kadc);

static int __init parse_tag_als_calibration(const struct tag *tag)
{
	als_kadc = tag->u.als_kadc.kadc;

	return 0;
}

__tagtable(ATAG_ALS, parse_tag_als_calibration);

/* CSA sensor calibration values */
#define ATAG_CSA	0x5441001f

unsigned int csa_kvalue1;
EXPORT_SYMBOL(csa_kvalue1);

unsigned int csa_kvalue2;
EXPORT_SYMBOL(csa_kvalue2);

unsigned int csa_kvalue3;
EXPORT_SYMBOL(csa_kvalue3);

static int __init parse_tag_csa_calibration(const struct tag *tag)
{
	unsigned int *ptr = (unsigned int *)&tag->u;
	csa_kvalue1 = ptr[0];
	csa_kvalue2 = ptr[1];
	csa_kvalue3 = ptr[2];

	printk(KERN_DEBUG "csa_kvalue1 = 0x%x, csa_kvalue2 = 0x%x, "
	"csa_kvalue3 = 0x%x\n", csa_kvalue1, csa_kvalue2, csa_kvalue3);

	return 0;
}
__tagtable(ATAG_CSA, parse_tag_csa_calibration);

/* Gyro/G-senosr calibration values */
#define ATAG_GRYO_GSENSOR	0x54410020
unsigned char gyro_gsensor_kvalue[37];
EXPORT_SYMBOL(gyro_gsensor_kvalue);

static int __init parse_tag_gyro_gsensor_calibration(const struct tag *tag)
{
	int i;
	unsigned char *ptr = (unsigned char *)&tag->u;
	memcpy(&gyro_gsensor_kvalue[0], ptr, sizeof(gyro_gsensor_kvalue));

	printk(KERN_DEBUG "gyro_gs data\n");
	for (i = 0; i < sizeof(gyro_gsensor_kvalue); i++)
		printk(KERN_DEBUG "[%d]:0x%x", i, gyro_gsensor_kvalue[i]);

	return 0;
}
__tagtable(ATAG_GRYO_GSENSOR, parse_tag_gyro_gsensor_calibration);

static int mfg_mode;
int __init board_mfg_mode_init(char *s)
{
	if (!strcmp(s, "normal"))
		mfg_mode = 0;
	else if (!strcmp(s, "factory2"))
		mfg_mode = 1;
	else if (!strcmp(s, "recovery"))
		mfg_mode = 2;
	else if (!strcmp(s, "charge"))
		mfg_mode = 3;
	else if (!strcmp(s, "power_test"))
		mfg_mode = 4;
	else if (!strcmp(s, "offmode_charging"))
		mfg_mode = 5;

	return 1;
}

int board_mfg_mode(void)
{
	return mfg_mode;
}

EXPORT_SYMBOL(board_mfg_mode);

__setup("androidboot.mode=", board_mfg_mode_init);

static int build_flag;

static int __init board_bootloader_setup(char *str)
{
	char temp[strlen(str) + 1];
	char *p = NULL;
	char *build = NULL;
	char *args = temp;

	printk(KERN_INFO "%s: %s\n", __func__, str);

	strcpy(temp, str);

	/*parse the last parameter*/
	while ((p = strsep(&args, ".")) != NULL) build = p;

	if (build) {
		if (strcmp(build, "0000") == 0) {
			printk(KERN_INFO "%s: SHIP BUILD\n", __func__);
			build_flag = SHIP_BUILD;
		} else if (strcmp(build, "2000") == 0) {
			printk(KERN_INFO "%s: ENG BUILD\n", __func__);
			build_flag = ENG_BUILD;
		} else {
			printk(KERN_INFO "%s: default ENG BUILD\n", __func__);
			build_flag = ENG_BUILD;
		}
	}
	return 1;
}
__setup("androidboot.bootloader=", board_bootloader_setup);

int board_build_flag(void)
{
	return build_flag;
}

EXPORT_SYMBOL(board_build_flag);

static int __init board_serialno_setup(char *serialno)
{
	char *str;

	/* use default serial number when mode is factory2 */
	if (board_mfg_mode() == 1 || !strlen(serialno))
		str = df_serialno;
	else
		str = serialno;
#ifdef CONFIG_USB_FUNCTION
	msm_hsusb_pdata.serial_number = str;
#endif
	board_sn = str;
	return 1;
}
__setup("androidboot.serialno=", board_serialno_setup);

char *board_serialno(void)
{
	return board_sn;
}

static int sku_id;
int board_get_sku_tag(void)
{
	return sku_id;
}

#define ATAG_SKUID 0x4d534D73
int __init parse_tag_skuid(const struct tag *tags)
{
	int skuid = 0, find = 0;
	struct tag *t = (struct tag *)tags;

	for (; t->hdr.size; t = tag_next(t)) {
		if (t->hdr.tag == ATAG_SKUID) {
			printk(KERN_DEBUG "find the skuid tag\n");
			find = 1;
			break;
		}
	}

	if (find) {
		skuid = t->u.revision.rev;
		sku_id = skuid;
	}
	printk(KERN_DEBUG "parse_tag_skuid: hwid = 0x%x\n", skuid);
	return skuid;
}
__tagtable(ATAG_SKUID, parse_tag_skuid);

#define ATAG_HERO_PANEL_TYPE 0x4d534D74
int panel_type;
int __init tag_panel_parsing(const struct tag *tags)
{
	panel_type = tags->u.revision.rev;

	printk(KERN_DEBUG "%s: panel type = %d\n", __func__,
		panel_type);

	return panel_type;
}
__tagtable(ATAG_HERO_PANEL_TYPE, tag_panel_parsing);

/* ISL29028 ID values */
#define ATAG_PS_TYPE 0x4d534D77
int ps_type;
EXPORT_SYMBOL(ps_type);
int __init tag_ps_parsing(const struct tag *tags)
{
	ps_type = tags->u.revision.rev;

	printk(KERN_DEBUG "%s: PS type = 0x%x\n", __func__,
		ps_type);

	return ps_type;
}
__tagtable(ATAG_PS_TYPE, tag_ps_parsing);

 /* Touch Controller ID values */
#define ATAG_TP_TYPE 0x4d534D78
int tp_type;
EXPORT_SYMBOL(tp_type);
int __init tag_tp_parsing(const struct tag *tags)
{
	tp_type = tags->u.revision.rev;

	printk(KERN_DEBUG "%s: TS type = 0x%x\n", __func__,
		tp_type);

	return tp_type;
}
__tagtable(ATAG_TP_TYPE, tag_tp_parsing);


#define ATAG_ENGINEERID 0x4d534D75
unsigned engineer_id;
int __init parse_tag_engineerid(const struct tag *tags)
{
	int engineerid = 0, find = 0;
	struct tag *t = (struct tag *)tags;

	for (; t->hdr.size; t = tag_next(t)) {
		if (t->hdr.tag == ATAG_ENGINEERID) {
			printk(KERN_DEBUG "find the engineer tag\n");
			find = 1;
			break;
		}
	}

	if (find) {
		engineer_id = t->u.revision.rev;
		engineerid = t->u.revision.rev;
	}
	printk(KERN_DEBUG "parse_tag_engineerid: 0x%x\n", engineerid);
	return engineerid;
}
__tagtable(ATAG_ENGINEERID, parse_tag_engineerid);

#define ATAG_MFG_GPIO_TABLE 0x59504551
int __init parse_tag_mfg_gpio_table(const struct tag *tags)
{
       unsigned char *dptr = (unsigned char *)(&tags->u);
       __u32 size;

       size = min((__u32)(tags->hdr.size - 2) * sizeof(__u32), (__u32)MFG_GPIO_TABLE_MAX_SIZE);
       memcpy(mfg_gpio_table, dptr, size);
       return 0;
}
__tagtable(ATAG_MFG_GPIO_TABLE, parse_tag_mfg_gpio_table);

char * board_get_mfg_sleep_gpio_table(void)
{
        return mfg_gpio_table;
}
EXPORT_SYMBOL(board_get_mfg_sleep_gpio_table);

static char *emmc_tag;
static int __init board_set_emmc_tag(char *get_hboot_emmc)
{
	if (strlen(get_hboot_emmc))
		emmc_tag = get_hboot_emmc;
	else
		emmc_tag = NULL;
	return 1;
}
__setup("androidboot.emmc=", board_set_emmc_tag);

int board_emmc_boot(void)
{
	if (emmc_tag) {
		if (!strcmp(emmc_tag, "true"))
			return 1;
	}

	return 0;
}

#define ATAG_MEMSIZE 0x5441001e
unsigned memory_size;
int __init parse_tag_memsize(const struct tag *tags)
{
	int mem_size = 0, find = 0;
	struct tag *t = (struct tag *)tags;

	for (; t->hdr.size; t = tag_next(t)) {
		if (t->hdr.tag == ATAG_MEMSIZE) {
			printk(KERN_DEBUG "find the memsize tag\n");
			find = 1;
			break;
		}
	}

	if (find) {
		memory_size = t->u.revision.rev;
		mem_size = t->u.revision.rev;
	}
	printk(KERN_DEBUG "parse_tag_memsize: %d\n", memory_size);
	return mem_size;
}
__tagtable(ATAG_MEMSIZE, parse_tag_memsize);

int __init parse_tag_extdiag(const struct tag *tags)
{
	const struct tag *t = tags;

	for (; t->hdr.size; t = tag_next(t)) {
		if (t->hdr.tag == 0x54410021)
			return t->u.revision.rev;
	}
	return 0;
}

#if defined(CONFIG_ARCH_MSM8X60)
static unsigned int radio_flag;
int __init radio_flag_init(char *s)
{
	radio_flag = simple_strtoul(s, 0, 16);
	return 1;
}
__setup("radioflag=", radio_flag_init);

unsigned int get_radio_flag(void)
{
	return radio_flag;
}
#endif

BLOCKING_NOTIFIER_HEAD(psensor_notifier_list);

int register_notifier_by_psensor(struct notifier_block *nb)
{
	return blocking_notifier_chain_register(&psensor_notifier_list, nb);
}

int unregister_notifier_by_psensor(struct notifier_block *nb)
{
	return blocking_notifier_chain_unregister(&psensor_notifier_list, nb);
}

#if defined(CONFIG_MACH_PHOTON)
/******************************************************************************
 * Acoustic driver settings for photon
 ******************************************************************************/
static struct msm_rpc_endpoint *mic_endpoint = NULL;

void amss_4735_mic_bias_callback(bool on) {
	struct {
			struct rpc_request_hdr hdr;
			uint32_t data;
	} req;

	if (!mic_endpoint)
		mic_endpoint = msm_rpc_connect(0x30000061, 0x10001, 0);
	if (!mic_endpoint) {
		printk(KERN_ERR "%s: couldn't open rpc endpoint\n", __func__);
		return;
	}
	req.data=cpu_to_be32(on);
	msm_rpc_call(mic_endpoint, 0x1c, &req, sizeof(req), 5 * HZ);
}

struct htc_acoustic_wce_amss_data amss_4735_acoustic_data = {
	.volume_table = (MSM_SHARED_RAM_BASE+0x71aa8),
	.wb_volume_table = (MSM_SHARED_RAM_BASE+0x71c42),
	.ce_table = (MSM_SHARED_RAM_BASE+0x7325c),
	.adie_table = (MSM_SHARED_RAM_BASE+0x7265c),
	.codec_table = (MSM_SHARED_RAM_BASE+0x71ddc),
	.voc_cal_field_size = 17,
	.mic_bias_callback = amss_4735_mic_bias_callback,
};

struct platform_device acoustic_device = {
	.name = "htc_acoustic",
	.id = -1,
	.dev = {
		.platform_data = &amss_4735_acoustic_data,
	},
};
#endif

