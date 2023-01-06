/*
 * SPDX-FileCopyrightText: 2022 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef PBLE_OVERRIDES_H
#define PBLE_OVERRIDES_H

#if !defined(PBLE_STATIC_ASSERT)
#define PBLE_STATIC_ASSERT(exp)	__extension__ _Static_assert(exp, #exp)
#endif

#if !defined(PBLE_ASSERT)
#define PBLE_ASSERT(exp)
#endif

#if !defined(PBLE_LOG_DEBUG)
#define PBLE_LOG_DEBUG(...)
#endif

#if !defined(PBLE_LOG_INFO)
#define PBLE_LOG_INFO(...)
#endif

#if !defined(PI_LOG_WARN)
#define PBLE_LOG_WARN(...)
#endif

#if !defined(PBLE_LOG_ERROR)
#define PBLE_LOG_ERROR(...)
#endif

#endif /* PBLE_OVERRIDES_H */
