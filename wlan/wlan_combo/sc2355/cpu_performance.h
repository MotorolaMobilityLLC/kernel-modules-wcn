/*
* SPDX-FileCopyrightText: 2021-2023 Unisoc (Shanghai) Technologies Co. Ltd
* SPDX-License-Identifier: GPL-2.0-only
*/

#ifndef __CPU_PERFORMANCE_H__
#define __CPU_PERFORMANCE_H__

#include <linux/pm_qos.h>

#include "tx.h"

#define DISABLE_PD_THRESHOLD (16 * 0x100000)  //128Mbit/s  or 16Mbyte/s
#define SET_UCLAMP_THRESHOLD (16 * 0x100000)  //128Mbit/s  or 16Mbyte/s

struct throughput_sta {
	unsigned long tx_bytes;
	unsigned long tx_last_time;
	unsigned long rx_bytes;
	unsigned long rx_last_time;
	unsigned long throughput_rx;
	unsigned long throughput_tx;
	bool disable_pd_flag;
	bool uclamp_set_flag;
	struct  pm_qos_request pm_qos_request_idle;
};

void sc2355_tp_static_init(void);
void sc2355_tp_static_deinit(void);
void sc2355_rx_tp_statistic(unsigned int len);
void sc2355_tp_ctl_core_pd(unsigned int len);
void sc2355_reset_cpu_prf_param(struct sprd_hif *hif);
void sc2355_tp_ctl_uclamp(struct sprd_hif *hif);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 15, 0))
void sc2355_set_wcn_thread_uclamp(void);
#endif
#endif
