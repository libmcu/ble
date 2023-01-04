# SPDX-License-Identifier: MIT

ifneq ($(PBLE_ROOT),)
pble-basedir := $(PBLE_ROOT)/
endif

PBLE_SRCS := $(pble-basedir)src/common.c

ifeq ($(PBLE_TARGET_PLATFORM), dummy)
PBLE_SRCS += $(pble-basedir)src/dummy.c
else ifeq ($(PBLE_TARGET_PLATFORM), esp32)
PBLE_SRCS += $(pble-basedir)src/esp32.c
else ifeq ($(PBLE_TARGET_PLATFORM), nrf52)
PBLE_SRCS += $(pble-basedir)src/nrf52.c
else ifeq ($(PBLE_TARGET_PLATFORM), zephyr)
PBLE_SRCS += $(pble-basedir)src/zephyr.c
else
$(error "No target specified.")
endif

PBLE_INCS := $(pble-basedir)include
