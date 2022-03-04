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

#ifndef __SPRDWL_NPI_H__
#define __SPRDWL_NPI_H__

#define SPRDWL_NL_GENERAL_SOCK_ID	(1022)
#define SPRDWL_NPI_CMD_START		(0)
#define SPRDWL_NPI_CMD_SET_WLAN_CAP	(40)
#define SPRDWL_STA_GC_EN_SLEEP		(0x3)
#define SPRDWL_STA_GC_NO_SLEEP		(0x0)
#define SPRDWL_PSM_PATH			"/opt/etc/.psm.info"

#define SPRDWL_NPI_CMD_SET_PROTECTION_MODE 50
#define SPRDWL_NPI_CMD_GET_PROTECTION_MODE 51
#define SPRDWL_NPI_CMD_SET_RTS_THRESHOLD   52

/* enable: 0x0
 * disable: 0x1
 * STA: bit 0
 * GC: bit 1
 */
enum sprdwl_nl_commands {
	SPRDWL_NL_CMD_UNSPEC,
	SPRDWL_NL_CMD_NPI,
	SPRDWL_NL_CMD_GET_INFO,
	SPRDWL_NL_CMD_MAX,
};

enum sprdwl_nl_attrs {
	SPRDWL_NL_ATTR_UNSPEC,
	SPRDWL_NL_ATTR_IFINDEX,
	SPRDWL_NL_ATTR_AP2CP,
	SPRDWL_NL_ATTR_CP2AP,
	SPRDWL_NL_ATTR_MAX,
};

struct sprdwl_npi_cmd_hdr {
	unsigned char type;
	unsigned char subtype;
	unsigned short len;
} __packed;

struct sprdwl_npi_cmd_resp_hdr {
	unsigned char type;
	unsigned char subtype;
	unsigned short len;
	int status;
} __packed;

enum sprdwl_npi_cmd_type {
	SPRDWL_HT2CP_CMD = 1,
	SPRDWL_CP2HT_REPLY,
};

void sprdwl_init_npi(void);
void sprdwl_deinit_npi(void);
#endif /*__NPI_H__*/
