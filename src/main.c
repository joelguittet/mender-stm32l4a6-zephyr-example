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
#include "mender-inventory.h"
#include "mender-ota.h"

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
    for (int i = 0; i < NET_IF_MAX_IPV4_ADDR; i++) {
        if (NET_ADDR_DHCP == iface->config.ip.ipv4->unicast[i].addr_type) {
            char buf[NET_IPV4_ADDR_LEN];
            LOG_INF("Your address: %s", net_addr_ntop(AF_INET, &iface->config.ip.ipv4->unicast[i].address.in_addr, buf, sizeof(buf)));
            LOG_INF("Lease time: %u seconds", iface->config.dhcpv4.lease_time);
            LOG_INF("Subnet: %s", net_addr_ntop(AF_INET, &iface->config.ip.ipv4->netmask, buf, sizeof(buf)));
            LOG_INF("Router: %s", net_addr_ntop(AF_INET, &iface->config.ip.ipv4->gw, buf, sizeof(buf)));
        }
    }

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

    /* Activate mender add-ons */
    /* The application can activate each add-on depending of the current status of the device */
    /* In this example, add-ons are activated has soon as authentication succeeds */
#ifdef CONFIG_MENDER_CLIENT_ADD_ON_CONFIGURE
    if (MENDER_OK != (ret = mender_configure_activate())) {
        LOG_ERR("Unable to activate configure add-on");
        return ret;
    }
#endif /* CONFIG_MENDER_CLIENT_ADD_ON_CONFIGURE */
#ifdef CONFIG_MENDER_CLIENT_ADD_ON_INVENTORY
    if (MENDER_OK != (ret = mender_inventory_activate())) {
        LOG_ERR("Unable to activate inventory add-on");
        return ret;
    }
#endif /* CONFIG_MENDER_CLIENT_ADD_ON_INVENTORY */

    /* Validate the image if it is still pending */
    /* Note it is possible to do multiple diagnosic tests before validating the image */
    /* In this example, authentication success with the mender-server is enough */
    if (MENDER_OK != (ret = mender_ota_mark_app_valid_cancel_rollback())) {
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

    static int   tries = 0;
    mender_err_t ret   = MENDER_OK;

    /* Increment number of failures */
    tries++;
    LOG_INF("Mender client authentication failed (%d/%d)", tries, CONFIG_EXAMPLE_AUTHENTICATION_FAILS_MAX_TRIES);

    /* Invalidate the image if it is still pending */
    /* Note it is possible to invalid the image later to permit clean closure before reboot */
    /* In this example, several authentication failures with the mender-server is enough */
    if (tries >= CONFIG_EXAMPLE_AUTHENTICATION_FAILS_MAX_TRIES) {
        if (MENDER_OK != (ret = mender_ota_mark_app_invalid_rollback_and_reboot())) {
            LOG_ERR("Unable to invalidate the image");
            return ret;
        }
    }

    return ret;
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
config_updated(mender_keystore_t *configuration) {

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
    tls_credential_add(CONFIG_MENDER_HTTP_CA_CERTIFICATE_TAG, TLS_CREDENTIAL_CA_CERTIFICATE, ca_certificate, sizeof(ca_certificate));
#endif

    /* Read base MAC address of the STM32 */
    char                 mac_address[18];
    struct net_linkaddr *linkaddr = net_if_get_link_addr(iface);
    assert(NULL != linkaddr);
    sprintf(mac_address,
            "%02x:%02x:%02x:%02x:%02x:%02x",
            linkaddr->addr[0],
            linkaddr->addr[1],
            linkaddr->addr[2],
            linkaddr->addr[3],
            linkaddr->addr[4],
            linkaddr->addr[5]);

    /* Retrieve running version of the STM32 */
    LOG_INF("Running project '%s' version '%s'", PROJECT_NAME, PROJECT_VERSION);

    /* Compute artifact name */
    char artifact_name[128];
    sprintf(artifact_name, "%s-v%s", PROJECT_NAME, PROJECT_VERSION);

    /* Retrieve device type */
    char *device_type = PROJECT_NAME;

    /* Initialize mender-client */
    mender_client_config_t    mender_client_config    = { .mac_address                  = mac_address,
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
                                                          .ota_begin              = mender_ota_begin,
                                                          .ota_write              = mender_ota_write,
                                                          .ota_abort              = mender_ota_abort,
                                                          .ota_end                = mender_ota_end,
                                                          .ota_set_boot_partition = mender_ota_set_boot_partition,
                                                          .restart                = restart_cb };
    assert(MENDER_OK == mender_client_init(&mender_client_config, &mender_client_callbacks));
    LOG_INF("Mender client initialized");

    /* Initialize mender add-ons */
#ifdef CONFIG_MENDER_CLIENT_ADD_ON_CONFIGURE
    mender_configure_config_t    mender_configure_config    = { .refresh_interval = 0 };
    mender_configure_callbacks_t mender_configure_callbacks = {
#ifndef CONFIG_MENDER_CLIENT_CONFIGURE_STORAGE
        .config_updated = config_updated,
#endif /* CONFIG_MENDER_CLIENT_CONFIGURE_STORAGE */
    };
    assert(MENDER_OK == mender_configure_init(&mender_configure_config, &mender_configure_callbacks));
    LOG_INF("Mender configure initialized");
#endif /* CONFIG_MENDER_CLIENT_ADD_ON_CONFIGURE */
#ifdef CONFIG_MENDER_CLIENT_ADD_ON_INVENTORY
    mender_inventory_config_t mender_inventory_config = { .refresh_interval = 0 };
    assert(MENDER_OK == mender_inventory_init(&mender_inventory_config));
    LOG_INF("Mender inventory initialized");
#endif /* CONFIG_MENDER_CLIENT_ADD_ON_INVENTORY */

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
    mender_keystore_t inventory[]
        = { { .name = "latitude", .value = "45.8325" }, { .name = "longitude", .value = "6.864722" }, { .name = NULL, .value = NULL } };
    if (MENDER_OK != mender_inventory_set(inventory)) {
        LOG_ERR("Unable to set mender inventory");
    }
#endif /* CONFIG_MENDER_CLIENT_ADD_ON_INVENTORY */

    /* Wait for mender-mcu-client events */
    k_event_wait_all(&mender_client_events, MENDER_CLIENT_EVENT_RESTART, false, K_FOREVER);

    /* Release mender add-ons */
#ifdef CONFIG_MENDER_CLIENT_ADD_ON_INVENTORY
    mender_inventory_exit();
#endif /* CONFIG_MENDER_CLIENT_ADD_ON_INVENTORY */
#ifdef CONFIG_MENDER_CLIENT_ADD_ON_CONFIGURE
    mender_configure_exit();
#endif /* CONFIG_MENDER_CLIENT_ADD_ON_CONFIGURE */

    /* Exit mender-client */
    mender_client_exit();

    /* Restart */
    LOG_INF("Restarting system");
    sys_reboot(SYS_REBOOT_WARM);

    return 0;
}
