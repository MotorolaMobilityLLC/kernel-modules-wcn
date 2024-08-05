/*
* SPDX-FileCopyrightText: 2020-2023 Unisoc (Shanghai) Technologies Co. Ltd
* SPDX-License-Identifier: GPL-2.0-only
*/

#include "common/common.h"
#include "common/npi.h"
#include "cmdevt.h"

/* subtype, channel, bw, {mode(2.4g : b,g,n,ac; 5g : a,n,ac), value} */

#define num_ce 8
#define num_fcc 12

#define power_backoff_ce { \
	{1, 149, 0, { {0, 127}, {1, 127}, {2, 127}, {3, 127}, {4, 12}, {5, 12}, {6, 12} } }, \
	{1, 153, 0, { {0, 127}, {1, 127}, {2, 127}, {3, 127}, {4, 12}, {5, 12}, {6, 12} } }, \
	{1, 157, 0, { {0, 127}, {1, 127}, {2, 127}, {3, 127}, {4, 12}, {5, 12}, {6, 12} } }, \
	{1, 161, 0, { {0, 127}, {1, 127}, {2, 127}, {3, 127}, {4, 12}, {5, 12}, {6, 12} } }, \
	{1, 165, 0, { {0, 127}, {1, 127}, {2, 127}, {3, 127}, {4, 12}, {5, 12}, {6, 12} } }, \
	{1, 151, 1, { {0, 127}, {1, 127}, {2, 127}, {3, 127}, {4, 12}, {5, 12}, {6, 12} } }, \
	{1, 159, 1, { {0, 127}, {1, 127}, {2, 127}, {3, 127}, {4, 12}, {5, 12}, {6, 12} } }, \
	{1, 155, 2, { {0, 127}, {1, 127}, {2, 127}, {3, 127}, {4, 12}, {5, 12}, {6, 12} } }, \
	}

#define power_backoff_fcc { \
	{1, 1, 0, { {0, 127}, {1, 15}, {2, 15}, {3, 127}, {4, 127}, {5, 127}, {6, 127} } },  \
	{1, 11, 0, { {0, 127}, {1, 15}, {2, 14}, {3, 127}, {4, 127}, {5, 127}, {6, 127} } }, \
	{1, 3, 1, { {0, 127}, {1, 127}, {2, 14}, {3, 127}, {4, 127}, {5, 127}, {6, 127} } }, \
	{1, 9, 1, { {0, 127}, {1, 127}, {2, 13}, {3, 127}, {4, 127}, {5, 127}, {6, 127} } }, \
	{1, 36, 0, { {0, 127}, {1, 127}, {2, 127}, {3, 127}, {4, 16}, {5, 15}, {6, 15} } }, \
	{1, 100, 0, { {0, 127}, {1, 127}, {2, 127}, {3, 127}, {4, 15}, {5, 14}, {6, 14} } }, \
	{1, 140, 0, { {0, 127}, {1, 127}, {2, 127}, {3, 127}, {4, 12}, {5, 11}, {6, 11} } }, \
	{1, 38, 1, { {0, 127}, {1, 127}, {2, 127}, {3, 127}, {4, 127}, {5, 13}, {6, 13} } }, \
	{1, 62, 1, { {0, 127}, {1, 127}, {2, 127}, {3, 127}, {4, 127}, {5, 15}, {6, 15} } }, \
	{1, 102, 1, { {0, 127}, {1, 127}, {2, 127}, {3, 127}, {4, 127}, {5, 14}, {6, 14} } }, \
	{1, 42, 2, { {0, 127}, {1, 127}, {2, 127}, {3, 127}, {4, 127}, {5, 127}, {6, 14} } }, \
	{1, 106, 2, { {0, 127}, {1, 127}, {2, 127}, {3, 127}, {4, 127}, {5, 127}, {6, 16} } }, \
	}

static struct sprd_fcc_priv fcc_info;

static struct fcc_power_bo g_fcc_power_table[MAX_FCC_COUNTRY_NUM] = {
	{
		.country = "AE",
		.num = num_ce,
		.power_backoff = power_backoff_ce,
	},
	{
		.country = "BR",
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
		.country = "DE",
		.num = num_ce,
		.power_backoff = power_backoff_ce,
	},
	{
		.country = "EC",
		.num = num_ce,
		.power_backoff = power_backoff_ce,
	},
	{
		.country = "ES",
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
		.country = "IN",
		.num = num_ce,
		.power_backoff = power_backoff_ce,
	},
	{
		.country = "IT",
		.num = num_ce,
		.power_backoff = power_backoff_ce,
	},
	{
		.country = "NL",
		.num = num_ce,
		.power_backoff = power_backoff_ce,
	},
	{
		.country = "PE",
		.num = num_ce,
		.power_backoff = power_backoff_ce,
	},
	{
		.country = "PL",
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
		.country = "TN",
		.num = num_ce,
		.power_backoff = power_backoff_ce,
	},
	{
		.country = "UA",
		.num = num_ce,
		.power_backoff = power_backoff_ce,
	},
	{
		.country = "UY",
		.num = num_ce,
		.power_backoff = power_backoff_ce,
	},
	{
		.country = "AR",
		.num = num_fcc,
		.power_backoff = power_backoff_fcc,
	},
	{
		.country = "AU",
		.num = num_fcc,
		.power_backoff = power_backoff_fcc,
	},
	{
		.country = "CO",
		.num = num_fcc,
		.power_backoff = power_backoff_fcc,
	},
	{
		.country = "DO",
		.num = num_fcc,
		.power_backoff = power_backoff_fcc,
	},
	{
		.country = "GT",
		.num = num_fcc,
		.power_backoff = power_backoff_fcc,
	},
	{
		.country = "MX",
		.num = num_fcc,
		.power_backoff = power_backoff_fcc,
	},
	{
		.country = "OM",
		.num = num_fcc,
		.power_backoff = power_backoff_fcc,
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
