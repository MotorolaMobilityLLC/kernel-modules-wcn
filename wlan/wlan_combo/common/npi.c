/*
* SPDX-FileCopyrightText: 2021-2023 Unisoc (Shanghai) Technologies Co. Ltd
* SPDX-License-Identifier: GPL-2.0-only
*/

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/platform_device.h>
#include <linux/types.h>
#include <linux/uaccess.h>
#include <linux/version.h>
#include "marlin_platform.h"
#include <net/genetlink.h>

#include "chip_ops.h"
#include "debug.h"
#include "iface.h"
#include "npi.h"
#include "delay_work.h"

struct sprd_wlan_adap_param adap_info;
struct set_5g_sar_info g_set_5g_sar_info;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(6, 4, 0))
static int npi_pre_doit(const struct genl_split_ops *ops,
			struct sk_buff *skb, struct genl_info *info)
#else
static int npi_pre_doit(const struct genl_ops *ops,
			struct sk_buff *skb, struct genl_info *info)
#endif
{
	struct net_device *ndev = NULL;
	struct sprd_vif *vif = NULL;
	struct sprd_priv *priv = NULL;
	int ifindex = 0;

	if (!info) {
		wl_err("%s NULL info!\n", __func__);
		return -EINVAL;
	}

	if (info->attrs[SPRD_NL_ATTR_IFINDEX]) {
		ifindex = nla_get_u32(info->attrs[SPRD_NL_ATTR_IFINDEX]);
		ndev = dev_get_by_index(genl_info_net(info), ifindex);

		if (!(ndev && (ndev->flags & IFF_UP))) {
			wl_err("%s NPI: net device is not ready yet\n", __func__);
			return -EFAULT;
		}

		vif = netdev_priv(ndev);
		priv = vif->priv;
		info->user_ptr[0] = ndev;
		info->user_ptr[1] = priv;
	} else {
		wl_err("nl80211_pre_doit: Not have attr_ifindex\n");
		return -EFAULT;
	}
	return 0;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(6, 4, 0))
static void npi_post_doit(const struct genl_split_ops *ops,
			  struct sk_buff *skb, struct genl_info *info)
#else
static void npi_post_doit(const struct genl_ops *ops,
			  struct sk_buff *skb, struct genl_info *info)
#endif
{
	if (info->user_ptr[0])
		dev_put(info->user_ptr[0]);
}

int sprd_npi_deal_setcca(struct genl_info *info,
			 unsigned char *s_buf, unsigned short s_len)
{
	unsigned char dbgstr[SPRD_NPI_DEBUG_STR_LEN] = {0};
	char *cca_param = NULL;
	int ret  = 0;
	struct net_device *ndev = NULL;
	struct sprd_vif *vif = NULL;
	struct sprd_priv *priv = NULL;
	struct sprd_npi_cmd_hdr *p_hdr = NULL, hdr = {0};
	unsigned short r_len = SPRD_NPI_RECV_BUF_LEN;
	unsigned char *r_buf = NULL;

	ndev = info->user_ptr[0];
	vif = netdev_priv(ndev);
	priv = info->user_ptr[1];

	if (s_len < (sizeof(struct sprd_npi_cmd_hdr) + 2 * sizeof(char))) {
		wl_err("%s, s_len is invalid, return", __func__);
		return -EINVAL;
	}

	r_buf = kzalloc(SPRD_NPI_RECV_BUF_LEN, GFP_KERNEL);
	if (!r_buf)
		return -ENOMEM;

	cca_param = s_buf + sizeof(struct sprd_npi_cmd_hdr);
	/*
	 * set_cca_param type value
	 * enable wifi adaptive: set_cca_param 3 1
	 * disable wifi adaptive: set_cca_param 3 0
	 */
	if (cca_param[0] == SPRD_NPI_CCA_CE) {
		spin_lock_bh(&adap_info.adap_lock);
		/*
		 * wifi_adaptive_flag
		 * BIT(4): iwnpi control
		 * BIT(0): adaptive value, enable/disable
		 */
		adap_info.wifi_adaptive_flag = BIT(4) | (cca_param[1] & BIT(0));

		/* when enable wifi adaptive, send data by normal channel */
		if (cca_param[1] == SPRD_NPI_CE_ENABLE)
			adap_info.special_data_flag = SPRD_NPI_NORMAL_ALL;
		else if (cca_param[1] == SPRD_NPI_CE_DISABLE)
			adap_info.special_data_flag = SPRD_NPI_DATA_SPECIAL;

		wl_info("%s wifi_adaptive_flag: 0x%x, special_data_flag: %d\n",
			__func__, adap_info.wifi_adaptive_flag,
			adap_info.special_data_flag);
		spin_unlock_bh(&adap_info.adap_lock);

		/* when connect to ap, send npi command directly */
		if (vif->sm_state == SPRD_CONNECTED) {
			if (cca_param[1] == SPRD_NPI_CE_DISABLE) {
				spin_lock_bh(&adap_info.adap_lock);
				adap_info.wifi_adaptive_flag = SPRD_NPI_CE_DISABLE;
				spin_unlock_bh(&adap_info.adap_lock);
			}

			sprd_npi_send_recv(priv, vif, s_buf, s_len, r_buf, &r_len);
			snprintf(dbgstr, sizeof(dbgstr), "[iwnpi][RECV][%d]:", r_len);
			p_hdr = (struct sprd_npi_cmd_hdr *)r_buf;
			wl_info("%s type is %d, subtype %d\n", dbgstr, p_hdr->type,
				p_hdr->subtype);
		} else {
			/* reply npi status, avoid err */
			hdr.len = sizeof(int);
			hdr.type = SPRD_CP2HT_REPLY;
			hdr.subtype = SPRD_NPI_CMD_SET_CCA_PARAM;
			r_len = sizeof(hdr) + hdr.len;
			memcpy(r_buf, &hdr, sizeof(hdr));
			memcpy(r_buf + sizeof(hdr), &ret, hdr.len);
		}
	} else {
		sprd_npi_send_recv(priv, vif, s_buf, s_len, r_buf, &r_len);
		snprintf(dbgstr, sizeof(dbgstr), "[iwnpi][RECV][%d]:", r_len);
		p_hdr = (struct sprd_npi_cmd_hdr *)r_buf;
		wl_info("%s type is %d, subtype %d\n", dbgstr, p_hdr->type,
			p_hdr->subtype);
	}

	ret = npi_nl_send_generic(info, SPRD_NL_ATTR_CP2AP,
				  SPRD_NL_CMD_NPI, r_len, r_buf);
	kfree(r_buf);
	return ret;
}

int sprd_npi_get_chipid(struct genl_info *info,
			unsigned char *s_buf, unsigned short s_len)
{
	int ret = 0;
	const char *id_name = NULL;
	const char *vendor = "UniSoC,";
	unsigned char status = 0;
	unsigned short r_len = SPRD_NPI_RECV_BUF_LEN;
	unsigned char *r_buf = NULL;

	r_buf = kzalloc(SPRD_NPI_RECV_BUF_LEN, GFP_KERNEL);
	if (!r_buf)
		return -ENOMEM;

	id_name = (char *)wcn_get_chip_name();

	snprintf(r_buf, SPRD_NPI_RECV_BUF_LEN, "%d", status);
	strncat(r_buf, vendor, strlen(vendor));
	strncat(r_buf, id_name, strlen(id_name));
	r_len = strlen(r_buf);
	wl_info("%s show chip name: %s\n", __func__, r_buf);

	ret = npi_nl_send_generic(info, SPRD_NL_ATTR_CP2AP,
				  SPRD_NL_CMD_NPI, r_len, r_buf);
	kfree(r_buf);
	return ret;
}

int sprd_npi_set_random_mac(struct genl_info *info,
			    unsigned char *s_buf, unsigned short s_len)
{
	int ret = 0;
	char *rand_mac = NULL;
	struct net_device *ndev = NULL;
	struct sprd_vif *vif = NULL;
	struct sprd_priv *priv = NULL;
	struct sprd_npi_cmd_hdr hdr = {0};
	unsigned short r_len = SPRD_NPI_RECV_BUF_LEN;
	unsigned char *r_buf = NULL;

	ndev = info->user_ptr[0];
	vif = netdev_priv(ndev);
	priv = info->user_ptr[1];
	if (s_len < (sizeof(struct sprd_npi_cmd_hdr) + sizeof(char))) {
		wl_err("%s, s_len is invalid, return", __func__);
		return -EINVAL;
	}

	r_buf = kzalloc(SPRD_NPI_RECV_BUF_LEN, GFP_KERNEL);
	if (!r_buf)
		return -ENOMEM;

	rand_mac = s_buf + sizeof(struct sprd_npi_cmd_hdr);
	priv->rand_mac_flag = *((unsigned int *)rand_mac);
	wl_info("%s NPI random mac flag %d\n", __func__, priv->rand_mac_flag);

	hdr.len = sizeof(int);
	hdr.type = SPRD_CP2HT_REPLY;
	hdr.subtype = SPRD_NPI_CMD_SET_RANDOM_MAC;
	r_len = sizeof(hdr) + hdr.len;
	memcpy(r_buf, &hdr, sizeof(hdr));
	memcpy(r_buf + sizeof(hdr), &ret, hdr.len);

	ret = npi_nl_send_generic(info, SPRD_NL_ATTR_CP2AP,
				  SPRD_NL_CMD_NPI, r_len, r_buf);
	kfree(r_buf);
	return ret;
}

int sprd_npi_set_country(struct genl_info *info,
			 unsigned char *s_buf, unsigned short s_len)
{
	int ret = 0;
	char *country = NULL;
	struct net_device *ndev = NULL;
	struct sprd_vif *vif = NULL;
	struct sprd_priv *priv = NULL;
	struct sprd_npi_cmd_hdr hdr = {0};
	unsigned short r_len = SPRD_NPI_RECV_BUF_LEN;
	unsigned char *r_buf = NULL;

	ndev = info->user_ptr[0];
	vif = netdev_priv(ndev);
	priv = info->user_ptr[1];

	if (s_len < (sizeof(struct sprd_npi_cmd_hdr) + 2 * sizeof(char))) {
		wl_err("%s, s_len is invalid, return", __func__);
		return -EINVAL;
	}

	r_buf = kzalloc(SPRD_NPI_RECV_BUF_LEN, GFP_KERNEL);
	if (!r_buf)
		return -ENOMEM;

	country = s_buf + sizeof(struct sprd_npi_cmd_hdr);
	wl_info("%s show country code : %c%c\n", __func__, country[0], country[1]);

	ret = regulatory_hint(priv->wiphy, country);
	hdr.len = sizeof(int);
	hdr.type = SPRD_CP2HT_REPLY;
	hdr.subtype = SPRD_NPI_CMD_SET_COUNTRY;
	r_len = sizeof(hdr) + hdr.len;
	memcpy(r_buf, &hdr, sizeof(hdr));
	memcpy(r_buf + sizeof(hdr), &ret, hdr.len);

	ret = npi_nl_send_generic(info, SPRD_NL_ATTR_CP2AP,
				  SPRD_NL_CMD_NPI, r_len, r_buf);
	kfree(r_buf);
	return ret;
}

int sprd_npi_5gpw_backoff(struct genl_info *info,
			  unsigned char *s_buf, unsigned short s_len)
{
	int ret = 0;
	struct net_device *ndev = NULL;
	struct sprd_vif *vif = NULL;
	struct sprd_priv *priv = NULL;
	struct sprd_npi_cmd_hdr hdr = {0};
	unsigned short r_len = SPRD_NPI_RECV_BUF_LEN;
	unsigned char *r_buf = NULL, *value = NULL;
	u8 sar_value;

	ndev = info->user_ptr[0];
	vif = netdev_priv(ndev);
	priv = info->user_ptr[1];

	if (s_len < (sizeof(struct sprd_npi_cmd_hdr) + sizeof(char))) {
		wl_err("%s, s_len is invalid, return", __func__);
		return -EINVAL;
	}

	r_buf = kzalloc(SPRD_NPI_RECV_BUF_LEN, GFP_KERNEL);
	if (!r_buf)
		return -ENOMEM;

	value = s_buf + sizeof(struct sprd_npi_cmd_hdr);
	sprd_5g_sar_info_set(priv, value);
	sar_value = sprd_pw_backoff_band2value(priv, g_set_5g_sar_info.channel);
	if (sar_value) {
		ret = sprd_set_sar(vif->priv, vif, SPRD_SET_SAR_RELATIVE, sar_value);
		if (ret)
			goto out;
	}

	hdr.len = sizeof(int);
	hdr.type = SPRD_CP2HT_REPLY;
	hdr.subtype = SPRD_NPI_CMD_5GPW_BACKOFF;
	r_len = sizeof(hdr) + hdr.len;
	memcpy(r_buf, &hdr, sizeof(hdr));
	memcpy(r_buf + sizeof(hdr), &ret, hdr.len);

	ret = npi_nl_send_generic(info, SPRD_NL_ATTR_CP2AP,
				  SPRD_NL_CMD_NPI, r_len, r_buf);

out:
	kfree(r_buf);
	return ret;
}

/* match npi command processed by driver */
static const struct sprd_npi_ops *npi_match_cmd(struct sprd_priv *priv,
						unsigned char cmd)
{
	const struct sprd_npi_ops *npi_cmd = NULL;
	u32 i;

	for (i = 0; i < priv->chip.ops->n_npi_ops; i++) {
		if (priv->chip.ops->npi_ops[i].cmd != cmd)
			continue;

		npi_cmd = &priv->chip.ops->npi_ops[i];
		break;
	}

	return npi_cmd;
}

static int npi_match_cmd_doit(struct genl_info *info,
			      const struct sprd_npi_ops *npi_cmd,
			      unsigned char *s_buf, unsigned short s_len)
{
	struct sprd_npi_cmd_hdr hdr = {0};
	unsigned short r_len = SPRD_NPI_RECV_BUF_LEN;
	unsigned char *r_buf = NULL;
	int ret = 0;

	if (!npi_cmd->doit) {
		wl_err("%s: doit is NULL\n", __func__);
		ret = -EOPNOTSUPP;
		goto out;
	}
	ret = npi_cmd->doit(info, s_buf, s_len);
	if (ret != 0) {
		wl_info("%s: doit error %d\n", __func__, ret);
		goto out;
	}

	/* npi_cmd->doit successfully */
	return 0;

out:
	r_buf = kzalloc(SPRD_NPI_RECV_BUF_LEN, GFP_KERNEL);
	if (!r_buf)
		return -ENOMEM;

	hdr.len = sizeof(int);
	hdr.type = SPRD_CP2HT_REPLY;
	hdr.subtype = npi_cmd->cmd;
	r_len = sizeof(struct sprd_npi_cmd_hdr) + hdr.len;
	memcpy(r_buf, &hdr, sizeof(struct sprd_npi_cmd_hdr));
	memcpy(r_buf + sizeof(struct sprd_npi_cmd_hdr), &ret, hdr.len);

	ret = npi_nl_send_generic(info, SPRD_NL_ATTR_CP2AP,
				  SPRD_NL_CMD_NPI, r_len, r_buf);
	kfree(r_buf);
	return ret;
}

static int sprd_npi_cp2_proc(struct genl_info *info,
			     unsigned char *s_buf, unsigned short s_len)
{
	int ret = 0;
	struct net_device *ndev = NULL;
	struct sprd_vif *vif = NULL;
	struct sprd_priv *priv = NULL;
	struct sprd_npi_cmd_hdr *hdr = NULL;
	unsigned short r_len = SPRD_NPI_RECV_BUF_LEN;
	unsigned char *r_buf = NULL;
	unsigned char dbgstr[SPRD_NPI_DEBUG_STR_LEN] = {0};

	ndev = info->user_ptr[0];
	vif = netdev_priv(ndev);
	priv = info->user_ptr[1];

	r_buf = kzalloc(SPRD_NPI_RECV_BUF_LEN, GFP_KERNEL);
	if (!r_buf)
		return -ENOMEM;

	sprd_npi_send_recv(priv, vif, s_buf, s_len, r_buf, &r_len);

	snprintf(dbgstr, sizeof(dbgstr), "[iwnpi][RECV][%d]:", r_len);
	hdr = (struct sprd_npi_cmd_hdr *)r_buf;
	wl_info("%s type is %d, subtype %d\n", dbgstr, hdr->type,
		hdr->subtype);

	ret = npi_nl_send_generic(info, SPRD_NL_ATTR_CP2AP,
				  SPRD_NL_CMD_NPI, r_len, r_buf);
	kfree(r_buf);
	return ret;
}

static int npi_nl_handler(struct sk_buff *skb_2, struct genl_info *info)
{
	struct net_device *ndev = NULL;
	struct sprd_vif *vif = NULL;
	struct sprd_priv *priv = NULL;
	struct sprd_npi_cmd_hdr *hdr = NULL;
	unsigned short s_len = 0;
	unsigned char *s_buf = NULL;
	unsigned char dbgstr[SPRD_NPI_DEBUG_STR_LEN] = {0};
	const struct sprd_npi_ops *npi_cmd = NULL;
	int ret = 0;

	ndev = info->user_ptr[0];
	vif = netdev_priv(ndev);
	priv = info->user_ptr[1];
	if (!info->attrs[SPRD_NL_ATTR_AP2CP]) {
		wl_err("%s: invalid content\n", __func__);
		return -EPERM;
	}

	s_buf = nla_data(info->attrs[SPRD_NL_ATTR_AP2CP]);
	s_len = nla_len(info->attrs[SPRD_NL_ATTR_AP2CP]);
	if (s_len < sizeof(struct sprd_npi_cmd_hdr)) {
		wl_err("%s: invalid hdr\n", __func__);
		return -EPERM;
	}

	snprintf(dbgstr, sizeof(dbgstr), "[iwnpi][SEND][%d]:", s_len);
	hdr = (struct sprd_npi_cmd_hdr *)s_buf;
	wl_info("%s type is %d, subtype %d, mode %d\n", dbgstr,
		hdr->type, hdr->subtype, vif->mode);

	npi_cmd = npi_match_cmd(priv, hdr->subtype);

	if (npi_cmd)
		ret = npi_match_cmd_doit(info, npi_cmd, s_buf, s_len);
	else
		ret = sprd_npi_cp2_proc(info, s_buf, s_len);

	wl_debug("%s ret %d.\n", __func__, ret);
	return ret;
}

static int npi_nl_get_info_handler(struct sk_buff *skb_2,
				   struct genl_info *info)
{
	struct net_device *ndev = info->user_ptr[0];
	struct sprd_vif *vif = netdev_priv(ndev);
	unsigned char r_buf[64] = { 0 };
	unsigned short r_len = 0;
	int ret = 0;

	if (vif) {
		ether_addr_copy(r_buf, vif->ndev->dev_addr);
		r_len = ETH_ALEN;
		ret = npi_nl_send_generic(info, SPRD_NL_ATTR_CP2AP,
				      SPRD_NL_CMD_GET_INFO, r_len, r_buf);
	} else {
		wl_err("%s NULL vif!\n", __func__);
		ret = -1;
	}
	return ret;
}

static struct nla_policy sprd_genl_policy[SPRD_NL_ATTR_MAX + 1] = {
	[SPRD_NL_ATTR_IFINDEX] = {.type = NLA_U32},
	[SPRD_NL_ATTR_AP2CP] = {.type = NLA_BINARY, .len = 1024},
	[SPRD_NL_ATTR_CP2AP] = {.type = NLA_BINARY, .len = 1024}
};

static struct genl_ops sprd_nl_ops[] = {
	{
		.cmd = SPRD_NL_CMD_NPI,
		.doit = npi_nl_handler,
	},
	{
		.cmd = SPRD_NL_CMD_GET_INFO,
		.doit = npi_nl_get_info_handler,
	}
};

static struct genl_family sprd_nl_genl_family = {
	.hdrsize = 0,
	.name = "SPRD_NL",
	.version = 1,
	.maxattr = SPRD_NL_ATTR_MAX,
	.policy = sprd_genl_policy,
	.pre_doit = npi_pre_doit,
	.post_doit = npi_post_doit,
	.module = THIS_MODULE,
	.ops = sprd_nl_ops,
	.n_ops = ARRAY_SIZE(sprd_nl_ops),
};

int npi_nl_send_generic(struct genl_info *info, u8 attr, u8 cmd,
			u32 len, u8 *data)
{
	struct sk_buff *skb = NULL;
	void *hdr = NULL;
	int ret = -1;

	skb = nlmsg_new(NLMSG_GOODSIZE, GFP_KERNEL);
	if (!skb)
		return -ENOMEM;

	hdr = genlmsg_put(skb, info->snd_portid, info->snd_seq,
			  &sprd_nl_genl_family, 0, cmd);
	if (IS_ERR(hdr)) {
		ret = PTR_ERR(hdr);
		goto err_put;
	}

	if (nla_put(skb, attr, len, data)) {
		ret = -1;
		goto err_put;
	}

	genlmsg_end(skb, hdr);
	return genlmsg_reply(skb, info);

err_put:
	nlmsg_free(skb);
	return ret;
}

void sprd_init_npi(void)
{
	int ret = genl_register_family(&sprd_nl_genl_family);

	if (ret)
		wl_err("genl_register_family error: %d\n", ret);
}

void sprd_deinit_npi(void)
{
	int ret = genl_unregister_family(&sprd_nl_genl_family);

	if (ret)
		wl_err("genl_unregister_family error:%d\n", ret);
}

/* flag
 * SPRD_NPI_CMD_SET_FLAG_ADAP: set adaptive function
 * SPRD_NPI_CMD_SET_FLAG_ACS: set acs enable
 */
void sprd_npi_set_cca_param(struct sprd_priv *priv, struct sprd_vif *vif,
			    enum sprd_npi_cmd_set_cca_flag flag)
{
	struct sprd_npi_cmd_hdr *hdr = NULL;
	unsigned short s_len = 0, r_len = SPRD_NPI_RECV_BUF_LEN;
	unsigned char s_buf[SPRD_NPI_SEND_BUF_LEN] = { 0 }, *r_buf = NULL;
	unsigned char dbgstr[SPRD_NPI_DEBUG_STR_LEN] = { 0 };
	char *p = NULL;
	char tmp_flag = 0;

	r_buf = kzalloc(SPRD_NPI_RECV_BUF_LEN, GFP_KERNEL);
	if (!r_buf)
		return;

	p = s_buf + sizeof(struct sprd_npi_cmd_hdr);

	if (flag == SPRD_NPI_CMD_SET_FLAG_ADAP) {
		spin_lock_bh(&adap_info.adap_lock);
		tmp_flag = adap_info.wifi_adaptive_flag;

		/* wifi_adaptive_flag
		 * BIT(4): iwnpi control
		 * BIT(0): adaptive value, enable/disable
		 */
		if (tmp_flag & BIT(4)) {
			if (!(tmp_flag & BIT(0))) {
				adap_info.special_data_flag = SPRD_NPI_DATA_SPECIAL;
				adap_info.wifi_adaptive_flag = SPRD_NPI_CE_DISABLE;
			} else {
				adap_info.special_data_flag = SPRD_NPI_NORMAL_ALL;
			}
		}
		wl_debug("%s wifi_adaptive_flag: 0x%x, special_data_flag: %d\n",
			 __func__, adap_info.wifi_adaptive_flag,
			 adap_info.special_data_flag);
		spin_unlock_bh(&adap_info.adap_lock);
	} else if (flag == SPRD_NPI_CMD_SET_FLAG_ACS) {
		spin_lock_bh(&adap_info.adap_lock);
		tmp_flag = adap_info.wifi_adaptive_flag;
		spin_unlock_bh(&adap_info.adap_lock);
	} else {
		wl_err("%s err flag %d\n", __func__, flag);
		goto err;
	}

	if (tmp_flag & BIT(4)) {
		hdr = (struct sprd_npi_cmd_hdr *)s_buf;
		hdr->type = SPRD_HT2CP_CMD;
		hdr->subtype = SPRD_NPI_CMD_SET_CCA_PARAM;
		hdr->len = 2 * sizeof(char);
		s_len = sizeof(struct sprd_npi_cmd_hdr) + hdr->len;

		*p++ = SPRD_NPI_CCA_CE;
		if (flag == SPRD_NPI_CMD_SET_FLAG_ADAP)
			*p = tmp_flag & BIT(0);
		else if ((flag == SPRD_NPI_CMD_SET_FLAG_ACS) && (tmp_flag & BIT(0)))
			*p = SPRD_NPI_ACS_ENABLE;
		else
			goto err;

		snprintf(dbgstr, sizeof(dbgstr), "[iwnpi][SEND][%d]:", s_len);
		wl_debug("%s type is %d, subtype %d\n", dbgstr, hdr->type, hdr->subtype);
		sprd_npi_send_recv(priv, vif, s_buf, s_len, r_buf, &r_len);
		snprintf(dbgstr, sizeof(dbgstr), "[iwnpi][RECV][%d]:", r_len);
		hdr = (struct sprd_npi_cmd_hdr *)r_buf;
		wl_debug("%s type is %d, subtype %d\n", dbgstr, hdr->type, hdr->subtype);
	}

err:
	kfree(r_buf);
	r_buf = NULL;
	return;
}

void sprd_evt_adaptive(struct sprd_vif *vif)
{
	struct sprd_work *misc_work;

	misc_work = sprd_alloc_work(0);
	if (!misc_work) {
		wl_err("%s out of memory\n", __func__);
		return;
	}

	misc_work->vif = vif;
	misc_work->id = SPRD_WORK_ADAPTIVE;

	sprd_queue_work(vif->priv, misc_work);
}

void sprd_wifi_adaptive_work(struct sprd_priv *priv, struct sprd_vif *vif)
{
	sprd_npi_set_cca_param(priv, vif, SPRD_NPI_CMD_SET_FLAG_ADAP);
}
