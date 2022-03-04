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

#ifdef SOFTAP_HOOK
void sprdwl_hook_reset_channel(struct wiphy *wiphy,
			       struct cfg80211_ap_settings *set);
#else
static inline void sprdwl_hook_reset_channel(struct wiphy *wiphy,
					     struct cfg80211_ap_settings *set)
{
}
#endif
