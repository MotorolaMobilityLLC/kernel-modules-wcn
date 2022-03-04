/*
 * SPDX-FileCopyrightText: 2015-2022 Unisoc (Shanghai) Technologies Co., Ltd
 * SPDX-License-Identifier: GPL-2.0
 *
 * Copyright 2015-2022 Unisoc (Shanghai) Technologies Co., Ltd
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License
 * as published by the Free Software Foundation.
 */

#ifndef SPRDWL_VERSION_H
#define SPRDWL_VERSION_H

#define SPRDWL_DRIVER_VERSION "Marlin3"
#define SPRDWL_UPDATE "000e"
#define SPRDWL_RESERVE ""
#define MAIN_DRV_VERSION (1)
#define MAX_API			(256)
#define DEFAULT_COMPAT (255)

#define VERSION_1 (1)
#define VERSION_2 (2)
#define VERSION_3 (3)
#define VERSION_4 (4)

struct sprdwl_ver {
	char kernel_ver[8];
	char drv_ver[8];
	char update[8];
	char reserve[8];
};

struct api_version_t {
	unsigned char cmd_id;
	unsigned char drv_version;
	unsigned char fw_version;
};

/*struct used for priv to store all info*/
struct sync_api_verion_t {
	unsigned int compat;
	unsigned int main_drv;
	unsigned int main_fw;
	struct api_version_t *api_array;
};

#endif
