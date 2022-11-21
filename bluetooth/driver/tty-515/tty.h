/*
 * Copyright (C) 2015 Spreadtrum Communications Inc.
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

#ifndef __MTTY_H
#define __MTTY_H
#include <misc/wcn_bus.h>


//sipc
#define SPRD_BT_DST         3
#define SPRD_BT_CHANNEL     4
#define SPRD_BT_TX_BUFID    11
#define SPRD_BT_RX_BUFID    10

enum mtty_state {
    MTTY_STATE_CLOSE,
    MTTY_STATE_OPEN
};

enum mtty_log_level {
    MTTY_LOG_LEVEL_NONE,
    MTTY_LOG_LEVEL_VER,
};

/*struct mtty_init_data {
    char *name;
};*/

//sipc2
struct mtty_init_data {
    char        *name;
    uint8_t     dst;
    uint8_t     channel;
    uint32_t    tx_bufid;
    uint32_t    rx_bufid;
};

enum sprd_hif_type {

    SPRD_HW_SC2332_SIPC,
    SPRD_HW_SC2355_SDIO,
    SPRD_HW_SC2355_PCIE,
    SPRD_HW_SC2355_SIPC2,
};


struct mtty_match_data {
    enum sprd_hif_type hw_type;
};

#define BT_SIPC_HEAD_LEN      0

#define MTTY_DEV_MAX_NR     1
#define BT_TX_CHANNEL    3
//#define BT_TX_CHANNEL1    4
#define BT_RX_CHANNEL     17
#define BT_TX_INOUT    1
#define BT_RX_INOUT     0
#define BT_TX_POOL_SIZE   64  // the max buffer is 64
#define BT_RX_POOL_SIZE   1
#define BT_SDIO_HEAD_LEN   4

#define BT_SIPC_TX_CHANNEL    8
#define BT_SIPC_RX_CHANNEL    9

#define BT_SIPC_RX_MAX_NUM    4
#define BT_SIPC_RX_DMA_SIZE   2048
#endif
