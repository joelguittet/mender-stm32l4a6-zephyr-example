# @file      CMakeLists.txt
# @brief     mender-stm32l4a6-zephyr-example CMakeLists file
#
# Copyright joelguittet and mender-mcu-client contributors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

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

# Generate Root CA include files
generate_inc_file_for_target(app "src/AmazonRootCA1.der" "${ZEPHYR_BINARY_DIR}/include/generated/AmazonRootCA1.der.inc")
generate_inc_file_for_target(app "src/GoogleTrustServicesR4.der" "${ZEPHYR_BINARY_DIR}/include/generated/GoogleTrustServicesR4.der.inc")
