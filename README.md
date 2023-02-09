# mender-stm32l4a6-zephyr-example

[![CI Badge](https://github.com/joelguittet/mender-stm32l4a6-zephyr-example/workflows/ci/badge.svg)](https://github.com/joelguittet/mender-stm32l4a6-zephyr-example/actions)
[![Issues Badge](https://img.shields.io/github/issues/joelguittet/mender-stm32l4a6-zephyr-example)](https://github.com/joelguittet/mender-stm32l4a6-zephyr-example/issues)
[![License Badge](https://img.shields.io/github/license/joelguittet/mender-stm32l4a6-zephyr-example)](https://github.com/joelguittet/mender-stm32l4a6-zephyr-example/blob/master/LICENSE)

[Mender MCU client](https://github.com/joelguittet/mender-mcu-client) is an open source over-the-air (OTA) library updater for MCU devices. This demonstration project runs on STM32L4 hardware using Zephyr RTOS.


## Getting started

This project is used with a [NUCLEO-L4A6ZG](https://www.st.com/en/evaluation-tools/nucleo-l4a6zg.html) evaluation board and an Ethernet [W5500](https://www.wiznet.io/product-item/w5500) module available low cost at several places. The device tree overlay `nucleo_l4a6zg_firmware.overlay` specifies the wiring of the module, and it is possible to adapt it to your own hardware.

The project is built using Zephyr RTOS v3.3.0-rc2. It depends on [cJSON](https://github.com/DaveGamble/cJSON). There is no other dependencies.

To start using Mender, we recommend that you begin with the Getting started section in [the Mender documentation](https://docs.mender.io).

To start using Zephyr, we recommend that you begin with the Getting started section in [the Zephyr documentation](https://docs.zephyrproject.org/latest/develop/getting_started/index.html). It is highly recommended to be familiar with Zephyr environment and tools to use this example.

### Open the project

Clone the project and retrieve mender-mcu-client and cJSON submodules using `git submodule update --init --recursive`.

Then open a new Zephyr virtual environment terminal.

### Configuration of the application

The example application should first be configured to set at least:
- `MENDER_SERVER_TENANT_TOKEN` to set the Tenant Token of your account on "https://hosted.mender.io" server.

You may want to customize few interesting settings:
- `MENDER_SERVER_HOST` if using your own Mender server instance. Tenant Token is not required in this case.
- `MENDER_CLIENT_AUTHENTICATION_POLL_INTERVAL` is the interval to retry authentication on the mender server.
- `MENDER_CLIENT_INVENTORY_POLL_INTERVAL` is the interval to publish inventory data.
- `MENDER_CLIENT_UPDATE_POLL_INTERVAL` is the interval to check for new deployments.

Other settings are available in the Kconfig in sections "Example Configuration" and "Mender client Configuration". You can also refer to the mender-mcu-client API.

### Building and flashing the application

The application relies on mcuboot and requires to build a signed binary file to be flashed on the evaluation board.

Use the following commands to build and flash mcuboot (please adapt the paths to your own installation):

```
west build -p always -s $HOME/zephyrproject/bootloader/mcuboot/boot/zephyr -d build-mcuboot -b nucleo_l4a6zg -- -DDTC_OVERLAY_FILE=path/to/mender-stm32l4a6-zephyr-example/nucleo_l4a6zg_mcuboot.overlay -DCONFIG_BOOT_SWAP_USING_MOVE=y -DCONFIG_BOOT_MAX_IMG_SECTORS=256
west flash -d build-mcuboot
```

Use the following command to build, sign and flash the application (please adapt the paths to your own installation):

```
west build -p always -b nucleo_l4a6zg path/to/mender-stm32l4a6-zephyr-example
$HOME/zephyrproject/bootloader/mcuboot/scripts/imgtool.py sign --key $HOME/zephyrproject/bootloader/mcuboot/root-rsa-2048.pem --header-size 0x200 -S 0x7E000 --align 8 --version $(head -n1 path/to/mender-stm32l4a6-zephyr-example/VERSION) build/zephyr/zephyr.hex build/zephyr/zephyr-signed.hex
west flash --hex-file build/zephyr/zephyr-signed.hex
```

### Execution of the application

After flashing the application on the NUCLEO-L4A6ZG evaluation board and displaying logs, you should be able to see the following:

```
*** Booting Zephyr OS build v3.3.0-rc1 ***
I: Starting bootloader
I: Primary image: magic=unset, swap_type=0x1, copy_done=0x3, image_ok=0x3
I: Secondary image: magic=unset, swap_type=0x1, copy_done=0x3, image_ok=0x3
I: Boot source: none
I: Swap type: none
I: Bootloader chainload address offset: 0xe000
I: Jumping to the first image slot


[00:00:00.012,000] <inf> eth_w5500: W5500 Initialized
*** Booting Zephyr OS build v3.3.0-rc1 ***
[00:00:00.027,000] <inf> mender_stm32l4a6_zephyr_example: Running project 'mender-stm32l4a6-zephyr-example' version '0.1'
[00:00:00.044,000] <inf> fs_nvs: 4 Sectors of 2048 bytes
[00:00:00.052,000] <inf> fs_nvs: alloc wra: 0, 7e8
[00:00:00.059,000] <inf> fs_nvs: data wra: 0, 0
[00:00:00.068,000] <inf> mender_stm32l4a6_zephyr_example: Mender client initialized
[00:00:00.078,000] <inf> mender: CMAKE_SOURCE_DIR/mender-mcu-client/platform/board/zephyr/src/mender-storage.c (107): Authentication keys are not available
[00:00:00.094,000] <inf> mender: CMAKE_SOURCE_DIR/mender-mcu-client/core/src/mender-client.c (377): Generating authentication keys...
[00:00:04.089,000] <inf> mender_stm32l4a6_zephyr_example: Your address: 192.168.1.243
[00:00:04.099,000] <inf> mender_stm32l4a6_zephyr_example: Lease time: 86400 seconds
[00:00:04.109,000] <inf> mender_stm32l4a6_zephyr_example: Subnet: 255.255.255.0
[00:00:04.119,000] <inf> mender_stm32l4a6_zephyr_example: Router: 192.168.1.1
[00:02:16.758,000] <inf> mender: CMAKE_SOURCE_DIR/mender-mcu-client/platform/board/zephyr/src/mender-storage.c (170): OTA ID not available
[00:02:23.673,000] <err> mender: CMAKE_SOURCE_DIR/mender-mcu-client/core/src/mender-api.c (686): [401] Unauthorized: dev auth: unauthorized
[00:02:23.689,000] <inf> mender_stm32l4a6_zephyr_example: Mender client authentication failed (1/3)
```

Which means you now have generated authentication keys on the device. Generating is a bit long but authentication keys are stored in `storage_partition` of the MCU so it's done only the first time the device is flashed. You now have to accept your device on the mender interface. Once it is accepted on the mender interface the following will be displayed:

```
[00:03:23.711,000] <inf> mender_stm32l4a6_zephyr_example: Mender client authenticated
[00:03:26.563,000] <inf> mender: CMAKE_SOURCE_DIR/mender-mcu-client/core/src/mender-client.c (533): No deployment available
```

Congratulation! Your device is connected to the mender server. Device type is `mender-stm32l4a6-zephyr-example` and the current software version is displayed.

### Create a new deployment

First retrieve [mender-artifact](https://docs.mender.io/downloads#mender-artifact) tool.

Change VERSION to `0.2`, rebuild and sign the firmware using the following commands. We previously used `hex` file because it is required to flash the device, but we now use `bin` file that is required for the mender-mcu-client:

```
west build -b nucleo_l4a6zg path/to/mender-stm32l4a6-zephyr-example
$HOME/zephyrproject/bootloader/mcuboot/scripts/imgtool.py sign --key $HOME/zephyrproject/bootloader/mcuboot/root-rsa-2048.pem --header-size 0x200 -S 0x7E000 --align 8 --version $(head -n1 path/to/mender-stm32l4a6-zephyr-example/VERSION) build/zephyr/zephyr.bin build/zephyr/zephyr-signed.bin
```

Then create a new artifact using the following command line:

```
path/to/mender-artifact write rootfs-image --compression none --device-type mender-stm32l4a6-zephyr-example --artifact-name mender-stm32l4a6-zephyr-example-v$(head -n1 path/to/mender-stm32l4a6-zephyr-example/VERSION) --output-path build/zephyr/mender-stm32l4a6-zephyr-example-v$(head -n1 path/to/mender-stm32l4a6-zephyr-example/VERSION).mender --file build/zephyr/zephyr-signed.bin
```

Upload the artifact `mender-stm32l4a6-zephyr-example-v0.2.mender` to the mender server and create a new deployment.

The device checks for the new deployment, downloads the artifact and installs it on the `slot1_partition`. Then it reboots to apply the update:

```
[00:14:38.451,000] <inf> mender: CMAKE_SOURCE_DIR/mender-mcu-client/core/src/mender-client.c (507): Downloading deployment artifact with id 'e2c957c7-847f-4a15-a247-4beb918eac9-
[00:14:39.865,000] <inf> mender_stm32l4a6_zephyr_example: Deployment status is 'downloading'
[00:14:51.764,000] <inf> mender: CMAKE_SOURCE_DIR/mender-mcu-client/platform/board/zephyr/src/mender-ota.c (40): Start flashing OTA artifact 'zephyr-signed.bin' with size 280360
[00:15:17.223,000] <inf> mender: CMAKE_SOURCE_DIR/mender-mcu-client/core/src/mender-client.c (513): Download done, installing artifact
[00:15:18.761,000] <inf> mender_stm32l4a6_zephyr_example: Deployment status is 'installing'
[00:15:30.176,000] <inf> mender_stm32l4a6_zephyr_example: Deployment status is 'rebooting'
[00:15:30.187,000] <inf> mender_stm32l4a6_zephyr_example: Restarting system
uart:~$ *** Booting Zephyr OS build v3.3.0-rc1 ***
I: Starting bootloader
I: Primary image: magic=unset, swap_type=0x1, copy_done=0x3, image_ok=0x3
I: Secondary image: magic=good, swap_type=0x2, copy_done=0x3, image_ok=0x3
I: Boot source: none
I: Swap type: test
I: Starting swap using move algorithm.
I: Bootloader chainload address offset: 0xe000
I: Jumping to the first image slot


[00:00:00.012,000] <inf> eth_w5500: W5500 Initialized
*** Booting Zephyr OS build v3.3.0-rc1 ***
[00:00:00.027,000] <inf> mender_stm32l4a6_zephyr_example: Running project 'mender-stm32l4a6-zephyr-example' version '0.2'
[00:00:00.043,000] <inf> fs_nvs: 4 Sectors of 2048 bytes
[00:00:00.051,000] <inf> fs_nvs: alloc wra: 1, 7d0
[00:00:00.058,000] <inf> fs_nvs: data wra: 1, 1f8
[00:00:00.067,000] <inf> mender_stm32l4a6_zephyr_example: Mender client initialized
[00:00:06.062,000] <inf> mender_stm32l4a6_zephyr_example: Your address: 192.168.1.243
[00:00:06.072,000] <inf> mender_stm32l4a6_zephyr_example: Lease time: 86400 seconds
[00:00:06.083,000] <inf> mender_stm32l4a6_zephyr_example: Subnet: 255.255.255.0
[00:00:06.092,000] <inf> mender_stm32l4a6_zephyr_example: Router: 192.168.1.1
[00:00:16.995,000] <inf> mender_stm32l4a6_zephyr_example: Mender client authenticated
[00:00:17.005,000] <inf> mender: CMAKE_SOURCE_DIR/mender-mcu-client/platform/board/zephyr/src/mender-ota.c (125): Application has been mark valid and rollback canceled
[00:00:18.395,000] <inf> mender_stm32l4a6_zephyr_example: Deployment status is 'success'
[00:00:21.160,000] <inf> mender: CMAKE_SOURCE_DIR/mender-mcu-client/core/src/mender-client.c (533): No deployment available
```

Congratulation! You have updated the device. Mender server displays the success of the deployment.

### Failure or wanted rollback

In case of failure to connect and authenticate to the server the current example application performs a rollback to the previous release.
You can customize the behavior of the example application to add your own checks and perform the rollback in case the tests fail.

### Using an other zephyr evaluation board

The zephyr integration into the mender-mcu-client is generic and it is not limited to STM32 MCUs.
Several points discussed below should be taken into consideration to use an other hardware, including evaluation boards with other MCU families.

#### Flash sectors size

The flash sector size is an important criteria to select an evaluation board. The STM32L4A6ZG MCU has 2KB sectors, which is very convenient to define a custom layout. Other MCUs have variable sectors size and it can be very difficult to choose the partitions.

What remains important is that partitions are aligned on sectors. Moreover `slot0_partition` and `slot1_partition` must have the same size.

#### Signing the image

Signing the image requires to provide the size of the of `slot0_partition` and `slot1_partition`. In the current example the size is `0x7E000`. You should adapt this if you have a different flash layout.

#### Configuration of the storage partition

The storage partition should be at least 4KB and must contains at least 3 sectors. In the current example the size of the `storage_partition` is 8KB and the configuration is set with `CONFIG_MENDER_STORAGE_SECTOR_COUNT=4` (4 sectors of 2KB). You should adapt this if you have a different flash layout.

#### Using an other network interface

The example is currently using a W5500 module connected to the NUCLEO-L4A6ZG evaluation board according to the device tree overlay. It is possible to use an other module depending of your own hardware. The mender-mcu-client expect to have a TCP-IP interface but it is not constraint by the physical hardware.
Note that the W5500 driver does not support the local-mac-address today. A patch is done in this example in `src/main.c` file line 175 and following. If you want to use another hardware you should be able to remove this patch.
