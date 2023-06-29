/*
  * SPDX-FileCopyrightText: 2021-2023 Unisoc (Shanghai) Technologies Co. Ltd
  * SPDX-License-Identifier: GPL-2.0-only
  */
#include <uapi/linux/sched/types.h>
#include <linux/version.h>

#include "cpu_performance.h"
#include "debug.h"

static struct throughput_sta throughput_static;
void sc2355_tp_static_init(void)
{
	throughput_static.tx_bytes = 0;
	throughput_static.tx_last_time = jiffies;
	throughput_static.rx_bytes = 0;
	throughput_static.rx_last_time = jiffies;
	throughput_static.disable_pd_flag = false;
	throughput_static.uclamp_set_flag = false;
	throughput_static.throughput_tx = 0;
	throughput_static.throughput_rx = 0;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 15, 0))
	cpu_latency_qos_add_request(&throughput_static.pm_qos_request_idle,
				    PM_QOS_CPU_LATENCY_DEFAULT_VALUE);
#else
	pm_qos_add_request(&throughput_static.pm_qos_request_idle,
			   PM_QOS_CPU_DMA_LATENCY, PM_QOS_CPU_DMA_LAT_DEFAULT_VALUE);
#endif
}

void sc2355_tp_static_deinit(void)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 15, 0))
	cpu_latency_qos_remove_request(&throughput_static.pm_qos_request_idle);
#else
	pm_qos_remove_request(&throughput_static.pm_qos_request_idle);
#endif
}

void sc2355_tp_ctl_core_pd(unsigned int len)
{
	throughput_static.tx_bytes += len;
	if (time_after(jiffies, throughput_static.tx_last_time +  msecs_to_jiffies(1000))) {
		throughput_static.tx_last_time = jiffies;
		if ((throughput_static.tx_bytes >= DISABLE_PD_THRESHOLD) ||
			(throughput_static.throughput_rx >= DISABLE_PD_THRESHOLD)) {
			if (!throughput_static.disable_pd_flag)	{
				throughput_static.disable_pd_flag = true;
				// forbid core powerdown
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 15, 0))
				cpu_latency_qos_update_request(&throughput_static.pm_qos_request_idle, 100);
#else
				pm_qos_update_request(&throughput_static.pm_qos_request_idle, 100);
#endif
			}
		} else {
			if (throughput_static.disable_pd_flag) {
				throughput_static.disable_pd_flag = false;
				//allow core powerdown
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 15, 0))
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

void sc2355_rx_tp_statistic(unsigned int len)
{
	throughput_static.rx_bytes += len;
	if (time_after(jiffies, throughput_static.rx_last_time +  msecs_to_jiffies(1000))) {
		throughput_static.rx_last_time = jiffies;
		throughput_static.throughput_rx = throughput_static.rx_bytes;
		throughput_static.rx_bytes = 0;
	}
}

//set uclamp params for bug 1959864
static int sc2355_set_thread_uclamp(struct task_struct *thread, int sched_util_min)
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
void sc2355_reset_cpu_prf_param(struct sprd_hif *hif)
{
	struct tx_mgmt *tx_mgmt = (struct tx_mgmt *)hif->tx_mgmt;

	if (throughput_static.disable_pd_flag) {
		throughput_static.disable_pd_flag = false;
		//allow core powerdown
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 15, 0))
		cpu_latency_qos_update_request(&throughput_static.pm_qos_request_idle,
					      PM_QOS_CPU_LATENCY_DEFAULT_VALUE);
#else
		pm_qos_update_request(&throughput_static.pm_qos_request_idle,
					      PM_QOS_CPU_DMA_LAT_DEFAULT_VALUE);
#endif
	}

	if (throughput_static.uclamp_set_flag) {
		//reset thread uclamp param
		sc2355_set_thread_uclamp(tx_mgmt->tx_thread, 0);
		throughput_static.uclamp_set_flag = false;
	}

	throughput_static.tx_bytes = 0;
	throughput_static.rx_bytes = 0;
	throughput_static.throughput_tx = 0;
	throughput_static.throughput_rx = 0;
}

void sc2355_tp_ctl_uclamp(struct sprd_hif *hif)
{
	struct tx_mgmt *tx_mgmt = (struct tx_mgmt *)hif->tx_mgmt;

	if (!throughput_static.uclamp_set_flag &&
		(throughput_static.throughput_tx >= SET_UCLAMP_THRESHOLD ||
		throughput_static.throughput_rx >= SET_UCLAMP_THRESHOLD)) {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 15, 0))
		sc2355_set_thread_uclamp(tx_mgmt->tx_thread, 600);
#else
		sc2355_set_thread_uclamp(tx_mgmt->tx_thread, 400);
#endif
		throughput_static.uclamp_set_flag = true;
	} else if (throughput_static.uclamp_set_flag &&
			(throughput_static.throughput_tx < SET_UCLAMP_THRESHOLD &&
			throughput_static.throughput_rx < SET_UCLAMP_THRESHOLD)) {
		sc2355_set_thread_uclamp(tx_mgmt->tx_thread, 0);
		throughput_static.uclamp_set_flag = false;
	}
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 15, 0))
extern int wcn_thread_setattr(unsigned dir, struct sched_attr *attr);
static struct sched_attr attr;
void sc2355_set_wcn_thread_uclamp(void)
{
	attr.sched_flags |= (SCHED_FLAG_KEEP_ALL | SCHED_FLAG_UTIL_CLAMP_MIN);
	attr.sched_policy = SCHED_NORMAL;
	if (attr.sched_util_min != 400 &&
		throughput_static.throughput_rx >= SET_UCLAMP_THRESHOLD) {
		attr.sched_util_min = 400;
		wcn_thread_setattr(0, &attr);
	/*need reset sdiohal_rx_thread util to 0*/
	} else if (attr.sched_util_min &&
		throughput_static.throughput_rx < SET_UCLAMP_THRESHOLD) {
		attr.sched_util_min = 0;
		wcn_thread_setattr(0, &attr);
	}
}
#endif

