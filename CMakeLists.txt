# @file      CMakeLists.txt
# @brief     mender-stm32l4a6-zephyr-example CMakeLists file
#
# MIT License
#
# Copyright (c) 2022-2023 joelguittet and mender-mcu-client contributors
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

cmake_minimum_required(VERSION 3.20.0)

# Extra components
list(APPEND ZEPHYR_EXTRA_MODULES
    "${CMAKE_CURRENT_SOURCE_DIR}/components/cJSON"
    "${CMAKE_CURRENT_SOURCE_DIR}/components/msgpack-c"
    "${CMAKE_CURRENT_SOURCE_DIR}/components/mender-mcu-client"
)

# Kconfig options
set(KCONFIG_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/Kconfig")

# Device tree overlay file
set(DTC_OVERLAY_FILE "${CMAKE_CURRENT_SOURCE_DIR}/nucleo_l4a6zg_firmware.overlay")

# Declare project
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(mender-stm32l4a6-zephyr-example)

# Sources
target_sources(app PRIVATE "src/main.c")

# Definitions
target_compile_definitions(app PRIVATE PROJECT_NAME="mender-stm32l4a6-zephyr-example")

# Generate Root CA include file
generate_inc_file_for_target(app "src/AmazonRootCA1.cer" "${ZEPHYR_BINARY_DIR}/include/generated/AmazonRootCA1.cer.inc")
