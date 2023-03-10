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

#include <zephyr/net/net_if.h>
#include <zephyr/net/net_mgmt.h>
#include <zephyr/sys/reboot.h>

/*
 * @brief Amazon RootCA certificate
 * @note This file is "CN=Starfield Services Root Certificate Authority - G2,O=Starfield Technologies\, Inc.,L=Scottsdale,ST=Arizona,C=US"
 *       It is retrieved from https://www.amazontrust.com/repository/ in DER format. It is converted to include file in application CMakeLists.txt.
 */
#if defined(CONFIG_NET_SOCKETS_SOCKOPT_TLS)
#include <zephyr/net/tls_credentials.h>
#if defined(CONFIG_TLS_CREDENTIAL_FILENAMES)
static const unsigned char ca_certificate[] = "SFSRootCAG2.cer";
#else
static const unsigned char ca_certificate[] = {
#include "SFSRootCAG2.cer.inc"
};
#endif
#endif

#include "mender-client.h"
#include "mender-ota.h"

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

    if (NET_EVENT_IPV4_ADDR_ADD != mgmt_event) {
        return;
    }

    for (int i = 0; i < NET_IF_MAX_IPV4_ADDR; i++) {
        if (NET_ADDR_DHCP == iface->config.ip.ipv4->unicast[i].addr_type) {
            char buf[NET_IPV4_ADDR_LEN];
            LOG_INF("Your address: %s", net_addr_ntop(AF_INET, &iface->config.ip.ipv4->unicast[i].address.in_addr, buf, sizeof(buf)));
            LOG_INF("Lease time: %u seconds", iface->config.dhcpv4.lease_time);
            LOG_INF("Subnet: %s", net_addr_ntop(AF_INET, &iface->config.ip.ipv4->netmask, buf, sizeof(buf)));
            LOG_INF("Router: %s", net_addr_ntop(AF_INET, &iface->config.ip.ipv4->gw, buf, sizeof(buf)));
        }
    }
}

/**
 * @brief Authentication success callback
 * @return MENDER_OK if application is marked valid and success deployment status should be reported to the server, error code otherwise
 */
static mender_err_t
authentication_success_cb(void) {

    LOG_INF("Mender client authenticated");

    /* Validate the image if it is still pending */
    /* Note it is possible to do multiple diagnosic tests before validating the image */
    /* In this example, authentication success with the mender-server is enough */
    return mender_ota_mark_app_valid_cancel_rollback();
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
        ret = mender_ota_mark_app_invalid_rollback_and_reboot();
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

    /* Restart */
    /* Note it is possible to not restart the system right now depending of the application */
    /* In this example, immediate restart is enough */
    LOG_INF("Restarting system");
    sys_reboot(SYS_REBOOT_WARM);

    return MENDER_OK;
}

/**
 * @brief Main function
 */
void
main(void) {

    /* Initialize network */
    struct net_if *iface = net_if_get_default();
    assert(NULL != iface);
    net_mgmt_init_event_callback(&mgmt_cb, net_event_handler, NET_EVENT_IPV4_ADDR_ADD);
    net_mgmt_add_event_callback(&mgmt_cb);
    net_dhcpv4_start(iface);

#if defined(CONFIG_NET_SOCKETS_SOCKOPT_TLS)
    /* Initialize certificate */
    tls_credential_add(CONFIG_MENDER_HTTP_CA_CERTIFICATE_TAG, TLS_CREDENTIAL_CA_CERTIFICATE, ca_certificate, sizeof(ca_certificate));
#endif

    /* Read base MAC address of the STM32 */
    char                 mac_address[18];
    struct net_linkaddr *linkaddr = net_if_get_link_addr(iface);
    assert(NULL != linkaddr);
    /**
     * TODO this is a temporary hack because the W5500 driver does not take care of the local-mac-address
     * This cause mender server to see the device with a different MAC at each boot because the MAC is randomized
     * This will be solved when https://github.com/zephyrproject-rtos/zephyr/pull/48763 will be merged
     */
#if 0
    sprintf(mac_address,
            "%02x:%02x:%02x:%02x:%02x:%02x",
            linkaddr->addr[0],
            linkaddr->addr[1],
            linkaddr->addr[2],
            linkaddr->addr[3],
            linkaddr->addr[4],
            linkaddr->addr[5]);
#else
    sprintf(mac_address, "00:08:dc:01:02:03");
#endif

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
                                                    .host                         = CONFIG_MENDER_SERVER_HOST,
                                                    .tenant_token                 = CONFIG_MENDER_SERVER_TENANT_TOKEN,
                                                    .authentication_poll_interval = CONFIG_MENDER_CLIENT_AUTHENTICATION_POLL_INTERVAL,
                                                    .inventory_poll_interval      = CONFIG_MENDER_CLIENT_INVENTORY_POLL_INTERVAL,
                                                    .update_poll_interval         = CONFIG_MENDER_CLIENT_UPDATE_POLL_INTERVAL,
                                                    .restart_poll_interval        = CONFIG_MENDER_CLIENT_RESTART_POLL_INTERVAL,
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

    /* Set mender inventory (this is just an example) */
    mender_inventory_t inventory[] = { { .name = "latitude", .value = "45.8325" }, { .name = "longitude", .value = "6.864722" } };
    if (MENDER_OK != mender_client_set_inventory(inventory, sizeof(inventory) / sizeof(inventory[0]))) {
        LOG_ERR("Unable to set mender inventory");
    }
}
