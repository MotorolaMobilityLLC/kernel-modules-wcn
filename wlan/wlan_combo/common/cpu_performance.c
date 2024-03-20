// SPDX-License-Identifier: GPL-2.0-only
/*
 * SPDX-FileCopyrightText: 2023-2024 Unisoc (Shanghai) Technologies Co. Ltd
 */

#include <uapi/linux/sched/types.h>
#include <linux/version.h>

#include "cpu_performance.h"
#include "debug.h"

struct throughput_sta throughput_static;

struct threshold_table threshold_tables[] = {
	{0, 0},
	{0, 0},
	{16 * 0x100000, 16 * 0x100000},		//128Mbit/s  or 16Mbyte/s
	{16 * 0x100000, 16 * 0x100000},
	{16 * 0x100000, 16 * 0x100000},
	{16 * 0x100000, 16 * 0x100000},
};

#define SPRD_TP_TYPE(hif)			\
	((hif)->hw_type == SPRD_HW_SC2355_SDIO)

void sprd_tp_static_init(void)
{
	throughput_static.tx_bytes = 0;
	throughput_static.last_time = jiffies;
	throughput_static.rx_bytes = 0;
	throughput_static.rx_last_time = jiffies;
	throughput_static.disable_pd_flag = false;
	throughput_static.uclamp_set_flag = false;
	throughput_static.throughput_tx = 0;
	throughput_static.throughput_rx = 0;

#if (KERNEL_VERSION(5, 15, 0) <= LINUX_VERSION_CODE)
	cpu_latency_qos_add_request(&throughput_static.pm_qos_request_idle,
				    PM_QOS_CPU_LATENCY_DEFAULT_VALUE);
#else
	pm_qos_add_request(&throughput_static.pm_qos_request_idle,
			   PM_QOS_CPU_DMA_LATENCY, PM_QOS_CPU_DMA_LAT_DEFAULT_VALUE);
#endif
}

void sprd_tp_static_deinit(void)
{
#if (KERNEL_VERSION(5, 15, 0) <= LINUX_VERSION_CODE)
	cpu_latency_qos_remove_request(&throughput_static.pm_qos_request_idle);
#else
	pm_qos_remove_request(&throughput_static.pm_qos_request_idle);
#endif
}

void sprd_tp_ctl_core_pd(struct sprd_hif *hif, unsigned int len)
{
	if (!SPRD_TP_TYPE(hif))
		return;

	throughput_static.tx_bytes += len;

	if (time_after(jiffies, throughput_static.last_time + msecs_to_jiffies(1000))) {
		throughput_static.last_time = jiffies;
		if (throughput_static.tx_bytes >=
		    threshold_tables[hif->hw_type].disable_pd ||
		    throughput_static.throughput_rx >=
		    threshold_tables[hif->hw_type].disable_pd) {
			if (!throughput_static.disable_pd_flag) {
				throughput_static.disable_pd_flag = true;
				// forbid core powerdown
#if (KERNEL_VERSION(5, 15, 0) <= LINUX_VERSION_CODE)
				cpu_latency_qos_update_request(&throughput_static.pm_qos_request_idle, 100);
#else
				pm_qos_update_request(&throughput_static.pm_qos_request_idle, 100);
#endif
			}
		} else {
			if (throughput_static.disable_pd_flag) {
				throughput_static.disable_pd_flag = false;
				//allow core powerdown
#if (KERNEL_VERSION(5, 15, 0) <= LINUX_VERSION_CODE)
				cpu_latency_qos_update_request(&throughput_static.pm_qos_request_idle,
							       PM_QOS_CPU_LATENCY_DEFAULT_VALUE);
#else
				pm_qos_update_request(&throughput_static.pm_qos_request_idle,
						      PM_QOS_CPU_DMA_LAT_DEFAULT_VALUE);
#endif
			}
		}
		throughput_static.throughput_tx = throughput_static.tx_bytes;
		throughput_static.tx_bytes = 0;
	}
}

void sprd_rx_tp_statistic(struct sprd_hif *hif, unsigned int len)
{
	if (!SPRD_TP_TYPE(hif))
		return;

	throughput_static.rx_bytes += len;

	if (time_after(jiffies, throughput_static.rx_last_time +  msecs_to_jiffies(1000))) {
		throughput_static.rx_last_time = jiffies;
		throughput_static.throughput_rx = throughput_static.rx_bytes;
		throughput_static.rx_bytes = 0;
	}
}

//set uclamp params for bug 1959864
int sprd_set_thread_uclamp(struct task_struct *thread, int sched_util_min)
{
	struct sched_attr attr = {};
	int ret = 0;

	if (!thread) {
		wl_err("%s: failed to set sched attr point thread null\n", __func__);
		return -1;
	}
	attr.sched_policy = thread->policy;
	if (thread->sched_reset_on_fork)
		attr.sched_flags |= SCHED_FLAG_RESET_ON_FORK;
	attr.sched_flags |= (SCHED_FLAG_KEEP_ALL | SCHED_FLAG_UTIL_CLAMP_MIN);
	attr.sched_util_min = sched_util_min;
	ret = sched_setattr(thread, &attr);

	return ret;
}

/* reset pd and uclamp parameters */
void sprd_reset_cpu_prf_param(struct task_struct *thread)
{
	if (throughput_static.disable_pd_flag) {
		throughput_static.disable_pd_flag = false;
		//allow core powerdown
#if (KERNEL_VERSION(5, 15, 0) <= LINUX_VERSION_CODE)
		cpu_latency_qos_update_request(&throughput_static.pm_qos_request_idle,
					       PM_QOS_CPU_LATENCY_DEFAULT_VALUE);
#else
		pm_qos_update_request(&throughput_static.pm_qos_request_idle,
				      PM_QOS_CPU_DMA_LAT_DEFAULT_VALUE);
#endif
	}

	//reset thread uclamp param
	sprd_set_thread_uclamp(thread, 0);
	throughput_static.uclamp_set_flag = false;
	throughput_static.tx_bytes = 0;
	throughput_static.rx_bytes = 0;
	throughput_static.throughput_tx = 0;
	throughput_static.throughput_rx = 0;
}

void sprd_tp_ctl_uclamp(struct sprd_hif *hif, struct task_struct *thread)
{
	if (!SPRD_TP_TYPE(hif))
		return;

	if (!throughput_static.uclamp_set_flag &&
	    (throughput_static.throughput_tx >=
	    threshold_tables[hif->hw_type].set_uclamp ||
	    throughput_static.throughput_rx >=
	    threshold_tables[hif->hw_type].set_uclamp)) {
#if (KERNEL_VERSION(5, 15, 0) <= LINUX_VERSION_CODE)
		sprd_set_thread_uclamp(thread, 600);
#else
		sprd_set_thread_uclamp(thread, 400);
#endif
		throughput_static.uclamp_set_flag = true;
	} else if (throughput_static.uclamp_set_flag &&
		   throughput_static.throughput_tx <
		   threshold_tables[hif->hw_type].set_uclamp &&
		   throughput_static.throughput_rx <
		   threshold_tables[hif->hw_type].set_uclamp) {
		sprd_set_thread_uclamp(thread, 0);
		throughput_static.uclamp_set_flag = false;
	}
}

#if (KERNEL_VERSION(5, 15, 0) <= LINUX_VERSION_CODE)
extern int wcn_thread_setattr(unsigned dir, struct sched_attr *attr);
static struct sched_attr attr;
#endif
void sprd_set_wcn_thread_uclamp(struct sprd_hif *hif)
{
#if (KERNEL_VERSION(5, 15, 0) <= LINUX_VERSION_CODE)
	if (hif->hw_type != SPRD_HW_SC2355_SDIO)
		return;
	attr.sched_flags |= (SCHED_FLAG_KEEP_ALL | SCHED_FLAG_UTIL_CLAMP_MIN);
	attr.sched_policy = SCHED_NORMAL;
	if (attr.sched_util_min != 400 &&
	    throughput_static.throughput_rx >=
	    threshold_tables[hif->hw_type].set_uclamp) {
		attr.sched_util_min = 400;
		wcn_thread_setattr(0, &attr);
	/*need reset sdiohal_rx_thread util to 0*/
	} else if (attr.sched_util_min &&
		   throughput_static.throughput_rx <
		   threshold_tables[hif->hw_type].set_uclamp) {
		attr.sched_util_min = 0;
		wcn_thread_setattr(0, &attr);
	}
#endif
}

