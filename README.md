# mender-stm32l4a6-zephyr-example

[![CI Badge](https://github.com/joelguittet/mender-stm32l4a6-zephyr-example/workflows/ci/badge.svg)](https://github.com/joelguittet/mender-stm32l4a6-zephyr-example/actions)
[![Issues Badge](https://img.shields.io/github/issues/joelguittet/mender-stm32l4a6-zephyr-example)](https://github.com/joelguittet/mender-stm32l4a6-zephyr-example/issues)
[![License Badge](https://img.shields.io/github/license/joelguittet/mender-stm32l4a6-zephyr-example)](https://github.com/joelguittet/mender-stm32l4a6-zephyr-example/blob/master/LICENSE)

[Mender MCU client](https://github.com/joelguittet/mender-mcu-client) is an open source over-the-air (OTA) library updater for MCU devices. This demonstration project runs on STM32L4 hardware using Zephyr RTOS.


## Getting started

This project is used with a [NUCLEO-L4A6ZG](https://www.st.com/en/evaluation-tools/nucleo-l4a6zg.html) evaluation board and an Ethernet [W5500](https://www.wiznet.io/product-item/w5500) module available low cost at several places. The device tree overlay `nucleo_l4a6zg_firmware.overlay` specifies the wiring of the module, as illustrated on the following image, and it is possible to adapt it to your own hardware.

![NUCLEO-L4A6ZG and W5500 wiring](https://raw.githubusercontent.com/joelguittet/mender-stm32l4a6-zephyr-example/master/.github/docs/wiring.png)

The project is built using Zephyr RTOS v3.4.0 and Zephyr SDK >= v0.16.0. It depends on [cJSON](https://github.com/DaveGamble/cJSON). There is no other dependencies.

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
- `MENDER_CLIENT_UPDATE_POLL_INTERVAL` is the interval to check for new deployments.
- `MENDER_CLIENT_INVENTORY_REFRESH_INTERVAL` is the interval to publish inventory data.
- `MENDER_CLIENT_CONFIGURE_REFRESH_INTERVAL` is the interval to refresh device configuration.

Other settings are available in the Kconfig in sections "Mender General Options", "Mender Addons Options", "Mender Network Options", "Mender Scheduler Options", "Mender Storage Options" and "Example Configuration". You can also refer to the mender-mcu-client API and configuration keys.

Particularly, it is possible to activate the Device Troubleshoot add-on that will permit to display the Zephyr console of the device directly on the Mender interface as shown on the following screenshot.

![Troubleshoot console](https://raw.githubusercontent.com/joelguittet/mender-stm32l4a6-zephyr-example/master/.github/docs/troubleshoot.png)

In order to get the Device Troubleshoot add-on working, the following configuration keys must be defined in the `prj.conf` file:

```
CONFIG_HEAP_MEM_POOL_SIZE=1500
CONFIG_SHELL_BACKEND_SERIAL=n
CONFIG_SHELL_AUTOSTART=n
CONFIG_SHELL_STACK_SIZE=3072
```

Important note: due to W5500 support limitation in Zephyr, a single socket can be opened at a time. Impact is that when the Device Troubleshoot connection is established, other connection will fail, including the connections made by the applications. This is why until a solution is found the Device Troubleshoot add-on is not activated by default in this example.

### Building and flashing the application

The application relies on mcuboot and requires to build a signed binary file to be flashed on the evaluation board.

Use the following commands to build and flash mcuboot (please adapt the paths to your own installation):

```
west build -s $HOME/zephyrproject/bootloader/mcuboot/boot/zephyr -d build-mcuboot -b nucleo_l4a6zg -- -DDTC_OVERLAY_FILE=path/to/mender-stm32l4a6-zephyr-example/nucleo_l4a6zg_mcuboot.overlay -DCONFIG_BOOT_SWAP_USING_MOVE=y -DCONFIG_BOOT_MAX_IMG_SECTORS=256
west flash -d build-mcuboot
```

Use the following command to build, sign and flash the application (please adapt the paths to your own installation):

```
west build -b nucleo_l4a6zg path/to/mender-stm32l4a6-zephyr-example
$HOME/zephyrproject/bootloader/mcuboot/scripts/imgtool.py sign --key $HOME/zephyrproject/bootloader/mcuboot/root-rsa-2048.pem --header-size 0x200 -S 0x7E000 --align 8 --version $(head -n1 path/to/mender-stm32l4a6-zephyr-example/VERSION.txt) build/zephyr/zephyr.hex build/zephyr/zephyr-signed.hex
west flash --hex-file build/zephyr/zephyr-signed.hex
```

### Execution of the application

After flashing the application on the NUCLEO-L4A6ZG evaluation board and displaying logs, you should be able to see the following:

```
[00:00:00.012,000] <inf> eth_w5500: W5500 Initialized
*** Booting Zephyr OS build zephyr-v3.4.0 ***
[00:00:00.017,000] <inf> mender_stm32l4a6_zephyr_example: MAC address of the device '00:08:dc:01:02:03'
[00:00:00.027,000] <inf> mender_stm32l4a6_zephyr_example: Running project 'mender-stm32l4a6-zephyr-example' version '0.1'
[00:00:00.045,000] <inf> fs_nvs: 4 Sectors of 2048 bytes
[00:00:00.053,000] <inf> fs_nvs: alloc wra: 0, 7e8
[00:00:00.061,000] <inf> fs_nvs: data wra: 0, 0
[00:00:00.068,000] <inf> mender_stm32l4a6_zephyr_example: Mender client initialized
[00:00:00.078,000] <inf> mender_stm32l4a6_zephyr_example: Mender configure initialized
[00:00:00.089,000] <inf> mender_stm32l4a6_zephyr_example: Mender inventory initialized
[00:00:00.110,000] <inf> mender: CMAKE_SOURCE_DIR/mender-mcu-client/platform/board/zephyr/src/mender-storage.c (111): Authentication keys are not available
[00:00:00.126,000] <inf> mender: CMAKE_SOURCE_DIR/mender-mcu-client/core/src/mender-client.c (322): Generating authentication keys...
[00:00:05.051,000] <inf> mender_stm32l4a6_zephyr_example: Your address: 192.168.1.216
[00:00:05.061,000] <inf> mender_stm32l4a6_zephyr_example: Lease time: 43200 seconds
[00:00:05.071,000] <inf> mender_stm32l4a6_zephyr_example: Subnet: 255.255.255.0
[00:00:05.081,000] <inf> mender_stm32l4a6_zephyr_example: Router: 192.168.1.1
[00:03:50.215,000] <inf> mender: CMAKE_SOURCE_DIR/mender-mcu-client/platform/board/zephyr/src/mender-storage.c (187): OTA ID not available
[00:03:59.183,000] <err> mender: CMAKE_SOURCE_DIR/mender-mcu-client/core/src/mender-api.c (735): [401] Unauthorized: dev auth: unauthorized
[00:03:59.198,000] <inf> mender_stm32l4a6_zephyr_example: Mender client authentication failed (1/3)
```

Which means you now have generated authentication keys on the device. Generating is a bit long but authentication keys are stored in `storage_partition` of the MCU so it's done only the first time the device is flashed. You now have to accept your device on the mender interface. Once it is accepted on the mender interface the following will be displayed:

```
[00:04:59.233,000] <inf> mender_stm32l4a6_zephyr_example: Mender client authenticated
[00:05:03.588,000] <inf> mender: CMAKE_SOURCE_DIR/mender-mcu-client/core/src/mender-client.c (443): Checking for deployment...
[00:05:04.997,000] <inf> mender: CMAKE_SOURCE_DIR/mender-mcu-client/core/src/mender-client.c (451): No deployment available
```

Congratulation! Your device is connected to the mender server. Device type is `mender-stm32l4a6-zephyr-example` and the current software version is displayed.

### Create a new deployment

First retrieve [mender-artifact](https://docs.mender.io/downloads#mender-artifact) tool.

Change `VERSION.txt` file to `0.2`, rebuild and sign the firmware using the following commands. We previously used `hex` file because it is required to flash the device, but we now use `bin` file that is required for the mender-mcu-client:

```
west build -b nucleo_l4a6zg path/to/mender-stm32l4a6-zephyr-example
$HOME/zephyrproject/bootloader/mcuboot/scripts/imgtool.py sign --key $HOME/zephyrproject/bootloader/mcuboot/root-rsa-2048.pem --header-size 0x200 -S 0x7E000 --align 8 --version $(head -n1 path/to/mender-stm32l4a6-zephyr-example/VERSION.txt) build/zephyr/zephyr.bin build/zephyr/zephyr-signed.bin
```

Then create a new artifact using the following command line:

```
path/to/mender-artifact write rootfs-image --compression none --device-type mender-stm32l4a6-zephyr-example --artifact-name mender-stm32l4a6-zephyr-example-v$(head -n1 path/to/mender-stm32l4a6-zephyr-example/VERSION.txt) --output-path build/zephyr/mender-stm32l4a6-zephyr-example-v$(head -n1 path/to/mender-stm32l4a6-zephyr-example/VERSION.txt).mender --file build/zephyr/zephyr-signed.bin
```

Upload the artifact `mender-stm32l4a6-zephyr-example-v0.2.mender` to the mender server and create a new deployment.

The device checks for the new deployment, downloads the artifact and installs it on the `slot1_partition`. Then it reboots to apply the update:

```
[00:07:02.170,000] <inf> mender: CMAKE_SOURCE_DIR/mender-mcu-client/core/src/mender-client.c (443): Checking for deployment...
[00:07:03.608,000] <inf> mender: CMAKE_SOURCE_DIR/mender-mcu-client/core/src/mender-client.c (463): Downloading deployment artifact with id '121eb07a-1188-4316-a58d-b798ee60988-
[00:07:05.062,000] <inf> mender_stm32l4a6_zephyr_example: Deployment status is 'downloading'
[00:07:07.798,000] <inf> mender: CMAKE_SOURCE_DIR/mender-mcu-client/core/src/mender-artifact.c (380): Artifact has valid version
[00:07:07.817,000] <inf> mender: CMAKE_SOURCE_DIR/mender-mcu-client/platform/board/zephyr/src/mender-ota.c (42): Start flashing OTA artifact 'zephyr-signed.bin' with size 293224
[00:07:33.707,000] <inf> mender: CMAKE_SOURCE_DIR/mender-mcu-client/core/src/mender-client.c (473): Download done, installing artifact
[00:07:35.124,000] <inf> mender_stm32l4a6_zephyr_example: Deployment status is 'installing'
[00:09:15.191,000] <inf> mender_stm32l4a6_zephyr_example: Deployment status is 'rebooting'
[00:09:16.652,000] <inf> mender_stm32l4a6_zephyr_example: Device configuration received from the server
[00:09:18.039,000] <inf> mender_stm32l4a6_zephyr_example: Restarting system

...

[00:00:00.012,000] <inf> eth_w5500: W5500 Initialized
*** Booting Zephyr OS build zephyr-v3.4.0 ***
[00:00:00.017,000] <inf> mender_stm32l4a6_zephyr_example: MAC address of the device '00:08:dc:01:02:03'
[00:00:00.027,000] <inf> mender_stm32l4a6_zephyr_example: Running project 'mender-stm32l4a6-zephyr-example' version '0.2'
[00:00:00.045,000] <inf> fs_nvs: 4 Sectors of 2048 bytes
[00:00:00.052,000] <inf> fs_nvs: alloc wra: 1, 7d0
[00:00:00.060,000] <inf> fs_nvs: data wra: 1, 1f8
[00:00:00.067,000] <inf> mender_stm32l4a6_zephyr_example: Mender client initialized
[00:00:00.077,000] <inf> mender_stm32l4a6_zephyr_example: Mender configure initialized
[00:00:00.088,000] <inf> mender_stm32l4a6_zephyr_example: Mender inventory initialized
[00:00:08.051,000] <inf> mender_stm32l4a6_zephyr_example: Your address: 192.168.1.216
[00:00:08.061,000] <inf> mender_stm32l4a6_zephyr_example: Lease time: 43200 seconds
[00:00:08.071,000] <inf> mender_stm32l4a6_zephyr_example: Subnet: 255.255.255.0
[00:00:08.081,000] <inf> mender_stm32l4a6_zephyr_example: Router: 192.168.1.1
[00:00:19.024,000] <inf> mender_stm32l4a6_zephyr_example: Mender client authenticated
[00:00:19.035,000] <inf> mender: CMAKE_SOURCE_DIR/mender-mcu-client/platform/board/zephyr/src/mender-ota.c (146): Application has been mark valid and rollback canceled
[00:00:20.449,000] <inf> mender_stm32l4a6_zephyr_example: Deployment status is 'success'
[00:00:24.798,000] <inf> mender: CMAKE_SOURCE_DIR/mender-mcu-client/core/src/mender-client.c (443): Checking for deployment...
[00:00:26.258,000] <inf> mender: CMAKE_SOURCE_DIR/mender-mcu-client/core/src/mender-client.c (451): No deployment available
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

#### Using an other mender instance

The communication with the server is done using HTTPS. To get it working, the Root CA that is providing the server certificate should be integrated and registered in the application (see `tls_credential_add` in the `src/main.c` file). Format of the expected Root CA certificate is DER.
In this example we are using the `https://hosted.mender.io` server. While checking the details of the server certificate in your browser, you will see that is is provided by Amazon Root CA 1. Thus the Amazon Root CA 1 certificate `AmazonRootCA1.cer` retrieved at `https://www.amazontrust.com/repository` is integrated in the application.
