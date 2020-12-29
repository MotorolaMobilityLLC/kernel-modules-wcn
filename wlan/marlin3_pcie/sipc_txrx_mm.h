/*
 * Copyright (C) 2020 Spreadtrum Communications Inc.
 *
 * Authors	:
 * baojie.cai <baojie.cai@unisoc.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef __SIPC_TXRX_BUF_MM_H__
#define __SIPC_TXRX_BUF_MM_H__

#include <linux/types.h>
#include <linux/spinlock.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/skbuff.h>
#include "core_sc2355.h"
#include "msg.h"


enum {
	SIPC_LOC_BUFF_FREE,
	SIPC_LOC_TX_INTF,
	SIPC_LOC_RX_INTF,
};

enum {
	SIPC_MEMORY_FREE,
	SIPC_MEMORY_ALLOC = 0x5a,
};

struct sipc_buf_node {
	struct list_head list;
	u8 flag;
	u8 ctxt_id;
	u8 location;
	u8 resv;
	void *addr;
	void *buf;
	void *priv;
} __packed;

struct sipc_mem_region {
	phys_addr_t phy_base;
	void  *virt_base;
	size_t size;
	unsigned int page_count;
};

#define SPRDWL_SIPC_MEM_TXRX_TOTAL				0x280000
#define SPRDWL_SIPC_MEM_RX_OFFSET				0x1B5800
#define SIPC_TXRX_BUF_BLOCK_TYPE               (1)
#define SIPC_TXRX_BUF_SINGLE_TYPE              (0)
#define SIPC_TXRX_TX_BUF_MAX_NUM               (300)
#define SIPC_TXRX_RX_BUF_MAX_NUM               (450)
struct sipc_buf_mm {
	struct sprdwl_msg_list nlist;
	void *virt_start;
	void *virt_end;
	unsigned long offset;
	int len;
	int type;
	int buf_count;
	int padding;
};

struct sipc_txrx_mm {
	struct sipc_buf_mm *tx_buf;
	struct sipc_buf_mm *rx_buf;
	struct sipc_mem_region smem;
};



struct sipc_buf_node *sipc_buf_mm_alloc(struct sipc_buf_mm *mm);
struct sipc_buf_node *sipc_alloc_tx_buf(void);
void sipc_free_tx_buf(struct sipc_buf_node *node);
int sipc_get_tx_buf_len(void);
int sipc_get_tx_buf_num(void);
int sipc_txrx_buf_init(struct platform_device *pdev);
void sipc_txrx_buf_deinit(void);
struct sipc_buf_node *sipc_rx_alloc_node_buf(void);
void sipc_mm_rx_buf_deinit(void);
void sipc_mm_rx_buf_flush(void);
void sipc_free_node_buf(struct sipc_buf_node *node,
			 struct sprdwl_msg_list *list);
struct sipc_buf_mm *sipc_get_rx_mm_buf(void);
struct sipc_buf_mm *sipc_get_tx_mm_buf(void);
int sipc_skb_to_tx_buf(struct sprdwl_intf *dev,
				struct sprdwl_msg_buf *msg_pos);
struct sk_buff *sipc_rx_mm_buf_to_skb(struct sk_buff *skb);
void sipc_queue_node_buf(struct sipc_buf_node *node,
			  struct sprdwl_msg_list *list);
void sipc_rx_list_dump(void);
void sprdwl_sipc_txrx_buf_deinit(enum sprdwl_hw_type hw_type);
void sipc_set_address_offset(unsigned long offset);
unsigned long sipc_get_address_offset(void);
void *sipc_fill_mbuf(void *data, unsigned int len);

#endif /*__SIPC_TXRX_BUF_MM_H__*/
