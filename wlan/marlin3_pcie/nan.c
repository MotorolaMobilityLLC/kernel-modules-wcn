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

#include "sprdwl.h"
#include "nan.h"

/* macro */
#define NAN_RSP_LEN 128

/* structure */

/* cmd handler*/

int sprdwl_vendor_nan_cmds(struct wiphy *wiphy,
			   struct wireless_dev *wdev,
			   const void  *data, int len)
{
	struct sprdwl_msg_buf *msg;
	struct sprdwl_vif *vif = netdev_priv(wdev->netdev);
	u8 rsp[NAN_RSP_LEN] = {0x0};
	u16 rsp_len = NAN_RSP_LEN;
	int ret = 0;

	msg = sprdwl_cmd_getbuf(vif->priv, len,
				vif->ctx_id, 1, WIFI_CMD_NAN);
	if (!msg)
		return -ENOMEM;

	memcpy(msg->data, data, len);
	ret = sprdwl_cmd_send_recv(vif->priv, msg,
				    CMD_WAIT_TIMEOUT, rsp, &rsp_len);

	if (!ret && rsp_len) {
		sprdwl_event_nan(vif, rsp, rsp_len);
	} else {
		wl_err("%s: ret=%d, rsp_len=%d\n", __func__, ret, rsp_len);
	}

	return ret;
}

/* event handler*/
int sprdwl_event_nan(struct sprdwl_vif *vif, u8 *data, u16 len)
{
	struct sprdwl_priv *priv = vif->priv;
	struct wiphy *wiphy = priv->wiphy;
	struct sk_buff *skb;

	/* Alloc the skb for vendor event */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 4, 83)
	skb = cfg80211_vendor_event_alloc(wiphy, &vif->wdev, NLMSG_HDRLEN + len,
#else
	skb = cfg80211_vendor_event_alloc(wiphy, NLMSG_HDRLEN + len,
#endif
					  SPRDWL_VENDOR_EVENT_NAN_INDEX,
					  GFP_KERNEL);
	if (!skb) {
		netdev_info(vif->ndev, "skb alloc failed");
		return -ENOMEM;
	}

	/* Push the data to the skb */
	if (nla_put(skb, SRPDWL_VENDOR_ATTR_NAN, len, data)) {
		netdev_info(vif->ndev, "nla put failed");
		kfree_skb(skb);
		return -1;
	}

	cfg80211_vendor_event(skb, GFP_KERNEL);

	return 0;
}
