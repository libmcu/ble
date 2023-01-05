# SPDX-License-Identifier: MIT

list(APPEND PBLE_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/src/common.c)

if(TARGET_PLATFORM STREQUAL esp32)
	list(APPEND PBLE_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/src/esp32.c)
elseif(TARGET_PLATFORM STREQUAL nrf52)
	list(APPEND PBLE_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/src/nrf52.c)
elseif(TARGET_PLATFORM STREQUAL zephyr)
	list(APPEND PBLE_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/src/zephyr.c)
else()
	message(WARNING "No target specified. Building with dummy target.")
	list(APPEND PBLE_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/src/dummy.c)
endif()

list(APPEND PBLE_INCS ${CMAKE_CURRENT_SOURCE_DIR}/include)
