/*
 * SPDX-FileCopyrightText: 2022 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: MIT
 */

#pragma push_macro("BLE_GAP_EVT_CONNECTED")
#pragma push_macro("BLE_GAP_EVT_DISCONNECTED")
#define BLE_GAP_EVT_CONNECTED		LIBMCU_BLE_GAP_EVT_CONNECTED
#define BLE_GAP_EVT_DISCONNECTED	LIBMCU_BLE_GAP_EVT_DISCONNECTED
#include "pble/ble.h"
#pragma pop_macro("BLE_GAP_EVT_DISCONNECTED")
#pragma pop_macro("BLE_GAP_EVT_CONNECTED")
#include "pble/ble_overrides.h"

#include <errno.h>
#include <string.h>

#include "ble.h"
#include "nrf_sdh.h"
#include "nrf_sdh_ble.h"
#include "ble_advertising.h"

PBLE_STATIC_ASSERT(BLE_GAP_EVT_MAX < UINT8_MAX);

#define BLE_PRIORITY			3
#define ADV_INTERVAL_UNIT_THOUSANDTH	625

enum ble_tag {
	CONN_CFG_TAG			= 0,
	CONN_TAG			= 1,
};

struct ble {
	struct ble_api api;

	ble_event_callback_t gap_event_callback;
	ble_event_callback_t gatt_event_callback;

	struct {
		ble_advertising_t handle;

		enum ble_adv_mode mode;
		uint16_t min_ms;
		uint16_t max_ms;
		uint32_t duration_ms;

		struct ble_adv_payload payload;
		struct ble_adv_payload scan_response;
	} adv;

	uint8_t addr_type;
	uint8_t addr[BLE_ADDR_LEN];
	uint16_t connection_handle;
};

static struct ble *onair;
static int adv_start(struct ble *self);
static int adv_stop(struct ble *self);

static void on_ble_events(ble_evt_t const * p_ble_evt, void * p_context)
{
}

static void on_error(uint32_t error_code)
{
}

static void on_adv_event(ble_adv_evt_t ble_adv_evt)
{
	switch (ble_adv_evt) {
	case BLE_ADV_EVT_FAST:
		break;
	case BLE_ADV_EVT_IDLE:
		break;
        default:
		break;
	}
}

static int adv_set_interval(struct ble *self, uint16_t min_ms, uint16_t max_ms)
{
	PBLE_ASSERT(min_ms >= BLE_ADV_MIN_INTERVAL_MS &&
			max_ms <= BLE_ADV_MAX_INTERVAL_MS);

	self->adv.min_ms = min_ms;
	self->adv.max_ms = max_ms;

	return 0;
}

static int adv_set_duration(struct ble *self, uint32_t msec)
{
	self->adv.duration_ms = msec;
	return 0;
}

static void register_gap_event_callback(struct ble *self,
					ble_event_callback_t cb)
{
	self->gap_event_callback = cb;
}

static int adv_set_payload(struct ble *self,
			   const struct ble_adv_payload *payload)
{
	memcpy(&self->adv.payload, payload, sizeof(*payload));
	return 0;
}

static int adv_set_scan_response(struct ble *self,
				 const struct ble_adv_payload *payload)
{
	memcpy(&self->adv.scan_response, payload, sizeof(*payload));
	return 0;
}

static int initialize_advertising(struct ble *self,
		const ble_adv_modes_config_t *cfg)
{
	ble_advertising_t *handle = &self->adv.handle;

	memset(handle, 0, sizeof(*handle));

	handle->adv_mode_current = BLE_ADV_MODE_IDLE;
	handle->adv_modes_config = *cfg;
	handle->conn_cfg_tag = CONN_CFG_TAG;
	handle->evt_handler = on_adv_event;
	handle->error_handler = on_error;
	handle->current_slave_link_conn_handle = BLE_CONN_HANDLE_INVALID;
	handle->p_adv_data = &handle->adv_data;

	if (!handle->initialized) {
		handle->adv_handle = BLE_GAP_ADV_SET_HANDLE_NOT_SET;
	}

	/* advertising data */
	handle->adv_data.adv_data.p_data = handle->enc_advdata[0];
	memcpy(handle->adv_data.adv_data.p_data, self->adv.payload.payload,
			self->adv.payload.index);
	handle->adv_data.adv_data.len = self->adv.payload.index;

	/* scan response data */
	handle->adv_data.scan_rsp_data.p_data = handle->enc_scan_rsp_data[0];
	memcpy(handle->adv_data.scan_rsp_data.p_data,
			self->adv.scan_response.payload,
			self->adv.scan_response.index);
	handle->adv_data.scan_rsp_data.len = self->adv.scan_response.index;

	handle->adv_params.primary_phy = BLE_GAP_PHY_1MBPS;
	handle->adv_params.duration =
		handle->adv_modes_config.ble_adv_fast_timeout;
	handle->adv_params.properties.type =
		BLE_GAP_ADV_TYPE_CONNECTABLE_SCANNABLE_UNDIRECTED;
	handle->adv_params.p_peer_addr = NULL;
	handle->adv_params.filter_policy = BLE_GAP_ADV_FP_ANY;
	handle->adv_params.interval =
		handle->adv_modes_config.ble_adv_fast_interval;

	switch (self->adv.mode) {
	case BLE_ADV_DIRECT_IND:
		handle->adv_params.properties.type =
			BLE_GAP_ADV_TYPE_CONNECTABLE_NONSCANNABLE_DIRECTED;
		break;
	case BLE_ADV_NONCONN_IND:
		handle->adv_params.properties.type =
			BLE_GAP_ADV_TYPE_NONCONNECTABLE_NONSCANNABLE_UNDIRECTED;
		break;
	case BLE_ADV_SCAN_IND:
		handle->adv_params.properties.type =
			BLE_GAP_ADV_TYPE_NONCONNECTABLE_SCANNABLE_UNDIRECTED;
		break;
	default:
	case BLE_ADV_IND:
		break;
	}

	uint32_t err = sd_ble_gap_adv_set_configure(&handle->adv_handle, NULL,
				&handle->adv_params);
	if (err != NRF_SUCCESS) {
		PBLE_LOG_ERROR("advertising configuration error: %x", err);
		return -EINVAL;
	}

	handle->initialized = true;

	return 0;
}

static int adv_start(struct ble *self)
{
	if (!nrf_sdh_is_enabled()) {
		return -ENODEV;
	}

	ble_adv_modes_config_t cfg = {
		.ble_adv_fast_enabled = true,
		.ble_adv_fast_interval = self->adv.min_ms * 1000UL /
			ADV_INTERVAL_UNIT_THOUSANDTH,
		.ble_adv_fast_timeout = self->adv.duration_ms / 10,
	};
 
	if (initialize_advertising(self, &cfg)) {
		PBLE_LOG_ERROR("advertising init failure");
		return -EINVAL;
	}

	ble_advertising_conn_cfg_tag_set(&self->adv.handle, CONN_TAG);

	uint32_t err = ble_advertising_start(&self->adv.handle,
			BLE_ADV_MODE_FAST);
	if (err != NRF_SUCCESS) {
		PBLE_LOG_ERROR("advertising start failure: %x", err);
		return -EFAULT;
	}

	return 0;
}

static int adv_stop(struct ble *self)
{
	return 0;
}

static int adv_init(struct ble *self, enum ble_adv_mode mode)
{
	memset(&self->adv, 0, sizeof(self->adv));

	self->adv.mode = mode;

	self->adv.min_ms = 180;
	self->adv.max_ms = 180;
	self->adv.duration_ms = 180000;

	return 0;
}

static enum ble_device_addr get_device_address(struct ble *iface,
		uint8_t addr[BLE_ADDR_LEN])
{
	memcpy(addr, iface->addr, BLE_ADDR_LEN);
	return iface->addr_type;
}

static int initialize(struct ble *iface)
{
	(void)iface;
	uint32_t ram_start = 0;

	if (nrf_sdh_enable_request() != NRF_SUCCESS) {
		PBLE_LOG_ERROR("softdevice not ready");
		return -1;
	}
	if (nrf_sdh_ble_default_cfg_set(CONN_TAG, &ram_start) != NRF_SUCCESS) {
		PBLE_LOG_ERROR("invalid softdevice configuration");
		return -2;
	}
	if (nrf_sdh_ble_enable(&ram_start) != NRF_SUCCESS) {
		PBLE_LOG_ERROR("BLE start failure. Check RAM start address: %x",
				ram_start);
		return -3;
	}

	NRF_SDH_BLE_OBSERVER(event_handle, BLE_PRIORITY, on_ble_events, NULL);

	ble_gap_conn_sec_mode_t sec_mode;
	BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);
	if (sd_ble_gap_device_name_set(&sec_mode,
			(const uint8_t *)BLE_DEFAULT_DEVICE_NAME,
			sizeof(BLE_DEFAULT_DEVICE_NAME)-1) != NRF_SUCCESS) {
		PBLE_LOG_ERROR("can not set device name");
	}

	return 0;
}

static int enable_device(struct ble *self,
		enum ble_device_addr addr_type, uint8_t addr[BLE_ADDR_LEN])
{
	if (!onair) {
		self->addr_type = addr_type;
		if (addr) {
			memcpy(self->addr, addr, sizeof(self->addr));
		}

		initialize(self);
		onair = self;
	}

	return 0;
}

static int disable_device(struct ble *self)
{
	if (!nrf_sdh_is_enabled()) {
		goto out;
	}

out:
	onair = NULL;
	return 0;
}

struct ble *nrf52_ble_create(void)
{
	static struct ble self = {
		.api = {
			.enable = enable_device,
			.disable = disable_device,
			.register_gap_event_callback = register_gap_event_callback,
			.get_device_address = get_device_address,

			.adv_init = adv_init,
			.adv_set_interval = adv_set_interval,
			.adv_set_duration = adv_set_duration,
			.adv_set_payload = adv_set_payload,
			.adv_set_scan_response = adv_set_scan_response,
			.adv_start = adv_start,
			.adv_stop = adv_stop,
		},
	};

	return &self;
}

void nrf52_ble_destroy(struct ble *self)
{
	memset(self, 0, sizeof(*self));
}
