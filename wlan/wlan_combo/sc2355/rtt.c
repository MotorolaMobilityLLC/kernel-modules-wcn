/*
* SPDX-FileCopyrightText: 2021-2023 Unisoc (Shanghai) Technologies Co. Ltd
* SPDX-License-Identifier: GPL-2.0-only
*/

#include <linux/limits.h>
#include "common/common.h"
#include "common/iface.h"
#include "rtt.h"
#include "cmdevt.h"
#include "sc2355_intf.h"

/* FTM session ID we use with FW */
#define RTT_ESSION_ID			1

/* fixed spare allocation we reserve in NL messages we allocate */
#define RTT_NL_EXTRA_ALLOC		32

/* approx maximum length for FTM_MEAS_RESULT NL80211 event */
#define RTT_MEAS_RESULT_MAX_LENGTH	2048

/* maximum number of allowed FTM measurements per burst */
#define RTT_MAX_MEAS_PER_BURST		31

/* maximum number of allowed RTT bursts, 1U << 15 */
#define RTT_MAX_BURST			32768

/* initial token to use on non-secure FTM measurement */
#define RTT_DEFAULT_INITIAL_TOKEN	2

#define RTT_MAX_LCI_LENGTH		(240)
#define RTT_MAX_LCR_LENGTH		(240)

/* max rtt cmd response length */
#define RTT_RSP_LEN			(128)

#ifndef BITS_PER_LONG
#ifdef CONFIG_64BIT
#define BITS_PER_LONG			64
#else
#define BITS_PER_LONG			32
#endif /* CONFIG_64BIT */
#endif

enum rtt_subcmd {
	RTT_ENABLE,
	RTT_DISABLE,
	RTT_GET_CAPABILITIES,
	RTT_RANGE_REQUEST,
	RTT_RANGE_CANCEL,
	RTT_SET_CLI,
	RTT_SET_CLR,
	RTT_GET_RESPONDER_INFO,
	RTT_ENABLE_RESPONDER,
	RTT_DISABLE_RESPONDER,
};

enum rtt_subevt {
	RTT_SESSION_END,
	RTT_PER_DEST_RES,
};

struct cmd_rtt {
	u8 sub_cmd;
	__le16 len;
	u8 data[];
} __packed;

static void rtt_event_per_dest_res(struct sprd_priv *priv,
				   struct rtt_wifi_result *res)
{
	struct rtt_wifi_hal_result *peer_res;
	s64 rtt_time_tmp = 0, avg_rtt_time = 0, *rtt_time,
	    max_rtt_time = 0, min_rtt_time = 0,
	    pow_rtt_time = 0, sqrt_rtt_time = 0;
	int i, j, rtt_distance_tmp = 0, avg_rtt_distance = 0, *rtt_distance,
	    max_rtt_distance = 0, min_rtt_distance = 0,
	    pow_rtt_distance = 0, sqrt_rtt_distance = 0;

	mutex_lock(&priv->ftm.lock);

	if (!priv->ftm.session_started) {
		wl_err("%s: Session not running, ignoring res event\n",
		       __func__);
		goto out;
	}

	i = priv->rtt_results.peer_num;
	if (i >= RTT_MAX_PEER_NUM) {
		goto out;
	}
	peer_res = priv->rtt_results.peer_rtt_result[i];
	priv->rtt_results.peer_num++;
	memcpy(peer_res->mac_addr, res->mac_addr, ETH_ALEN);
	peer_res->burst_num = res->params.num_of_bursts_exp;
	peer_res->measurement_number = res->measurement_number;
	peer_res->success_number = res->success_number;
	peer_res->number_per_burst_peer = res->params.meas_per_burst;
	peer_res->status = res->status;
	peer_res->retry_after_duration = res->retry_after_duration;
	peer_res->type = res->type;
	peer_res->rssi = res->rssi;
	peer_res->rssi_spread = res->rssi_spread;
	peer_res->tx_rate = res->tx_rate;
	peer_res->rx_rate = res->rx_rate;

	wl_info("%s, burst_num: %u, measurement_number:%u,\
		success_number:%u, number_per_burst_peer:%u, status:%d\n",
		__func__, peer_res->burst_num, peer_res->measurement_number,
		peer_res->success_number, peer_res->number_per_burst_peer,
		peer_res->status);
	if (res->n_meas > RTT_MAX_RESULT_SUPPORT) {
		wl_err("%s, number exceeds meas array size.\n",
		       __func__);
		peer_res = NULL;
		goto out;
	}
	rtt_time = kzalloc(res->n_meas * sizeof(s64), GFP_KERNEL);
	if (!rtt_time) {
		wl_err("%s, rtt_time alloc failed.\n", __func__);
		goto out;
	}
	rtt_distance = kzalloc(res->n_meas * sizeof(int), GFP_KERNEL);
	if (!rtt_distance) {
		wl_err("%s, rtt_distance alloc failed.\n", __func__);
		kfree(rtt_time);
		rtt_time = NULL;
		goto out;
	}
	for (j = 0; j < res->n_meas; j++) {
		rtt_time_tmp = (s64)((res->meas[j].t4 - res->meas[j].t1)
				     - (res->meas[j].t3 - res->meas[j].t2)) / 2;
		avg_rtt_time += rtt_time_tmp;
		rtt_time[j] =  rtt_time_tmp;
		/* (ps*(10^-12)(s) * 3*10^8(m/s) * 10^3) (mm) */
		rtt_distance_tmp = (int)(div_s64(rtt_time_tmp*3, 10));
		avg_rtt_distance += rtt_distance_tmp;
		rtt_distance[j] = rtt_distance_tmp;

		if (j == 0) {
			max_rtt_time = min_rtt_time = rtt_time_tmp;
			max_rtt_distance = min_rtt_distance = rtt_distance_tmp;
		} else {
			if (max_rtt_time < rtt_time_tmp)
				max_rtt_time = rtt_time_tmp;
			if (min_rtt_time > rtt_time_tmp)
				min_rtt_time = rtt_time_tmp;
			if (max_rtt_distance < rtt_distance_tmp)
				max_rtt_distance = rtt_distance_tmp;
			if (min_rtt_distance > rtt_distance_tmp)
				min_rtt_distance = rtt_distance_tmp;
		}
		wl_info("%s,t1:%llu, t2:%llu, t3:%llu, t4s:%llu,\
			rtt time:%lld, rtt distance:%d!\n",
			__func__, res->meas[j].t1, res->meas[j].t2,
			res->meas[j].t3, res->meas[j].t4, rtt_time[j],
			rtt_distance[j]);
	}

	avg_rtt_time = div_s64(avg_rtt_time, res->n_meas);
	avg_rtt_distance = div_s64(avg_rtt_distance, res->n_meas);

	for (j = 0; j < res->n_meas; j++) {
		pow_rtt_time += (rtt_time[j] - avg_rtt_time) *
				(rtt_time[j] - avg_rtt_time);
		pow_rtt_distance += (rtt_distance[j] - avg_rtt_distance) *
				    (rtt_distance[j] - avg_rtt_distance);
	}

	pow_rtt_time = div_s64(pow_rtt_time, res->n_meas);
	pow_rtt_distance = div_s64(pow_rtt_distance, res->n_meas);
	sqrt_rtt_time = (u64)int_sqrt((unsigned long)pow_rtt_time);
	sqrt_rtt_distance = (int)int_sqrt((unsigned long)pow_rtt_distance);
	kfree(rtt_time);
	rtt_time = NULL;
	kfree(rtt_distance);
	rtt_distance = NULL;

	peer_res->rtt = avg_rtt_time;
	peer_res->rtt_sd = sqrt_rtt_time;
	peer_res->rtt_spread = max_rtt_time - min_rtt_time;
	peer_res->distance_mm = avg_rtt_distance;
	peer_res->distance_sd_mm = sqrt_rtt_distance;
	peer_res->distance_spread_mm = max_rtt_distance - min_rtt_distance;

	wl_info("%s, rtt:%llu, rtt_sd:%llu, rtt_spread:%llu, distance_mm:%d,\
		distance_sd_mm:%d, distance_spread_mm:%d\n",
		__func__, peer_res->rtt, peer_res->rtt_sd, peer_res->rtt_spread,
		peer_res->distance_mm, peer_res->distance_sd_mm,
		peer_res->distance_spread_mm);

	peer_res->ts = res->ts;
	peer_res->burst_duration = res->params.burst_duration;
	peer_res->negotiated_burst_num = 1;

out:
	mutex_unlock(&priv->ftm.lock);
}

static void rtt_event_end(struct sprd_priv *priv)
{
	struct sk_buff *reply = NULL;
	struct wiphy *wiphy = priv->wiphy;
	struct sprd_vif *vif = sprd_mode_to_vif(priv, SPRD_MODE_STATION);
	int i, rlen, ret = 0;
	struct nlattr *nl_res = NULL;

	if (!vif) {
		wl_err("%s, vif is NULL.\n", __func__);
		goto out;
	}
	rlen = priv->rtt_results.peer_num * sizeof(struct rtt_wifi_hal_result);
	reply = cfg80211_vendor_event_alloc(wiphy, &vif->wdev,
					    rlen + NLMSG_HDRLEN + 100,
					    SPRD_RTT_EVENT_COMPLETE_INDEX,
					    GFP_KERNEL);
	if (!reply || !priv->rtt_results.peer_num) {
		wl_err("%s, peer_num is %d\n", __func__,
		       priv->rtt_results.peer_num);
		goto out;
	}

	for (i = 0; i < priv->rtt_results.peer_num; i++) {
		nl_res = nla_nest_start(reply,
					SPRD_RTT_ATTRIBUTE_RESULTS_PER_TARGET);
		if (!nl_res) {
			wl_err("%s, %d\n", __func__, __LINE__);
			goto out;
		}
		if (nla_put(reply, SPRD_RTT_ATTRIBUTE_TARGET_MAC, ETH_ALEN,
			    priv->rtt_results.peer_rtt_result[i]->mac_addr) ||
		    nla_put_u32(reply, SPRD_RTT_ATTRIBUTE_RESULT_CNT, i + 1) ||
		    nla_put(reply, SPRD_RTT_ATTRIBUTE_RESULT,
			    sizeof(struct rtt_wifi_hal_result),
			    priv->rtt_results.peer_rtt_result[i])) {
			wl_err("%s, %d\n", __func__, __LINE__);
			goto out;
		}
		nla_nest_end(reply, nl_res);
	}

	ret = nla_put_u32(reply, SPRD_RTT_ATTRIBUTE_RESULTS_COMPLETE, 1);
	if (ret < 0) {
		wl_err("%s failed to put RTT_ATTRIBUTE_RESULTS_COMPLETE\n", __func__);
		goto out;
	}
	cfg80211_vendor_event(reply, GFP_KERNEL);
	reply = NULL;
	wl_info("report rtt result\n");
	priv->rtt_results.peer_num = 0;
	priv->ftm.session_started = 0;
out:
	if (reply) {
		kfree_skb(reply);
		reply = NULL;
	}
}

int sc2355_rtt_event(struct sprd_vif *vif, u8 *data, u16 len)
{
	struct sprd_priv *priv = vif->priv;
	u8 sub_event;
	struct rtt_wifi_result *res;

	print_hex_dump_debug("rtt result debug: ", DUMP_PREFIX_OFFSET,
			     16, 1, data, len, 0);

	sub_event = *data;
	data++;
	len--;

	switch (sub_event) {
	case RTT_SESSION_END:
		wl_info("rec rtt result completed!\n");
		rtt_event_end(priv);
		break;
	case RTT_PER_DEST_RES:
		wl_info("rec rtt result!\n");
		if (len < sizeof(struct rtt_wifi_result)) {
			netdev_err(vif->ndev, "%s: invalid data len\n", __func__);
			return -1;
		}
		res = (struct rtt_wifi_result *)data;
		rtt_event_per_dest_res(priv, res);
		break;
	default:
		netdev_err(vif->ndev, "%s: unknown FTM event\n", __func__);
		break;
	}
	return 0;
}

int sc2355_rtt_get_capabilities(struct wiphy *wiphy, struct wireless_dev *wdev,
				const void *data, int len)
{
	struct sprd_msg *msg;
	struct cmd_rtt *cmd;
	struct sprd_vif *vif = netdev_priv(wdev->netdev);
	struct sprd_priv *priv = wiphy_priv(wiphy);
	u8 rsp[RTT_RSP_LEN] = { 0x0 };
	u16 rsp_len = sizeof(struct cmd_rtt) +
		      sizeof(struct rtt_capabilities);
	int ret = 0;
	struct sk_buff *skb;
	struct rtt_capabilities *rtt_cap;

	if (U16_MAX < (sizeof(struct cmd_rtt) + len) || len < 0) {
		netdev_err(vif->ndev,
			   "%s: param data len is invalid\n", __func__);
		return -EINVAL;
	}

	wl_info("rtt get capability\n");
	/* get the capabilities from the FW */
	mutex_lock(&priv->ftm.lock);
	msg = get_cmdbuf(vif->priv, vif, rsp_len, CMD_RTT);
	if (!msg) {
		mutex_unlock(&priv->ftm.lock);
		return -ENOMEM;
	}
	cmd = (struct cmd_rtt *)msg->data;
	cmd->sub_cmd = RTT_GET_CAPABILITIES;
	cmd->len = len;
	memcpy(cmd->data, data, len);

	ret = send_cmd_recv_rsp(vif->priv, msg, rsp, &rsp_len);
	if (ret) {
		netdev_err(vif->ndev,
			   "%s: ret=%d, rsp_len=%d\n", __func__, ret, rsp_len);
		mutex_unlock(&priv->ftm.lock);
		return ret;
	}

	rtt_cap = (struct rtt_capabilities *)(rsp + sizeof(struct cmd_rtt));
	wl_info("rtt cap:%d %d %d %d %d %d %d %d\n",
		rtt_cap->rtt_one_sided_supported,
		rtt_cap->rtt_ftm_supported, rtt_cap->lci_support,
		rtt_cap->lcr_support, rtt_cap->preamble_support,
		rtt_cap->bw_support, rtt_cap->responder_supported,
		rtt_cap->mc_version);

	/* report capabilities */
	skb = cfg80211_vendor_cmd_alloc_reply_skb(wiphy, rsp_len);
	if (!skb)
		goto nla_put_failure;
	if (nla_put_nohdr(skb, sizeof(struct rtt_capabilities), rtt_cap) ||
	    nla_put_u32(skb, NL80211_ATTR_VENDOR_ID, OUI_SPREAD) ||
	    nla_put_u32(skb, NL80211_ATTR_VENDOR_SUBCMD,
			SPRD_NL80211_SUBCMD_LOC_GET_CAPA))
		goto nla_put_failure;

	mutex_unlock(&priv->ftm.lock);
	return cfg80211_vendor_cmd_reply(skb);
nla_put_failure:
	kfree_skb(skb);
	skb = NULL;
	mutex_unlock(&priv->ftm.lock);
	return -ENOMEM;
}

int rtt_target_info_handler(const struct nlattr *iter,
			    struct rtt_meas_peer_info *peer_info)
{
	const struct nlattr *iter2;
	int rem2, type;

	nla_for_each_nested(iter2, iter, rem2) {
		type = nla_type(iter2);
		switch (type) {
		case SPRD_RTT_ATTRIBUTE_TARGET_MAC:
			memcpy(peer_info->mac_addr, nla_data(iter2), ETH_ALEN);
			wl_info("mac: %pM\n", peer_info->mac_addr);
			break;
		case SPRD_RTT_ATTRIBUTE_TARGET_TYPE:
			peer_info->wifi_rtt_type = nla_get_u8(iter2);
			wl_info("rtt type = %d\n", peer_info->wifi_rtt_type);
			break;
		case SPRD_RTT_ATTRIBUTE_TARGET_PEER:
			peer_info->rtt_peer_type = nla_get_u8(iter2);
			wl_info("peer_type = %d\n", peer_info->rtt_peer_type);
			break;
		case SPRD_RTT_ATTRIBUTE_TARGET_CHAN:
			memcpy(&peer_info->channel,nla_data(iter2),
				       sizeof(struct wifi_channel_info));
			wl_info("channel: width = %d, freq = %d, freq0 = %d, "
				"freq1 = %d\n", peer_info->channel.width,
				peer_info->channel.center_freq,
				peer_info->channel.center_freq0,
				peer_info->channel.center_freq1);
			break;
		case SPRD_RTT_ATTRIBUTE_TARGET_PERIOD:
			peer_info->burst_period = nla_get_u32(iter2);
			wl_info("burst_period = %d\n", peer_info->burst_period);
			break;
		case SPRD_RTT_ATTRIBUTE_TARGET_NUM_BURST:
			peer_info->num_burst_exponent = nla_get_u32(iter2);
			peer_info->num_burst = 1U << peer_info->num_burst_exponent;
			wl_info("num_burst_exponent = %d, num_burst = %d\n",
				peer_info->num_burst_exponent, peer_info->num_burst);
			break;
		case SPRD_RTT_ATTRIBUTE_TARGET_NUM_FTM_BURST:
			peer_info->num_frames_per_burst = nla_get_u32(iter2);
			wl_info("num_frames_per_burst = %d\n",
				peer_info->num_frames_per_burst);
			break;
		case SPRD_RTT_ATTRIBUTE_TARGET_NUM_RETRY_FTM:
			peer_info->num_retries_per_rtt_frame = nla_get_u32(iter2);
			wl_info("num_retries_per_rtt_frame = %d\n",
				peer_info->num_retries_per_rtt_frame);
			break;
		case SPRD_RTT_ATTRIBUTE_TARGET_NUM_RETRY_FTMR:
			peer_info->num_retries_per_ftmr = nla_get_u32(iter2);
			wl_info("num_retries_per_ftmr = %d\n",
				peer_info->num_retries_per_ftmr);
			break;
		case SPRD_RTT_ATTRIBUTE_TARGET_LCI:
			peer_info->LCI_request = nla_get_u8(iter2);
			wl_info("lci_request = %d\n", peer_info->LCI_request);
			break;
		case SPRD_RTT_ATTRIBUTE_TARGET_LCR:
			peer_info->LCR_request = nla_get_u8(iter2);
			wl_info("LCR_request = %d\n", peer_info->LCR_request);
			break;
		case SPRD_RTT_ATTRIBUTE_TARGET_BURST_DURATION:
			peer_info->burst_duration = nla_get_u32(iter2);
			wl_info("burst_duration = %d\n", peer_info->burst_duration);
			break;
		case SPRD_RTT_ATTRIBUTE_TARGET_PREAMBLE:
			peer_info->preamble = nla_get_u8(iter2);
			wl_info("preamble = %d\n", peer_info->preamble);
			break;
		case SPRD_RTT_ATTRIBUTE_TARGET_BW:
			peer_info->bw = nla_get_u8(iter2);
			wl_info("rtt_bw = %d\n", peer_info->bw);
			break;
		default:
			wl_err("peer_info type 0x%x not support\n", type);
			break;

		}
	}
	return 0;
}

int sc2355_rtt_start_session(struct wiphy *wiphy, struct wireless_dev *wdev,
			     const void *data, int data_len)
{
	struct sprd_priv *priv = wiphy_priv(wiphy);
	struct sprd_vif *vif = netdev_priv(wdev->netdev);
	struct rtt_session_request *request;
	const struct nlattr *iter, *iter1;
	int rem, rem1, ret, type, index = 0;
	struct sprd_msg *msg;
	struct cmd_rtt *cmd;
	u8 rsp[RTT_RSP_LEN] = {0x0};
	u16 rsp_len = RTT_RSP_LEN, cmd_data_len;
	struct rtt_meas_peer_info *peer_info;

	wl_info("rtt start\n");
	print_hex_dump(KERN_DEBUG, "conf:", DUMP_PREFIX_OFFSET, 16, 1, data,
		       data_len, true);
	mutex_lock(&priv->ftm.lock);
	if (priv->ftm.session_started) {
		wl_err("%s, rtt already started!\n", __func__);
		mutex_unlock(&priv->ftm.lock);
		return 0;
	}
	request = kzalloc(sizeof(*request) + RTT_MAX_CONFIG *
			  sizeof(struct rtt_meas_peer_info), GFP_KERNEL);
	if (!request) {
		wl_err("%s, request alloc failed!\n", __func__);
		mutex_unlock(&priv->ftm.lock);
		return -EAGAIN;
	}
	nla_for_each_attr(iter, data, data_len, rem) {
		type = nla_type(iter);
		switch(type) {
		case SPRD_RTT_ATTRIBUTE_TARGET_CNT:
			request->n_peers = nla_get_u8(iter);
			wl_info("%s, peer num:%d, type: %d, len: %d\n",
				__func__, nla_get_u8(iter),
				nla_type(iter), nla_len(iter));
			break;
		case SPRD_RTT_ATTRIBUTE_TARGET_INFO:
			nla_for_each_nested(iter1, iter, rem1) {
				peer_info = &request->peers[index];
				rtt_target_info_handler(iter1, peer_info);
				if(peer_info->burst_duration < 4) {
					ret = -EPERM;
					wl_err("cp2 not support duration\n");
					goto out;
				}
				index++;
			}
			break;
		default:
			wl_err("start_rtt nla type 0x%x not support\n", type);
			ret = -EINVAL;
			goto out;
		}
	}

	if(index != request->n_peers) {
		wl_err("index %d is out of number of peer\n", index);
		ret = -EINVAL;
		goto out;
	}

	cmd_data_len = 1 + index * sizeof(struct rtt_meas_peer_info);
	msg = get_cmdbuf(vif->priv, vif, rsp_len, CMD_RTT);
	if (!msg) {
		ret = -ENOMEM;
		goto out;
	}
	cmd = (struct cmd_rtt *)msg->data;
	cmd->sub_cmd = RTT_RANGE_REQUEST;
	cmd->len = cmd_data_len;
	memcpy(cmd->data, request, cmd_data_len);

	ret = send_cmd_recv_rsp(vif->priv, msg, rsp, &rsp_len);
	if (ret) {
		netdev_err(vif->ndev,
			   "%s: ret=%d, rsp_len=%d\n", __func__, ret, rsp_len);
	} else {
		priv->ftm.session_started = 1;
	}
out:
	kfree(request);
	request = NULL;
	mutex_unlock(&priv->ftm.lock);
	return ret;
}

int sc2355_rtt_abort_session(struct wiphy *wiphy, struct wireless_dev *wdev,
			     const void *data, int len)
{
	struct sprd_msg *msg;
	struct cmd_rtt *cmd;
	struct sprd_priv *priv = wiphy_priv(wiphy);
	struct sprd_vif *vif = netdev_priv(wdev->netdev);
	struct rtt_meas_cancel_peer_info *request;
	const struct nlattr *iter;
	int ret, rem, type, index = 0, cmd_data_len;

	wl_info("rtt stop\n");
	print_hex_dump(KERN_DEBUG, "conf:", DUMP_PREFIX_OFFSET, 16, 1, data, len, true);

	mutex_lock(&priv->ftm.lock);
	if (!priv->ftm.session_started) {
		netdev_err(vif->ndev,
			   "%s: FTM session not started\n", __func__);
		ret = -EAGAIN;
		goto out2;
	}

	/* bug 2028856, hackerone 1701183 */
	if (U16_MAX < (sizeof(struct cmd_rtt) + len) || len <= 0) {
		netdev_err(vif->ndev,
			   "%s: param data len is invalid\n", __func__);
		ret = -EINVAL;
		goto out2;
	}

	request = kzalloc(sizeof(*request) + RTT_MAX_CONFIG * ETH_ALEN,
					GFP_KERNEL);
	if (!request) {
		wl_err("%s, request alloc failed!\n", __func__);
		ret = -EAGAIN;
		goto out2;
	}
	nla_for_each_attr(iter, data, len, rem) {
		type = nla_type(iter);
		switch (type) {
		case SPRD_RTT_ATTRIBUTE_TARGET_CNT:
			request->n_peers = nla_get_u8(iter);
			wl_info("%s, peer num:%d, type: %d, len: %d\n", __func__,
				nla_get_u8(iter), nla_type(iter), nla_len(iter));
			break;
		case SPRD_RTT_ATTRIBUTE_TARGET_MAC:
			memcpy(request->mac_addr[index], nla_data(iter), ETH_ALEN);
			wl_info("mac: %pM\n", request->mac_addr[index]);
			index++;
			break;
		}
	}

	/* send cancel range request */
	cmd_data_len = 1 + index * ETH_ALEN;
	msg = get_cmdbuf(priv, vif, sizeof(struct cmd_rtt) + len, CMD_RTT);
	if (!msg) {
		ret = -ENOMEM;
		goto out1;
	}
	cmd = (struct cmd_rtt *)msg->data;
	cmd->sub_cmd = RTT_RANGE_CANCEL;
	cmd->len = cmd_data_len;
	memcpy(cmd->data, request, cmd_data_len);
	ret = send_cmd_recv_rsp(priv, msg, NULL, 0);
	if (ret)
		netdev_err(vif->ndev, "%s: ret=%d\n", __func__, ret);
	else
		priv->ftm.session_started = 0;
out1:
	kfree(request);
	request = NULL;
out2:
	mutex_unlock(&priv->ftm.lock);
	return ret;
}

int sc2355_rtt_get_responder_info(struct wiphy *wiphy,
				  struct wireless_dev *wdev,
				  const void *data, int len)
{
	struct sprd_vif *vif = netdev_priv(wdev->netdev);

	/* get responder info */
	netdev_info(vif->ndev, "%s: not implemented yet\n", __func__);
	return -ENOTSUPP;
}

int sc2355_rtt_configure_responder(struct wiphy *wiphy,
				   struct wireless_dev *wdev,
				   const void *data, int data_len)
{
	struct sprd_vif *vif = netdev_priv(wdev->netdev);

	/* enable or disable responder */
	netdev_info(vif->ndev, "%s: not implemented yet\n", __func__);
	return -ENOTSUPP;
}

void sc2355_rtt_init(struct sprd_priv *priv)
{
	int i;
	priv->ftm.session_started = 0;
	mutex_init(&priv->ftm.lock);
	for (i = 0; i < 10; i++)
		priv->rtt_results.peer_rtt_result[i] =
			kzalloc(2 * sizeof(struct rtt_wifi_hal_result), GFP_KERNEL);
}

void sc2355_rtt_deinit(struct sprd_priv *priv)
{
	int i;

	for (i = 0; i < 10; i++)
		kfree(priv->rtt_results.peer_rtt_result[i]);
	mutex_destroy(&priv->ftm.lock);
}
