/*
 * SPDX-FileCopyrightText: 2022 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "pble/ble.h"

struct ble *ble_create_default(void)
{
	return 0;
}

void ble_destroy_default(struct ble *self)
{
	(void)self;
}
