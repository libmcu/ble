# SPDX-License-Identifier: MIT

ifneq ($(PBLE_ROOT),)
pble-basedir := $(PBLE_ROOT)/
endif

PBLE_SRCS := $(pble-basedir)src/common.c

ifeq ($(TARGET_PLATFORM), esp32)
PBLE_SRCS += $(pble-basedir)src/esp32.c
else ifeq ($(TARGET_PLATFORM), nrf52)
PBLE_SRCS += $(pble-basedir)src/nrf52.c
else ifeq ($(TARGET_PLATFORM), zephyr)
PBLE_SRCS += $(pble-basedir)src/zephyr.c
else
$(warning "No target specified. Building with dummy target")
PBLE_SRCS += $(pble-basedir)src/dummy.c
endif

PBLE_INCS := $(pble-basedir)include
