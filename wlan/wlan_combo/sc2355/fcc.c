/*
* SPDX-FileCopyrightText: 2020-2023 Unisoc (Shanghai) Technologies Co. Ltd
* SPDX-License-Identifier: GPL-2.0-only
*/

#include "common/common.h"
#include "common/npi.h"
#include "cmdevt.h"

/* subtype, channel, bw, {mode(2.4g : b,g,n,ac; 5g : a,n,ac), value} */

#define num_ce 8

#define power_backoff_ce {{1, 149, 0, { {0, 127}, {1, 127}, {2, 127}, {3, 127}, {4, 12}, {5, 12}, {6, 12} } }, {1, 153, 0, { {0, 127}, {1, 127}, {2, 127}, {3, 127}, {4, 12}, {5, 12}, {6, 12} } }, {1, 157, 0, { {0, 127}, {1, 127}, {2, 127}, {3, 127}, {4, 12}, {5, 12}, {6, 12} } }, {1, 161, 0, { {0, 127}, {1, 127}, {2, 127}, {3, 127}, {4, 12}, {5, 12}, {6, 12} } }, {1, 165, 0, { {0, 127}, {1, 127}, {2, 127}, {3, 127}, {4, 12}, {5, 12}, {6, 12} } }, {1, 151, 1, { {0, 127}, {1, 127}, {2, 127}, {3, 127}, {4, 12}, {5, 12}, {6, 12} } }, {1, 159, 1, { {0, 127}, {1, 127}, {2, 127}, {3, 127}, {4, 12}, {5, 12}, {6, 12} } }, {1, 155, 2, { {0, 127}, {1, 127}, {2, 127}, {3, 127}, {4, 12}, {5, 12}, {6, 12} } }, }

static struct sprd_fcc_priv fcc_info;

static struct fcc_power_bo g_fcc_power_table[MAX_FCC_COUNTRY_NUM] = {
	{
		.country = "AE",
		.num = num_ce,
		.power_backoff = power_backoff_ce,
	},
	{
		.country = "AL",
		.num = num_ce,
		.power_backoff = power_backoff_ce,
	},
	{
		.country = "AM",
		.num = num_ce,
		.power_backoff = power_backoff_ce,
	},
	{
		.country = "AN",
		.num = num_ce,
		.power_backoff = power_backoff_ce,
	},
	{
		.country = "AT",
		.num = num_ce,
		.power_backoff = power_backoff_ce,
	},
	{
		.country = "AW",
		.num = num_ce,
		.power_backoff = power_backoff_ce,
	},
	{
		.country = "AZ",
		.num = num_ce,
		.power_backoff = power_backoff_ce,
	},
	{
		.country = "BA",
		.num = num_ce,
		.power_backoff = power_backoff_ce,
	},
	{
		.country = "BD",
		.num = num_ce,
		.power_backoff = power_backoff_ce,
	},
	{
		.country = "BE",
		.num = num_ce,
		.power_backoff = power_backoff_ce,
	},
	{
		.country = "BG",
		.num = num_ce,
		.power_backoff = power_backoff_ce,
	},
	{
		.country = "BH",
		.num = num_ce,
		.power_backoff = power_backoff_ce,
	},
	{
		.country = "BL",
		.num = num_ce,
		.power_backoff = power_backoff_ce,
	},
	{
		.country = "BN",
		.num = num_ce,
		.power_backoff = power_backoff_ce,
	},
	{
		.country = "BO",
		.num = num_ce,
		.power_backoff = power_backoff_ce,
	},
	{
		.country = "BR",
		.num = num_ce,
		.power_backoff = power_backoff_ce,
	},
	{
		.country = "BY",
		.num = num_ce,
		.power_backoff = power_backoff_ce,
	},
	{
		.country = "BZ",
		.num = num_ce,
		.power_backoff = power_backoff_ce,
	},
	{
		.country = "CH",
		.num = num_ce,
		.power_backoff = power_backoff_ce,
	},
	{
		.country = "CL",
		.num = num_ce,
		.power_backoff = power_backoff_ce,
	},
	{
		.country = "CR",
		.num = num_ce,
		.power_backoff = power_backoff_ce,
	},
	{
		.country = "CY",
		.num = num_ce,
		.power_backoff = power_backoff_ce,
	},
	{
		.country = "CZ",
		.num = num_ce,
		.power_backoff = power_backoff_ce,
	},
	{
		.country = "DE",
		.num = num_ce,
		.power_backoff = power_backoff_ce,
	},
	{
		.country = "DK",
		.num = num_ce,
		.power_backoff = power_backoff_ce,
	},
	{
		.country = "DZ",
		.num = num_ce,
		.power_backoff = power_backoff_ce,
	},
	{
		.country = "EC",
		.num = num_ce,
		.power_backoff = power_backoff_ce,
	},
	{
		.country = "EE",
		.num = num_ce,
		.power_backoff = power_backoff_ce,
	},
	{
		.country = "EG",
		.num = num_ce,
		.power_backoff = power_backoff_ce,
	},
	{
		.country = "ES",
		.num = num_ce,
		.power_backoff = power_backoff_ce,
	},
	{
		.country = "FI",
		.num = num_ce,
		.power_backoff = power_backoff_ce,
	},
	{
		.country = "FR",
		.num = num_ce,
		.power_backoff = power_backoff_ce,
	},
	{
		.country = "GB",
		.num = num_ce,
		.power_backoff = power_backoff_ce,
	},
	{
		.country = "GE",
		.num = num_ce,
		.power_backoff = power_backoff_ce,
	},
	{
		.country = "GF",
		.num = num_ce,
		.power_backoff = power_backoff_ce,
	},
	{
		.country = "GL",
		.num = num_ce,
		.power_backoff = power_backoff_ce,
	},
	{
		.country = "GP",
		.num = num_ce,
		.power_backoff = power_backoff_ce,
	},
	{
		.country = "GR",
		.num = num_ce,
		.power_backoff = power_backoff_ce,
	},
	{
		.country = "HU",
		.num = num_ce,
		.power_backoff = power_backoff_ce,
	},
	{
		.country = "IE",
		.num = num_ce,
		.power_backoff = power_backoff_ce,
	},
	{
		.country = "IL",
		.num = num_ce,
		.power_backoff = power_backoff_ce,
	},
	{
		.country = "IN",
		.num = num_ce,
		.power_backoff = power_backoff_ce,
	},
	{
		.country = "IR",
		.num = num_ce,
		.power_backoff = power_backoff_ce,
	},
	{
		.country = "IS",
		.num = num_ce,
		.power_backoff = power_backoff_ce,
	},
	{
		.country = "IT",
		.num = num_ce,
		.power_backoff = power_backoff_ce,
	},
	{
		.country = "JO",
		.num = num_ce,
		.power_backoff = power_backoff_ce,
	},
	{
		.country = "KE",
		.num = num_ce,
		.power_backoff = power_backoff_ce,
	},
	{
		.country = "KH",
		.num = num_ce,
		.power_backoff = power_backoff_ce,
	},
	{
		.country = "KP",
		.num = num_ce,
		.power_backoff = power_backoff_ce,
	},
	{
		.country = "KR",
		.num = num_ce,
		.power_backoff = power_backoff_ce,
	},
	{
		.country = "KW",
		.num = num_ce,
		.power_backoff = power_backoff_ce,
	},
	{
		.country = "KZ",
		.num = num_ce,
		.power_backoff = power_backoff_ce,
	},
	{
		.country = "LB",
		.num = num_ce,
		.power_backoff = power_backoff_ce,
	},
	{
		.country = "LI",
		.num = num_ce,
		.power_backoff = power_backoff_ce,
	},
	{
		.country = "LK",
		.num = num_ce,
		.power_backoff = power_backoff_ce,
	},
	{
		.country = "LT",
		.num = num_ce,
		.power_backoff = power_backoff_ce,
	},
	{
		.country = "LU",
		.num = num_ce,
		.power_backoff = power_backoff_ce,
	},
	{
		.country = "LV",
		.num = num_ce,
		.power_backoff = power_backoff_ce,
	},
	{
		.country = "MA",
		.num = num_ce,
		.power_backoff = power_backoff_ce,
	},
	{
		.country = "MC",
		.num = num_ce,
		.power_backoff = power_backoff_ce,
	},
	{
		.country = "MK",
		.num = num_ce,
		.power_backoff = power_backoff_ce,
	},
	{
		.country = "MQ",
		.num = num_ce,
		.power_backoff = power_backoff_ce,
	},
	{
		.country = "MU",
		.num = num_ce,
		.power_backoff = power_backoff_ce,
	},
	{
		.country = "MW",
		.num = num_ce,
		.power_backoff = power_backoff_ce,
	},
	{
		.country = "MY",
		.num = num_ce,
		.power_backoff = power_backoff_ce,
	},
	{
		.country = "NG",
		.num = num_ce,
		.power_backoff = power_backoff_ce,
	},
	{
		.country = "NL",
		.num = num_ce,
		.power_backoff = power_backoff_ce,
	},
	{
		.country = "NO",
		.num = num_ce,
		.power_backoff = power_backoff_ce,
	},
	{
		.country = "NP",
		.num = num_ce,
		.power_backoff = power_backoff_ce,
	},
	{
		.country = "PE",
		.num = num_ce,
		.power_backoff = power_backoff_ce,
	},
	{
		.country = "PF",
		.num = num_ce,
		.power_backoff = power_backoff_ce,
	},
	{
		.country = "PG",
		.num = num_ce,
		.power_backoff = power_backoff_ce,
	},
	{
		.country = "PK",
		.num = num_ce,
		.power_backoff = power_backoff_ce,
	},
	{
		.country = "PL",
		.num = num_ce,
		.power_backoff = power_backoff_ce,
	},
	{
		.country = "PT",
		.num = num_ce,
		.power_backoff = power_backoff_ce,
	},
	{
		.country = "QA",
		.num = num_ce,
		.power_backoff = power_backoff_ce,
	},
	{
		.country = "RE",
		.num = num_ce,
		.power_backoff = power_backoff_ce,
	},
	{
		.country = "RO",
		.num = num_ce,
		.power_backoff = power_backoff_ce,
	},
	{
		.country = "RS",
		.num = num_ce,
		.power_backoff = power_backoff_ce,
	},
	{
		.country = "RU",
		.num = num_ce,
		.power_backoff = power_backoff_ce,
	},
	{
		.country = "SA",
		.num = num_ce,
		.power_backoff = power_backoff_ce,
	},
	{
		.country = "SE",
		.num = num_ce,
		.power_backoff = power_backoff_ce,
	},
	{
		.country = "SG",
		.num = num_ce,
		.power_backoff = power_backoff_ce,
	},
	{
		.country = "SI",
		.num = num_ce,
		.power_backoff = power_backoff_ce,
	},
	{
		.country = "SK",
		.num = num_ce,
		.power_backoff = power_backoff_ce,
	},
	{
		.country = "SV",
		.num = num_ce,
		.power_backoff = power_backoff_ce,
	},
	{
		.country = "SY",
		.num = num_ce,
		.power_backoff = power_backoff_ce,
	},
	{
		.country = "TH",
		.num = num_ce,
		.power_backoff = power_backoff_ce,
	},
	{
		.country = "TN",
		.num = num_ce,
		.power_backoff = power_backoff_ce,
	},
	{
		.country = "TR",
		.num = num_ce,
		.power_backoff = power_backoff_ce,
	},
	{
		.country = "TT",
		.num = num_ce,
		.power_backoff = power_backoff_ce,
	},
	{
		.country = "UA",
		.num = num_ce,
		.power_backoff = power_backoff_ce,
	},
	{
		.country = "UG",
		.num = num_ce,
		.power_backoff = power_backoff_ce,
	},
	{
		.country = "UY",
		.num = num_ce,
		.power_backoff = power_backoff_ce,
	},
	{
		.country = "VE",
		.num = num_ce,
		.power_backoff = power_backoff_ce,
	},
	{
		.country = "VN",
		.num = num_ce,
		.power_backoff = power_backoff_ce,
	},
	{
		.country = "YE",
		.num = num_ce,
		.power_backoff = power_backoff_ce,
	},
	{
		.country = "YT",
		.num = num_ce,
		.power_backoff = power_backoff_ce,
	},
	{
		.country = "ZA",
		.num = num_ce,
		.power_backoff = power_backoff_ce,
	},
	{
		.country = "ZW",
		.num = num_ce,
		.power_backoff = power_backoff_ce,
	},
};

static int sc2355_fcc_fresh_bo(struct sprd_priv *priv, u8 channel, u8 bw, bool flag)
{
	struct sprd_power_backoff *p_backoff;
	struct fcc_power_bo *current_power_bo;
	int index;

	mutex_lock(&fcc_info.lock);
	fcc_info.flag = flag;
	if (flag) {
		fcc_info.channel = channel;
		fcc_info.bw = bw;
	}
	current_power_bo = fcc_info.cur_power_bo;
	mutex_unlock(&fcc_info.lock);

	if (!current_power_bo) {
		wl_debug("current_power_bo is NULL, reset default!\n");
		p_backoff = NULL;
	} else {
		for (index = 0; index < current_power_bo->num; index++) {
			p_backoff = &current_power_bo->power_backoff[index];
			if (channel == p_backoff->channel &&
			    bw == p_backoff->bw) {
				wl_info("match channel : %hhu bw : %hhu\n",
					channel, bw);
				break;
			}
		}

		if (index == current_power_bo->num) {
			wl_info("do not match channel %hhu bw %hhu, reset default\n",
				channel, bw);
			p_backoff = NULL;
		}
	}

	atomic_set(&priv->power_back_off, 1);
	sc2355_set_power_backoff(priv, NULL, p_backoff);
	atomic_set(&priv->power_back_off, 0);
	return 0;
}

void sc2355_fcc_fresh_bo_work(struct sprd_priv *priv, void *data, u16 len)
{
	struct fresh_bo_info *info = (struct fresh_bo_info *)data;
	sc2355_fcc_fresh_bo(priv, info->pw_channel, info->pw_bw, true);
}

void sc2355_fcc_match_country(struct sprd_priv *priv, const char *alpha2)
{
	bool found_country = false;
	bool need_refresh = false;
	struct fcc_power_bo *last_power_bo;
	int i, channel = 0, bw = 0;

	mutex_lock(&fcc_info.lock);
	for (i = 0; i < MAX_FCC_COUNTRY_NUM; i++) {
		if (g_fcc_power_table[i].country[0] == alpha2[0] &&
			g_fcc_power_table[i].country[1] == alpha2[1]) {
			wl_debug("matched fcc country %s!\n", alpha2);
			found_country = true;
			last_power_bo = fcc_info.cur_power_bo;
			fcc_info.cur_power_bo = &g_fcc_power_table[i];
			/* handle alpha2 change after connected */
			if (last_power_bo && last_power_bo != fcc_info.cur_power_bo)
				fcc_info.flag = true;
			/* handle set regdom just after connected */
			if (fcc_info.flag) {
				need_refresh = true;
				channel = fcc_info.channel;
				bw = fcc_info.bw;
				wl_debug("evt_fresh_backoff had came, now fresh it!\n");
			}
			break;
		}
	}

	if (!found_country) {
		wl_debug("not fcc country, need reset fcc power\n");
		fcc_info.cur_power_bo = NULL;
	}
	mutex_unlock(&fcc_info.lock);

	if (!found_country || need_refresh) {
		sc2355_fcc_fresh_bo(priv, channel, bw, false);
	}
}

void sc2355_fcc_reset_bo(void)
{
	mutex_lock(&fcc_info.lock);
	fcc_info.flag = false;
	fcc_info.channel = 0;
	fcc_info.bw = 0;
	mutex_unlock(&fcc_info.lock);
}

void sc2355_fcc_init(void)
{
	fcc_info.flag = false;
	mutex_init(&fcc_info.lock);
}

u8 sprd_pw_backoff_band2value(u8 channel)
{
	u8 value = 0;

	if (!channel)
		return value;
	mutex_lock(&g_set_5g_sar_info.lock);
	g_set_5g_sar_info.channel = channel;
	switch (channel) {
	case 30 ... 50:
		value = g_set_5g_sar_info.value[0];
		break;
	case 51 ... 70:
		value = g_set_5g_sar_info.value[1];
		break;
	case 71 ... 145:
		value = g_set_5g_sar_info.value[2];
		break;
	case 146 ... 170:
		value = g_set_5g_sar_info.value[3];
		break;
	default:
		value = g_set_5g_sar_info.value[4];
		break;
	}
	mutex_unlock(&g_set_5g_sar_info.lock);
	return value;

}

void sprd_5g_sar_info_init(void)
{
	mutex_init(&g_set_5g_sar_info.lock);
	g_set_5g_sar_info.channel = 0;
	memset(g_set_5g_sar_info.value, 0x00, 5);
}

void sprd_5g_sar_info_reset(void)
{
	mutex_lock(&g_set_5g_sar_info.lock);
	g_set_5g_sar_info.channel = 0;
	mutex_unlock(&g_set_5g_sar_info.lock);
}
void sprd_5g_sar_info_set(unsigned char *data)
{
	mutex_lock(&g_set_5g_sar_info.lock);
	if (data == NULL)
		memset(g_set_5g_sar_info.value, 0x00, 5);
	else
		memcpy(g_set_5g_sar_info.value, data, 5);
	wl_info("%s band sar: %d, %d, %d, %d, %d\n", __func__,
			g_set_5g_sar_info.value[0],
			g_set_5g_sar_info.value[1],
			g_set_5g_sar_info.value[2],
			g_set_5g_sar_info.value[3],
			g_set_5g_sar_info.value[4]);

	mutex_unlock(&g_set_5g_sar_info.lock);
}
