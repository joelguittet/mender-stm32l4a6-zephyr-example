# @file      CMakeLists.txt
# @brief     msgpack-c module CMakeLists file
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

if(CONFIG_MSGPACK_C)
    zephyr_include_directories("${CMAKE_CURRENT_LIST_DIR}/../include")
    zephyr_include_directories("${CMAKE_CURRENT_LIST_DIR}/../include/msgpack")
    zephyr_include_directories("${CMAKE_CURRENT_LIST_DIR}/../msgpack-c/include")
    zephyr_library()
    zephyr_library_sources(
        "${CMAKE_CURRENT_LIST_DIR}/../msgpack-c/src/objectc.c"
        "${CMAKE_CURRENT_LIST_DIR}/../msgpack-c/src/unpack.c"
        "${CMAKE_CURRENT_LIST_DIR}/../msgpack-c/src/version.c"
        "${CMAKE_CURRENT_LIST_DIR}/../msgpack-c/src/vrefbuffer.c"
        "${CMAKE_CURRENT_LIST_DIR}/../msgpack-c/src/zone.c"
    )
endif()
