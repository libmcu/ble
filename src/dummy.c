/*
 * SPDX-FileCopyrightText: 2022 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "pble/ble.h"

struct ble *dummy_ble_create(void)
{
	return 0;
}

void dummy_ble_destroy(struct ble *self)
{
	(void)self;
}
