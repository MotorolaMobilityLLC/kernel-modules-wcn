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

#include "rnd_mac_addr.h"

void random_mac_addr(u8 *addr)
{
	get_random_bytes(addr, ETH_ALEN);
	addr[0] &= 0xfe; /* unicast */
	addr[0] |= 0x02; /* locally administered */
}

int wlan_cmd_set_rand_mac(struct sprdwl_priv *priv, u8 vif_ctx_id,
		u8 random_mac_flag, u8 *addr)
{
	struct sprdwl_msg_buf *msg;
	u8 *p;

	msg = sprdwl_cmd_getbuf(priv, ETH_ALEN+1, vif_ctx_id,
			SPRDWL_HEAD_RSP, WIFI_CMD_RND_MAC);
	if (!msg)
		return -ENOMEM;
	p = (u8 *)msg->data;
	*p = random_mac_flag;

	memcpy(p+1, addr, ETH_ALEN);
	return sprdwl_cmd_send_recv(priv, msg,
			CMD_WAIT_TIMEOUT, NULL, NULL);
}
