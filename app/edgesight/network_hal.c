/****************************************************************************
 * app/edgesight/network_hal.c
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * EdgeSight - Network HAL implementation.
 * Wraps Ethernet + lwIP + MQTT client for NuttX.
 *
 * TODO: Replace stub code with actual lwIP/MQTT calls when
 * hardware is available.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include "network_hal.h"
#include <string.h>
#include <stdio.h>

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int network_hal_init(struct network_context_s *ctx,
                     const struct network_config_s *cfg)
{
  memset(ctx, 0, sizeof(*ctx));
  ctx->config = *cfg;

  /* On real hardware:
   *   1. Initialize Ethernet MAC (GMAC) + PHY (LAN8742)
   *   2. Start lwIP stack (netif_add, dhcp_start or static IP)
   *   3. Create MQTT client instance
   *
   * Reference: VENC_RTSP_Server/app_netxduo.c (uses NetXDuo,
   * we'll adapt to lwIP for NuttX)
   */

  ctx->state = NET_STATE_DISCONNECTED;
  ctx->initialized = true;

  printf("[network] Initialized (stub) broker=%s:%u\n",
         cfg->mqtt_broker ? cfg->mqtt_broker : "none",
         cfg->mqtt_port);
  return 0;
}

int network_hal_connect(struct network_context_s *ctx)
{
  if (!ctx->initialized)
    {
      return -1;
    }

  /* On real hardware:
   *   1. Wait for link up (PHY negotiation)
   *   2. Wait for DHCP (or set static IP)
   *   3. mqtt_client_connect(broker, port, client_id, ...)
   */

  ctx->state = NET_STATE_CONNECTED;
  printf("[network] Connected to MQTT broker (stub)\n");
  return 0;
}

int network_hal_send_alert(struct network_context_s *ctx,
                           const struct alert_message_s *alert)
{
  if (!ctx->initialized || ctx->state != NET_STATE_CONNECTED)
    {
      ctx->alerts_failed++;
      return -1;
    }

  /* On real hardware:
   *   Format JSON payload:
   *   {"level":"critical","confidence":0.84,"angle":90.0,
   *    "frame":1234,"msg":"Fall detected"}
   *   mqtt_publish(client, topic, payload, qos=1, retain=0);
   */

  printf("[network] ALERT sent: level=%u conf=%.2f \"%s\" (stub)\n",
         alert->level,
         (double)alert->confidence,
         alert->description ? alert->description : "");

  ctx->alerts_sent++;
  return 0;
}

int network_hal_poll(struct network_context_s *ctx)
{
  if (!ctx->initialized)
    {
      return NET_STATE_DISCONNECTED;
    }

  /* On real hardware:
   *   mqtt_yield(client, timeout_ms);
   *   Check link status, reconnect if needed
   */

  return (int)ctx->state;
}

uint32_t network_hal_get_state(const struct network_context_s *ctx)
{
  return ctx->state;
}

void network_hal_deinit(struct network_context_s *ctx)
{
  if (!ctx->initialized)
    {
      return;
    }

  /* On real hardware:
   *   mqtt_disconnect(client);
   *   netif_remove(&netif);
   */

  memset(ctx, 0, sizeof(*ctx));
  printf("[network] Deinitialized\n");
}
