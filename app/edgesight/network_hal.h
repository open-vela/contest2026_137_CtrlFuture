/****************************************************************************
 * app/edgesight/network_hal.h
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * EdgeSight - Network services abstraction layer.
 * Provides MQTT alert publishing and optional RTSP streaming.
 *
 ****************************************************************************/

#ifndef __APP_EDGESIGHT_NETWORK_HAL_H
#define __APP_EDGESIGHT_NETWORK_HAL_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <stdint.h>
#include <stdbool.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* Alert severity levels */

#define ALERT_LEVEL_INFO     0
#define ALERT_LEVEL_WARNING  1
#define ALERT_LEVEL_CRITICAL 2  /* Fall detected */

/* Network state */

#define NET_STATE_DISCONNECTED 0
#define NET_STATE_CONNECTING   1
#define NET_STATE_CONNECTED    2
#define NET_STATE_ERROR        3

/****************************************************************************
 * Public Types
 ****************************************************************************/

/* Network configuration */

struct network_config_s
{
  /* Ethernet */

  bool use_dhcp;
  uint32_t static_ip;        /* If !use_dhcp */
  uint32_t gateway;
  uint32_t netmask;

  /* MQTT broker */

  const char *mqtt_broker;   /* e.g. "192.168.1.100" */
  uint16_t mqtt_port;        /* default 1883 */
  const char *mqtt_client_id;
  const char *mqtt_topic;    /* e.g. "edgesight/alerts" */

  /* RTSP (optional) */

  bool enable_rtsp;
  uint16_t rtsp_port;        /* default 8554 */
};

/* Alert message payload */

struct alert_message_s
{
  uint8_t  level;            /* ALERT_LEVEL_xxx */
  uint32_t timestamp;        /* System ticks */
  float    confidence;       /* Detection confidence [0..1] */
  float    torso_angle;      /* Fall detection metric */
  uint32_t frame_number;
  const char *description;   /* Human-readable text */
};

/* Network context */

struct network_context_s
{
  bool initialized;
  uint32_t state;
  struct network_config_s config;
  void *mqtt_client;         /* MQTT client handle */
  uint32_t alerts_sent;
  uint32_t alerts_failed;
};

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

/**
 * @brief Initialize Ethernet + lwIP + MQTT client
 * @param ctx    Network context
 * @param cfg    Network configuration
 * @return 0 on success, negative errno on failure
 */

int network_hal_init(struct network_context_s *ctx,
                     const struct network_config_s *cfg);

/**
 * @brief Connect to MQTT broker
 * @param ctx    Network context
 * @return 0 on success, negative errno on failure
 */

int network_hal_connect(struct network_context_s *ctx);

/**
 * @brief Publish an alert message via MQTT
 * @param ctx    Network context
 * @param alert  Alert message to publish
 * @return 0 on success, negative errno on failure
 */

int network_hal_send_alert(struct network_context_s *ctx,
                           const struct alert_message_s *alert);

/**
 * @brief Check and maintain MQTT connection (call periodically)
 * @param ctx    Network context
 * @return current network state
 */

int network_hal_poll(struct network_context_s *ctx);

/**
 * @brief Get current network state
 * @param ctx    Network context
 * @return NET_STATE_xxx
 */

uint32_t network_hal_get_state(const struct network_context_s *ctx);

/**
 * @brief Disconnect and deinitialize network
 */

void network_hal_deinit(struct network_context_s *ctx);

#endif /* __APP_EDGESIGHT_NETWORK_HAL_H */
