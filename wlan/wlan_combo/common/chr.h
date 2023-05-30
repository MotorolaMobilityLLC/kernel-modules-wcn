/*
* SPDX-FileCopyrightText: 2020-2023 Unisoc (Shanghai) Technologies Co. Ltd
* SPDX-License-Identifier: GPL-2.0-only
*/

#ifndef __CHR_H__
#define __CHR_H__

#include <linux/delay.h>
#include <linux/err.h>
#include <linux/inet.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/net.h>
#include <linux/sched.h>
#include <net/net_namespace.h>
#include <net/sock.h>
#include <uapi/asm-generic/errno.h>
#include <uapi/linux/in.h>

#define CHR_VERSION			1
#define CHR_ARR_SIZE			64
#define CHR_BUF_SIZE			1024
#define CHR_CP2_DATA_LEN		11

#define CHR_OPENERR_FLAGSET(A, B) 	(*A = B)
/* format negotiated with CP2 */
struct evt_chr {
	u8 version; /* reserve for future */
	u32 evt_id;
	u32 evt_id_subtype; /* reserve for future */
	u8 evt_content_len;
	u8 *evt_content; /* CP2 define the event_content size is 100byte */
} __packed;

/* used by driver to store CHR params*/
struct chr_driver_params {
	u16 refcnt;
	u32 evt_id;
	u8 version;
	u8 evt_content_len;
	u8 *evt_content;
};

struct chr_open_error {
	u8 reason_code; /* 0 is power_on err, 1 is download_ini err*/
};

struct chr_linkloss_disc_error {
	u8 reason_code; /* 1 is device power off, 2 is beacon loss */
};

struct chr_system_disc_error {
	u8 reason_code;
};

struct chr_cmd {
	u8 evt_type[18];
	u8 module[12];
	u32 evt_id;
	u32 set;
	u32 maxcount;
	u32 timerlimit;
};

struct chr_refcnt_arr {
	u16 open_err_cnt[CHR_ARR_SIZE];
	u16 disc_linkloss_cnt[CHR_ARR_SIZE];
	u16 disc_systerr_cnt[CHR_ARR_SIZE];
};

/* The flag just used in sprd_iface_set_power to determine open_err evt*/
enum OPEN_ERR_LIST {
	OPEN_ERR_INIT = 0,
	OPEN_ERR_POWER_ON,
	OPEN_ERR_DOWNLOAD_INI
};

/* set chr cmd to cp2*/
struct cmd_chr_mode {
	u8 on_flag; /* 1 enable, 0 disable*/
	u8 version; /* reserve for future */
	u32 chr_evt_id[10];/* each array element represents an event id */
} __packed;

/* The following is about CHR */
enum REPORT_CHR_LIST {
	EVT_CHR_WIFI_MIN = 0x11501,

	/* Error From Driver */
	EVT_CHR_DRV_MIN = EVT_CHR_WIFI_MIN,

	EVT_CHR_OPEN_ERR = EVT_CHR_DRV_MIN,

	EVT_CHR_DRV_MAX = 0X13000,
	/* Wi-Fi Disconnect */
	EVT_CHR_FW_MIN = 0X13001,

	EVT_CHR_DISC_LINK_LOSS = EVT_CHR_FW_MIN,
	EVT_CHR_DISC_SYS_ERR,

	EVT_CHR_FW_MAX = 0X15000,

	EVT_CHR_WIFI_MAX = EVT_CHR_FW_MAX
};

struct sprd_chr {
	/* 0 means haven't received any messages,
	 * 1 is have received messages about open chr_evt,
	 * 2 is have received messages about close all chr_evt
	 */
	u8 sock_flag;
	u8 thread_exit;
	u8 open_err_flag;
	struct sprd_priv *priv;
	struct sprd_hif *hif;

	struct task_struct *chr_client_thread;
	/* this struct saves all chr_evt_refcnt*/
	struct chr_refcnt_arr *chr_refcnt;
	struct socket *chr_sock;

	/* this val only stores the chr_buf for CP2*/
	struct chr_cmd fw_cmd_list[CHR_ARR_SIZE];
	u32 fw_len;
	/* this val only stores the chr_buf for drv */
	struct chr_cmd drv_cmd_list[CHR_ARR_SIZE];
	u32 drv_len;
};


int sprd_chr_init(struct sprd_chr *chr);
void sprd_chr_deinit(struct sprd_chr *chr);

/* This function is used to report chr_disconnect evt from CP2 */
void sprd_chr_report_disconnect(struct sprd_vif *vif, u8 version,
				u32 evt_id, u32 evt_id_subtype,
				u8 evt_content_len, u8 *evt_content);
void sprd_chr_report_open_error(struct sprd_chr *chr, u32 evt_id,
				u8 err_code);
void sprd_chr_handle_open(struct sprd_chr *chr);
void sprd_chr_handle_power(struct sprd_chr *chr);
int sprd_chr_handle_probe(struct sprd_hif *hif, struct sprd_chr *chr);

#endif
