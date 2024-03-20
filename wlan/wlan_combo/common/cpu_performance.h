/* SPDX-License-Identifier: GPL-2.0-only
 * SPDX-FileCopyrightText: 2023-2024 Unisoc (Shanghai) Technologies Co. Ltd
 */

#ifndef __CPU_PERFORMANCE_H__
#define __CPU_PERFORMANCE_H__

#include <linux/pm_qos.h>
#include "hif.h"

struct throughput_sta {
	unsigned long tx_bytes;
	unsigned long last_time;
	unsigned long rx_bytes;
	unsigned long rx_last_time;
	unsigned long throughput_rx;
	unsigned long throughput_tx;
	bool disable_pd_flag;
	bool uclamp_set_flag;
	struct  pm_qos_request pm_qos_request_idle;
};

struct threshold_table {
	u32 set_uclamp;
	u32 disable_pd;
};

extern struct throughput_sta throughput_static;

void sprd_tp_static_init(void);
void sprd_tp_static_deinit(void);
void sprd_rx_tp_statistic(struct sprd_hif *hif, unsigned int len);
void sprd_tp_ctl_core_pd(struct sprd_hif *hif, unsigned int len);
void sprd_reset_cpu_prf_param(struct task_struct *thread);
void sprd_tp_ctl_uclamp(struct sprd_hif *hif, struct task_struct *thread);
int sprd_set_thread_uclamp(struct task_struct *thread, int sched_util_min);
void sprd_set_wcn_thread_uclamp(struct sprd_hif *hif);

#endif
