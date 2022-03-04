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

#ifndef __SIPC_DEBUG_H__
#define __SIPC_DEBUG_H__

#include "core_sc2355.h"

enum {
	SPRDWL_SIPC_DBG_TX,
	SPRDWL_SIPC_DBG_RX,
	SPRDWL_SIPC_DBG_MEM,
	SPRDWL_SIPC_DBG_CMD_TX,
	SPRDWL_SIPC_DBG_DATA_TX,
	SPRDWL_SIPC_DBG_ALL,
	SPRDWL_SIPC_DBG_MAX
};

void sipc_rx_list_dump(void);
void sipc_tx_list_dump(void);
void sipc_rx_mem_dump(void);
void sipc_tx_mem_dump(void);
void sipc_tx_cmd_test(void);
void sipc_tx_data_test(void);
#endif /*__SIPC_DEBUG_H__*/
