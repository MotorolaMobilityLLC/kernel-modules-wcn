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

#ifndef __EDMA_TEST_H__
#define __EDMA_TEST_H__

enum EDMA_TEST_CMD{
	EDMA_TEST_CMD_0,
	EDMA_TEST_CMD_1,
	EDMA_TEST_CMD_2,
	EDMA_TEST_CMD_3,
	EDMA_TEST_CMD_4,
	EDMA_TEST_CMD_5,
	EDMA_TEST_CMD_6,
	EDMA_TEST_CMD_7,
	EDMA_TEST_CMD_MAX
};

void edma_transceive_test_init(void);
void edma_transceive_test_deinit(void);
void edma_transceive_test_run(int pairs);
void edma_transceive_test_stop(void);
int edma_transceive_test_exec(void *args);
void edma_transceive_test_trigger(char *cmd);

#endif /* __EDMA_TEST_H__ */
