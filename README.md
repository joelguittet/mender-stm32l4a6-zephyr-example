# mender-stm32l4a6-zephyr-example

[![CI Badge](https://github.com/joelguittet/mender-stm32l4a6-zephyr-example/workflows/ci/badge.svg)](https://github.com/joelguittet/mender-stm32l4a6-zephyr-example/actions)
[![Issues Badge](https://img.shields.io/github/issues/joelguittet/mender-stm32l4a6-zephyr-example)](https://github.com/joelguittet/mender-stm32l4a6-zephyr-example/issues)
[![License Badge](https://img.shields.io/github/license/joelguittet/mender-stm32l4a6-zephyr-example)](https://github.com/joelguittet/mender-stm32l4a6-zephyr-example/blob/master/LICENSE)

[Mender MCU client](https://github.com/joelguittet/mender-mcu-client) is an open source over-the-air (OTA) library updater for MCU devices. This demonstration project runs on STM32L4 hardware using Zephyr RTOS.


## Getting started

This project is used with a [NUCLEO-L4A6ZG](https://www.st.com/en/evaluation-tools/nucleo-l4a6zg.html) evaluation board and an Ethernet [W5500](https://www.wiznet.io/product-item/w5500) module available low cost at several places. The device tree overlay `nucleo_l4a6zg_firmware.overlay` specifies the wiring of the module, as illustrated on the following image, and it is possible to adapt it to your own hardware.

![NUCLEO-L4A6ZG and W5500 wiring](https://raw.githubusercontent.com/joelguittet/mender-stm32l4a6-zephyr-example/master/.github/docs/wiring.png)

The project is built using Zephyr RTOS v3.7.0 and Zephyr SDK >= v0.16.0. It depends on [cJSON](https://github.com/DaveGamble/cJSON). There is no other dependencies.

To start using Mender, we recommend that you begin with the Getting started section in [the Mender documentation](https://docs.mender.io).

To start using Zephyr, we recommend that you begin with the Getting started section in [the Zephyr documentation](https://docs.zephyrproject.org/latest/develop/getting_started/index.html). It is highly recommended to be familiar with Zephyr environment and tools to use this example.

### Open the project

Clone the project and retrieve submodules using `git submodule update --init --recursive`.

Then open a new Zephyr virtual environment terminal.

### Configuration of the application

The example application should first be configured to set at least:
- `CONFIG_MENDER_SERVER_TENANT_TOKEN` to set the Tenant Token of your account on "https://hosted.mender.io" server.

You may want to customize few interesting settings:
- `CONFIG_MENDER_SERVER_HOST` if using your own Mender server instance. Tenant Token is not required in this case.
- `CONFIG_MENDER_CLIENT_AUTHENTICATION_POLL_INTERVAL` is the interval to retry authentication on the mender server.
- `CONFIG_MENDER_CLIENT_UPDATE_POLL_INTERVAL` is the interval to check for new deployments.
- `CONFIG_MENDER_CLIENT_INVENTORY_REFRESH_INTERVAL` is the interval to publish inventory data.
- `CONFIG_MENDER_CLIENT_CONFIGURE_REFRESH_INTERVAL` is the interval to refresh device configuration.

Other settings are available in the Kconfig. You can also refer to the mender-mcu-client API and configuration keys.

Particularly, it is possible to activate the Device Troubleshoot add-on that will permit to display the Zephyr console of the device directly on the Mender interface as shown on the following screenshot.

![Troubleshoot console](https://raw.githubusercontent.com/joelguittet/mender-stm32l4a6-zephyr-example/master/.github/docs/troubleshoot.png)

In order to get the Device Troubleshoot add-on working, the following configuration keys must be defined in the `prj.conf` file:

```
CONFIG_HEAP_MEM_POOL_SIZE=1500
CONFIG_SHELL_BACKEND_SERIAL=n
CONFIG_SHELL_AUTOSTART=n
CONFIG_SHELL_STACK_SIZE=3072
```

### Building and flashing the application

The application relies on mcuboot and requires to build a signed binary file to be flashed on the evaluation board.

Use the following commands to build and flash mcuboot (please adapt the paths to your own installation):

```
west build -s $HOME/zephyrproject/bootloader/mcuboot/boot/zephyr -d build-mcuboot -b nucleo_l4a6zg -- -DDTC_OVERLAY_FILE=path/to/mender-stm32l4a6-zephyr-example/nucleo_l4a6zg_mcuboot.overlay -DCONFIG_BOOT_SWAP_USING_SCRATCH=y -DCONFIG_BOOT_MAX_IMG_SECTORS=256
west flash -d build-mcuboot
```

Use the following command to build, sign and flash the application (please adapt the paths to your own installation):

```
west build -b nucleo_l4a6zg path/to/mender-stm32l4a6-zephyr-example
west flash --hex-file build/zephyr/zephyr.signed.hex
```

### Execution of the application

After flashing the application on the NUCLEO-L4A6ZG evaluation board and displaying logs, you should be able to see the following:

```
*** Booting MCUboot v2.1.0-rc1-4-g898a1ca64a75 ***
*** Using Zephyr OS build v3.6.0-7116-g40810983ead2 ***
I: Starting bootloader
I: Primary image: magic=unset, swap_type=0x1, copy_done=0x3, image_ok=0x3
I: Scratch: magic=unset, swap_type=0x1, copy_done=0x3, image_ok=0x3
I: Boot source: primary slot
I: Image index: 0, Swap type: none
I: Bootloader chainload address offset: 0xe000
I: Jumping to the first image slot


[00:00:02.016,000] <inf> eth_w5500: eth_w5500@0: Link up
[00:00:02.033,000] <inf> mender_stm32l4a6_zephyr_example: IPv4 address: 192.168.1.216
[00:00:02.044,000] <inf> mender_stm32l4a6_zephyr_example: Lease time: 43200 seconds
[00:00:02.054,000] <inf> mender_stm32l4a6_zephyr_example: Subnet: 255.255.255.0
[00:00:02.064,000] <inf> mender_stm32l4a6_zephyr_example: Router: 192.168.1.1
[00:00:02.074,000] <inf> mender_stm32l4a6_zephyr_example: MAC address of the device '00:08:dc:01:02:03'
[00:00:02.085,000] <inf> mender_stm32l4a6_zephyr_example: Running project 'mender-stm32l4a6-zephyr-example' version '0.1.0'
[00:00:02.104,000] <inf> mender_stm32l4a6_zephyr_example: Mender client initialized
[00:00:02.114,000] <inf> mender_stm32l4a6_zephyr_example: Mender client registered hello-world module
[00:00:02.126,000] <inf> mender: CMAKE_SOURCE_DIR/components/mender-mcu-client/platform/storage/zephyr/nvs/src/mender-storage.c (242): Device configuration not available
[00:00:02.144,000] <inf> mender_stm32l4a6_zephyr_example: Mender configure add-on registered
[00:00:02.155,000] <inf> mender_stm32l4a6_zephyr_example: Mender inventory add-on registered
[00:00:02.166,000] <inf> mender_stm32l4a6_zephyr_example: Device configuration retrieved
[00:00:02.177,000] <inf> mender: CMAKE_SOURCE_DIR/components/mender-mcu-client/platform/storage/zephyr/nvs/src/mender-storage.c (110): Authentication keys are not available
[00:00:02.195,000] <inf> mender: CMAKE_SOURCE_DIR/components/mender-mcu-client/platform/tls/generic/mbedtls/src/mender-tls.c (126): Generating authentication keys...
[00:04:54.037,000] <inf> mender: CMAKE_SOURCE_DIR/components/mender-mcu-client/platform/storage/zephyr/nvs/src/mender-storage.c (182): Deployment data not available
[00:04:54.054,000] <inf> mender_stm32l4a6_zephyr_example: Mender client connect network
[00:05:04.953,000] <err> mender: CMAKE_SOURCE_DIR/components/mender-mcu-client/core/src/mender-api.c (900): [401] Unauthorized: Unauthorized
[00:05:04.969,000] <inf> mender_stm32l4a6_zephyr_example: Mender client authentication failed
[00:05:04.980,000] <inf> mender_stm32l4a6_zephyr_example: Mender client released network
```

Which means you now have generated authentication keys on the device. Generating is a bit long but authentication keys are stored in `storage_partition` of the MCU so it's done only the first time the device is flashed. You now have to accept your device on the mender interface. Once it is accepted on the mender interface the following will be displayed:

```
[00:10:02.176,000] <inf> mender_stm32l4a6_zephyr_example: Mender client connect network
[00:10:13.205,000] <inf> mender_stm32l4a6_zephyr_example: Mender client authenticated
[00:10:13.215,000] <inf> mender: CMAKE_SOURCE_DIR/components/mender-mcu-client/core/src/mender-client.c (884): Checking for deployment...
[00:10:17.791,000] <inf> mender: CMAKE_SOURCE_DIR/components/mender-mcu-client/core/src/mender-client.c (892): No deployment available
[00:10:17.805,000] <inf> mender_stm32l4a6_zephyr_example: Mender client released network
```

Congratulation! Your device is connected to the mender server. Device type is `mender-stm32l4a6-zephyr-example` and the current software version is displayed.

### Create a new deployment

First retrieve [mender-artifact](https://docs.mender.io/downloads#mender-artifact) tool.

Change `VERSION` file to `VERSION_MAJOR=0` and `VERSION_MINOR=2`, rebuild and sign the firmware using the following commands. We previously used `hex` file because it is required to flash the device, but we now use `bin` file that is required for the mender-mcu-client:

```
west build -b nucleo_l4a6zg path/to/mender-stm32l4a6-zephyr-example
```

Then create a new artifact using the following command line:

```
path/to/mender-artifact write rootfs-image --compression none --device-type mender-stm32l4a6-zephyr-example --artifact-name mender-stm32l4a6-zephyr-example-v0.2.0 --output-path build/zephyr/mender-stm32l4a6-zephyr-example-v0.2.0.mender --file build/zephyr/zephyr.signed.bin
```

Upload the artifact `mender-stm32l4a6-zephyr-example-v0.2.0.mender` to the mender server and create a new deployment.

The device checks for the new deployment, downloads the artifact and installs it on the `slot1_partition`. Then it reboots to apply the update:

```
[00:40:02.176,000] <inf> mender_stm32l4a6_zephyr_example: Mender client connect network
[00:40:02.186,000] <inf> mender: CMAKE_SOURCE_DIR/components/mender-mcu-client/core/src/mender-client.c (884): Checking for deployment...
[00:40:07.045,000] <inf> mender: CMAKE_SOURCE_DIR/components/mender-mcu-client/core/src/mender-client.c (918): Downloading deployment artifact with id 'eac3b245-5bdd-427f-b763-?
[00:40:11.614,000] <inf> mender_stm32l4a6_zephyr_example: Deployment status is 'downloading'
[00:40:17.565,000] <inf> mender: CMAKE_SOURCE_DIR/components/mender-mcu-client/core/src/mender-artifact.c (382): Artifact has valid version
[00:40:17.585,000] <inf> mender: CMAKE_SOURCE_DIR/components/mender-mcu-client/platform/flash/zephyr/src/mender-flash.c (42): Start flashing artifact 'zephyr.signed.bin' with s6
[00:40:45.410,000] <inf> mender: CMAKE_SOURCE_DIR/components/mender-mcu-client/core/src/mender-client.c (930): Download done, installing artifact
[00:40:50.039,000] <inf> mender_stm32l4a6_zephyr_example: Deployment status is 'installing'
[00:40:54.588,000] <inf> mender_stm32l4a6_zephyr_example: Deployment status is 'rebooting'
[00:40:54.599,000] <inf> mender_stm32l4a6_zephyr_example: Mender client released network
[00:40:54.610,000] <inf> mender_stm32l4a6_zephyr_example: Restarting system
uart:~$ *** Booting MCUboot v2.1.0-rc1-4-g898a1ca64a75 ***
*** Using Zephyr OS build v3.6.0-7116-g40810983ead2 ***
I: Starting bootloader
I: Primary image: magic=unset, swap_type=0x1, copy_done=0x3, image_ok=0x3
I: Scratch: magic=unset, swap_type=0x1, copy_done=0x3, image_ok=0x3
I: Boot source: primary slot
I: Image index: 0, Swap type: test
I: Starting swap using scratch algorithm.
I: Bootloader chainload address offset: 0xe000
I: Jumping to the first image slot


[00:00:02.016,000] <inf> eth_w5500: eth_w5500@0: Link up
[00:00:02.033,000] <inf> mender_stm32l4a6_zephyr_example: IPv4 address: 192.168.1.216
[00:00:02.044,000] <inf> mender_stm32l4a6_zephyr_example: Lease time: 43200 seconds
[00:00:02.054,000] <inf> mender_stm32l4a6_zephyr_example: Subnet: 255.255.255.0
[00:00:02.064,000] <inf> mender_stm32l4a6_zephyr_example: Router: 192.168.1.1
[00:00:02.074,000] <inf> mender_stm32l4a6_zephyr_example: MAC address of the device '00:08:dc:01:02:03'
[00:00:02.086,000] <inf> mender_stm32l4a6_zephyr_example: Running project 'mender-stm32l4a6-zephyr-example' version '0.2.0'
[00:00:02.103,000] <inf> mender_stm32l4a6_zephyr_example: Mender client initialized
[00:00:02.113,000] <inf> mender_stm32l4a6_zephyr_example: Mender client registered hello-world module
[00:00:02.125,000] <inf> mender: CMAKE_SOURCE_DIR/components/mender-mcu-client/platform/storage/zephyr/nvs/src/mender-storage.c (242): Device configuration not available
[00:00:02.143,000] <inf> mender_stm32l4a6_zephyr_example: Mender configure add-on registered
[00:00:02.154,000] <inf> mender_stm32l4a6_zephyr_example: Mender inventory add-on registered
[00:00:02.165,000] <inf> mender_stm32l4a6_zephyr_example: Device configuration retrieved
[00:00:02.177,000] <inf> mender_stm32l4a6_zephyr_example: Mender client connect network
[00:00:12.840,000] <inf> mender_stm32l4a6_zephyr_example: Mender client authenticated
[00:00:12.851,000] <inf> mender: CMAKE_SOURCE_DIR/components/mender-mcu-client/platform/flash/zephyr/src/mender-flash.c (146): Application has been mark valid and rollback cancd
[00:00:17.383,000] <inf> mender_stm32l4a6_zephyr_example: Deployment status is 'success'
[00:00:17.394,000] <inf> mender: CMAKE_SOURCE_DIR/components/mender-mcu-client/core/src/mender-client.c (884): Checking for deployment...
[00:00:21.935,000] <inf> mender: CMAKE_SOURCE_DIR/components/mender-mcu-client/core/src/mender-client.c (892): No deployment available
[00:00:21.949,000] <inf> mender_stm32l4a6_zephyr_example: Mender client released network

```

Congratulation! You have updated the device. Mender server displays the success of the deployment.

### Failure or wanted rollback

In case of failure to connect and authenticate to the server the current example application performs a rollback to the previous release.
You can customize the behavior of the example application to add your own checks and perform the rollback in case the tests fail.

### Download and execute an LLEXT module

This example application supports LLEXT feature to permit loading and executing LLEXT modules using mender-mcu-client.

Compile the hello-world LLEXT sample from Zephyr as below:

```
west build -d build-llext -b nucleo_l4a6zg samples/subsys/llext/modules
```

Then create a new artifact using the following command line:

```
path/to/mender-artifact write module-image --compression none --device-type mender-stm32l4a6-zephyr-example --artifact-name hello-world-v1 --type hello-world --output-path build-llext/zephyr/mender-module-hello-world-v1.mender --file build-llext/zephyr/hello_world.llext
```

Upload the artifact `mender-module-hello-world-v1.mender` to the mender server and create a new deployment.

The device checks for the new deployment, downloads the artifact and call the `hellow_world` function of the module, which is simply displyaing `Hello, world, from an llext!` log in the console:

```
[00:00:12.987,000] <inf> mender: CMAKE_SOURCE_DIR/components/mender-mcu-client/core/src/mender-client.c (884): Checking for deployment...
[00:00:17.583,000] <inf> mender: CMAKE_SOURCE_DIR/components/mender-mcu-client/core/src/mender-client.c (918): Downloading deployment artifact with id '360e14aa-33d2-4f99-af7d-A
[00:00:22.177,000] <inf> mender_stm32l4a6_zephyr_example: Deployment status is 'downloading'
[00:00:27.725,000] <inf> mender: CMAKE_SOURCE_DIR/components/mender-mcu-client/core/src/mender-artifact.c (382): Artifact has valid version
[00:00:27.751,000] <inf> mender: CMAKE_SOURCE_DIR/components/mender-mcu-client/core/src/mender-client.c (930): Download done, installing artifact
[00:00:32.275,000] <inf> mender_stm32l4a6_zephyr_example: Deployment status is 'installing'
Hello, world, from an llext!
[00:00:36.817,000] <inf> mender_stm32l4a6_zephyr_example: Deployment status is 'success'
```

### Using an other zephyr evaluation board

The zephyr integration into the mender-mcu-client is generic and it is not limited to STM32 MCUs.
Several points discussed below should be taken into consideration to use an other hardware, including evaluation boards with other MCU families.

#### Flash sectors size

The flash sector size is an important criteria to select an evaluation board. The STM32L4A6ZG MCU has 2KB sectors, which is very convenient to define a custom layout. Other MCUs have variable sectors size and it can be very difficult to choose the partitions.

What remains important is that partitions are aligned on sectors. Moreover `slot0_partition` and `slot1_partition` must have the same size.

#### Configuration of the storage partition

The storage partition should be at least 4KB and must contains at least 3 sectors. In the current example the size of the `storage_partition` is 8KB and the configuration is set with `CONFIG_MENDER_STORAGE_SECTOR_COUNT=4` (4 sectors of 2KB). You should adapt this if you have a different flash layout.

#### Using an other network interface

The example is currently using a W5500 module connected to the NUCLEO-L4A6ZG evaluation board according to the device tree overlay. It is possible to use an other module depending of your own hardware. The mender-mcu-client expect to have a TCP-IP interface but it is not constraint by the physical hardware.

### Using an other mender instance

The communication with the server is done using HTTPS. To get it working, the Root CA that is providing the server certificate should be integrated and registered in the application (see `tls_credential_add` in the `src/main.c` file). Format of the expected Root CA certificate is DER.
In this example we are using the `https://hosted.mender.io` server. While checking the details of the server certificate in your browser, you will see that is is provided by Amazon Root CA 1. Thus the Amazon Root CA 1 certificate `AmazonRootCA1.cer` retrieved at `https://www.amazontrust.com/repository` is integrated in the application.
