/**
 * @file      main.c
 * @brief     Main entry point
 *
 * MIT License
 *
 * Copyright (c) 2022-2023 joelguittet and mender-mcu-client contributors
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <app_version.h>
#include <version.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(mender_stm32l4a6_zephyr_example, LOG_LEVEL_INF);

#include <zephyr/kernel.h>
#include <zephyr/net/net_if.h>
#include <zephyr/net/net_mgmt.h>
#include <zephyr/sys/reboot.h>

/*
 * Amazon Root CA 1 certificate, retrieved from https://www.amazontrust.com/repository in DER format.
 * It is converted to include file in application CMakeLists.txt.
 */
#if defined(CONFIG_NET_SOCKETS_SOCKOPT_TLS)
#include <zephyr/net/tls_credentials.h>
#if defined(CONFIG_TLS_CREDENTIAL_FILENAMES)
static const unsigned char ca_certificate[] = "AmazonRootCA1.cer";
#else
static const unsigned char ca_certificate[] = {
#include "AmazonRootCA1.cer.inc"
};
#endif
#endif

#include "mender-client.h"
#include "mender-configure.h"
#include "mender-flash.h"
#include "mender-inventory.h"
#include "mender-shell.h"
#include "mender-troubleshoot.h"

/**
 * @brief Mender client events
 */
static K_EVENT_DEFINE(mender_client_events);
#define MENDER_CLIENT_EVENT_NETWORK_UP (1 << 0)
#define MENDER_CLIENT_EVENT_RESTART    (1 << 1)

/**
 * @brief Network management callback
 */
static struct net_mgmt_event_callback mgmt_cb;

/**
 * @brief print DHCPv4 address information
 * @param iface Interface
 * @param if_addr Interface address
 * @param user_data user data (not used)
 */
static void
print_dhcpv4_addr(struct net_if *iface, struct net_if_addr *if_addr, void *user_data) {

    char           hr_addr[NET_IPV4_ADDR_LEN];
    struct in_addr netmask;

    /* Check address type */
    if (NET_ADDR_DHCP != if_addr->addr_type) {
        return;
    }

    LOG_INF("IPv4 address: %s", net_addr_ntop(AF_INET, &if_addr->address.in_addr, hr_addr, NET_IPV4_ADDR_LEN));
    LOG_INF("Lease time: %u seconds", iface->config.dhcpv4.lease_time);
    netmask = net_if_ipv4_get_netmask_by_addr(iface, &if_addr->address.in_addr);
    LOG_INF("Subnet: %s", net_addr_ntop(AF_INET, &netmask, hr_addr, NET_IPV4_ADDR_LEN));
    LOG_INF("Router: %s", net_addr_ntop(AF_INET, &iface->config.ip.ipv4->gw, hr_addr, NET_IPV4_ADDR_LEN));
}

/**
 * @brief Network event handler
 * @param cb Network management callback
 * @param mgmt_event Event
 * @param iface Interface
 */
static void
net_event_handler(struct net_mgmt_event_callback *cb, uint32_t mgmt_event, struct net_if *iface) {

    /* Check event */
    if (NET_EVENT_IPV4_ADDR_ADD != mgmt_event) {
        return;
    }

    /* Print interface information */
    net_if_ipv4_addr_foreach(iface, print_dhcpv4_addr, NULL);

    /* Application can now process to the initialization of the mender-mcu-client */
    k_event_post(&mender_client_events, MENDER_CLIENT_EVENT_NETWORK_UP);
}

/**
 * @brief Authentication success callback
 * @return MENDER_OK if application is marked valid and success deployment status should be reported to the server, error code otherwise
 */
static mender_err_t
authentication_success_cb(void) {

    mender_err_t ret;

    LOG_INF("Mender client authenticated");

#ifdef CONFIG_MENDER_CLIENT_ADD_ON_TROUBLESHOOT
    /* Activate troubleshoot add-on (deactivated by default) */
    if (MENDER_OK != (ret = mender_troubleshoot_activate())) {
        LOG_ERR("Unable to activate troubleshoot add-on");
        return ret;
    }
#endif /* CONFIG_MENDER_CLIENT_ADD_ON_TROUBLESHOOT */

    /* Validate the image if it is still pending */
    /* Note it is possible to do multiple diagnosic tests before validating the image */
    /* In this example, authentication success with the mender-server is enough */
    if (MENDER_OK != (ret = mender_flash_confirm_image())) {
        LOG_ERR("Unable to validate the image");
        return ret;
    }

    return ret;
}

/**
 * @brief Authentication failure callback
 * @return MENDER_OK if nothing to do, error code if the mender client should restart the application
 */
static mender_err_t
authentication_failure_cb(void) {

    static int tries = 0;

    /* Check if confirmation of the image is still pending */
    if (true == mender_flash_is_image_confirmed()) {
        LOG_INF("Mender client authentication failed");
        return MENDER_OK;
    }

    /* Increment number of failures */
    tries++;
    LOG_ERR("Mender client authentication failed (%d/%d)", tries, CONFIG_EXAMPLE_AUTHENTICATION_FAILS_MAX_TRIES);

    /* Restart the application after several authentication failures with the mender-server */
    /* The image has not been confirmed and the bootloader will now rollback to the previous working image */
    /* Note it is possible to customize this depending of the wanted behavior */
    return (tries >= CONFIG_EXAMPLE_AUTHENTICATION_FAILS_MAX_TRIES) ? MENDER_FAIL : MENDER_OK;
}

/**
 * @brief Deployment status callback
 * @param status Deployment status value
 * @param desc Deployment status description as string
 * @return MENDER_OK if the function succeeds, error code otherwise
 */
static mender_err_t
deployment_status_cb(mender_deployment_status_t status, char *desc) {

    /* We can do something else if required */
    LOG_INF("Deployment status is '%s'", desc);

    return MENDER_OK;
}

/**
 * @brief Restart callback
 * @return MENDER_OK if the function succeeds, error code otherwise
 */
static mender_err_t
restart_cb(void) {

    /* Application is responsible to shutdown and restart the system now */
    k_event_post(&mender_client_events, MENDER_CLIENT_EVENT_RESTART);

    return MENDER_OK;
}

#ifdef CONFIG_MENDER_CLIENT_ADD_ON_CONFIGURE
#ifndef CONFIG_MENDER_CLIENT_CONFIGURE_STORAGE

/**
 * @brief Device configuration updated
 * @param configuration Device configuration
 * @return MENDER_OK if the function succeeds, error code otherwise
 */
static mender_err_t
config_updated_cb(mender_keystore_t *configuration) {

    /* Application can use the new device configuration now */
    /* In this example, we just print the content of the configuration received from the Mender server */
    if (NULL != configuration) {
        size_t index = 0;
        LOG_INF("Device configuration received from the server");
        while ((NULL != configuration[index].name) && (NULL != configuration[index].value)) {
            LOG_INF("Key=%s, value=%s", configuration[index].name, configuration[index].value);
            index++;
        }
    }

    return MENDER_OK;
}

#endif /* CONFIG_MENDER_CLIENT_CONFIGURE_STORAGE */
#endif /* CONFIG_MENDER_CLIENT_ADD_ON_CONFIGURE */

/**
 * @brief Main function
 * @return Always returns 0
 */
int
main(void) {

    /* Initialize network */
    struct net_if *iface = net_if_get_default();
    assert(NULL != iface);
    net_mgmt_init_event_callback(&mgmt_cb, net_event_handler, NET_EVENT_IPV4_ADDR_ADD);
    net_mgmt_add_event_callback(&mgmt_cb);
    net_dhcpv4_start(iface);

    /* Wait for mender-mcu-client events */
    k_event_wait_all(&mender_client_events, MENDER_CLIENT_EVENT_NETWORK_UP, false, K_FOREVER);

#if defined(CONFIG_NET_SOCKETS_SOCKOPT_TLS)
    /* Initialize certificate */
    tls_credential_add(CONFIG_MENDER_NET_CA_CERTIFICATE_TAG, TLS_CREDENTIAL_CA_CERTIFICATE, ca_certificate, sizeof(ca_certificate));
#endif

    /* Read base MAC address of the device */
    char                 mac_address[18];
    struct net_linkaddr *linkaddr = net_if_get_link_addr(iface);
    assert(NULL != linkaddr);
    snprintf(mac_address,
             sizeof(mac_address),
             "%02x:%02x:%02x:%02x:%02x:%02x",
             linkaddr->addr[0],
             linkaddr->addr[1],
             linkaddr->addr[2],
             linkaddr->addr[3],
             linkaddr->addr[4],
             linkaddr->addr[5]);
    LOG_INF("MAC address of the device '%s'", mac_address);

    /* Retrieve running version of the device */
    LOG_INF("Running project '%s' version '%s'", PROJECT_NAME, APP_VERSION_STRING);

    /* Compute artifact name */
    char artifact_name[128];
    snprintf(artifact_name, sizeof(artifact_name), "%s-v%s", PROJECT_NAME, APP_VERSION_STRING);

    /* Retrieve device type */
    char *device_type = PROJECT_NAME;

    /* Initialize mender-client */
    mender_keystore_t         identity[]              = { { .name = "mac", .value = mac_address }, { .name = NULL, .value = NULL } };
    mender_client_config_t    mender_client_config    = { .identity                     = identity,
                                                    .artifact_name                = artifact_name,
                                                    .device_type                  = device_type,
                                                    .host                         = NULL,
                                                    .tenant_token                 = NULL,
                                                    .authentication_poll_interval = 0,
                                                    .update_poll_interval         = 0,
                                                    .recommissioning              = false };
    mender_client_callbacks_t mender_client_callbacks = { .authentication_success = authentication_success_cb,
                                                          .authentication_failure = authentication_failure_cb,
                                                          .deployment_status      = deployment_status_cb,
                                                          .restart                = restart_cb };
    assert(MENDER_OK == mender_client_init(&mender_client_config, &mender_client_callbacks));
    LOG_INF("Mender client initialized");

    /* Initialize mender add-ons */
#ifdef CONFIG_MENDER_CLIENT_ADD_ON_CONFIGURE
    mender_configure_config_t    mender_configure_config    = { .refresh_interval = 0 };
    mender_configure_callbacks_t mender_configure_callbacks = {
#ifndef CONFIG_MENDER_CLIENT_CONFIGURE_STORAGE
        .config_updated = config_updated_cb,
#endif /* CONFIG_MENDER_CLIENT_CONFIGURE_STORAGE */
    };
    assert(MENDER_OK
           == mender_client_register_addon(
               (mender_addon_instance_t *)&mender_configure_addon_instance, (void *)&mender_configure_config, (void *)&mender_configure_callbacks));
    LOG_INF("Mender configure add-on registered");
#endif /* CONFIG_MENDER_CLIENT_ADD_ON_CONFIGURE */
#ifdef CONFIG_MENDER_CLIENT_ADD_ON_INVENTORY
    mender_inventory_config_t mender_inventory_config = { .refresh_interval = 0 };
    assert(MENDER_OK == mender_client_register_addon((mender_addon_instance_t *)&mender_inventory_addon_instance, (void *)&mender_inventory_config, NULL));
    LOG_INF("Mender inventory add-on registered");
#endif /* CONFIG_MENDER_CLIENT_ADD_ON_INVENTORY */
#ifdef CONFIG_MENDER_CLIENT_ADD_ON_TROUBLESHOOT
    mender_troubleshoot_config_t    mender_troubleshoot_config = { .healthcheck_interval = 0 };
    mender_troubleshoot_callbacks_t mender_troubleshoot_callbacks
        = { .shell_begin = shell_begin_cb, .shell_resize = shell_resize_cb, .shell_write = shell_write_cb, .shell_end = shell_end_cb };
    assert(MENDER_OK
           == mender_client_register_addon(
               (mender_addon_instance_t *)&mender_troubleshoot_addon_instance, (void *)&mender_troubleshoot_config, (void *)&mender_troubleshoot_callbacks));
    LOG_INF("Mender troubleshoot add-on registered");
#endif /* CONFIG_MENDER_CLIENT_ADD_ON_TROUBLESHOOT */

#ifdef CONFIG_MENDER_CLIENT_ADD_ON_CONFIGURE
    /* Get mender configuration (this is just an example to illustrate the API) */
    mender_keystore_t *configuration;
    if (MENDER_OK != mender_configure_get(&configuration)) {
        LOG_ERR("Unable to get mender configuration");
    } else if (NULL != configuration) {
        size_t index = 0;
        LOG_INF("Device configuration retrieved");
        while ((NULL != configuration[index].name) && (NULL != configuration[index].value)) {
            LOG_INF("Key=%s, value=%s", configuration[index].name, configuration[index].value);
            index++;
        }
        mender_utils_keystore_delete(configuration);
    }
#endif /* CONFIG_MENDER_CLIENT_ADD_ON_CONFIGURE */

#ifdef CONFIG_MENDER_CLIENT_ADD_ON_INVENTORY
    /* Set mender inventory (this is just an example to illustrate the API) */
    mender_keystore_t inventory[] = { { .name = "zephyr-rtos", .value = KERNEL_VERSION_STRING },
                                      { .name = "mender-mcu-client", .value = mender_client_version() },
                                      { .name = "latitude", .value = "45.8325" },
                                      { .name = "longitude", .value = "6.864722" },
                                      { .name = NULL, .value = NULL } };
    if (MENDER_OK != mender_inventory_set(inventory)) {
        LOG_ERR("Unable to set mender inventory");
    }
#endif /* CONFIG_MENDER_CLIENT_ADD_ON_INVENTORY */

    /* Finally activate mender client */
    if (MENDER_OK != mender_client_activate()) {
        LOG_ERR("Unable to activate mender-client");
        goto RELEASE;
    }

    /* Wait for mender-mcu-client events */
    k_event_wait_all(&mender_client_events, MENDER_CLIENT_EVENT_RESTART, false, K_FOREVER);

RELEASE:

    /* Deactivate and release mender-client */
    mender_client_deactivate();
    mender_client_exit();

    /* Restart */
    LOG_INF("Restarting system");
    sys_reboot(SYS_REBOOT_WARM);

    return 0;
}
