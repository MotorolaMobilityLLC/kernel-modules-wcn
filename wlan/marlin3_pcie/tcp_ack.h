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

#ifndef __SPRDWL_TCP_ACK_H__
#define __SPRDWL_TCP_ACK_H__

#include "msg.h"

#define SPRDWL_TCP_ACK_NUM  32
#define SPRDWL_TCP_ACK_EXIT_VAL		0x800
#define SPRDWL_TCP_ACK_DROP_CNT		12

#define SPRDWL_ACK_OLD_TIME	4000
#define SPRDWL_U32_BEFORE(a, b)	((__s32)((__u32)a - (__u32)b) <= 0)

#define MAX_TCP_ACK 200
/*min window size in KB, it's 256KB*/
#define MIN_WIN 256
#define SIZE_KB 1024

extern unsigned int tcp_ack_drop_cnt;
struct sprdwl_tcp_ack_msg {
	u16 source;
	u16 dest;
	s32 saddr;
	s32 daddr;
	u32 seq;
	u16 win;
};

struct sprdwl_tcp_ack_info {
	int ack_info_num;
	int busy;
	int drop_cnt;
	int psh_flag;
	u32 psh_seq;
	u16 win_scale;
	/* seqlock for ack info */
	seqlock_t seqlock;
	unsigned long last_time;
	unsigned long timeout;
	struct timer_list timer;
	struct sprdwl_msg_buf *msgbuf;
	struct sprdwl_msg_buf *in_send_msg;
	struct sprdwl_tcp_ack_msg ack_msg;
};

struct sprdwl_tcp_ack_manage {
	/* 1 filter */
	atomic_t enable;
	int max_num;
	int free_index;
	unsigned long last_time;
	unsigned long timeout;
	atomic_t max_drop_cnt;
	/* lock for tcp ack alloc and free */
	spinlock_t lock;
	struct sprdwl_priv *priv;
	struct sprdwl_tcp_ack_info ack_info[SPRDWL_TCP_ACK_NUM];
	/*size in KB*/
	unsigned int ack_winsize;
};

void sprdwl_tcp_ack_init(struct sprdwl_priv *priv);
void sprdwl_tcp_ack_deinit(struct sprdwl_priv *priv);
void sprdwl_filter_rx_tcp_ack(struct sprdwl_priv *priv,
			      unsigned char *buf, unsigned int plen);
/* return val: 0 for not fileter, 1 for fileter */
int sprdwl_filter_send_tcp_ack(struct sprdwl_priv *priv,
			       struct sprdwl_msg_buf *msgbuf,
			       unsigned char *buf, unsigned int plen);
void enable_tcp_ack_delay(char *buf, unsigned char offset);
void adjust_tcp_ack_delay(char *buf, unsigned char offset);
void sprdwl_move_tcpack_msg(struct sprdwl_priv *priv,
			    struct sprdwl_msg_buf *msg);
void adjust_tcp_ack_delay_win(char *buf, unsigned char offset);
#endif
