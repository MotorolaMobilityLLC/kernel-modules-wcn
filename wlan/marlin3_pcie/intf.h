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

#ifndef __SPRDWL_INTF_H__
#define __SPRDWL_INTF_H__

#include "sprdwl.h"
#include "msg.h"

struct sprdwl_priv;

struct sprdwl_if_ops {
	struct sprdwl_msg_buf *(*get_msg_buf)(void *sdev,
					      enum sprdwl_head_type type,
#if defined(SC2355_FTR)
					      enum sprdwl_mode mode,
					      u8 ctx_id);
#else
					      enum sprdwl_mode mode);
#endif
	void (*free_msg_buf)(void *sdev, struct sprdwl_msg_buf *msg);
	int (*tx)(void *spdev, struct sprdwl_msg_buf *msg);
#if defined(SC2355_FTR)
	void (*force_exit)(void *spdev);
	int (*is_exit)(void *spdev);
#else
	void (*force_exit)(void);
	int (*is_exit)(void);
#endif /* SC2355_FTR */
	int (*suspend)(struct sprdwl_priv *priv);
	int (*resume)(struct sprdwl_priv *priv);
#if defined(SC2355_FTR)
	void (*debugfs)(void *spdev, struct dentry *dir);
#else
	void (*debugfs)(struct dentry *dir);
#endif /* SC2355_FTR */
#if defined(SC2355_FTR)
	void (*tcp_drop_msg)(void *spdev, struct sprdwl_msg_buf *msg);
#else
	void (*tcp_drop_msg)(struct sprdwl_msg_buf *msg);
#endif /* SC2355_FTR*/
	int (*ini_download_status)(void);
};

#endif/*__INTF_H__*/
