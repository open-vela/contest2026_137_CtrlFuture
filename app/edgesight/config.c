/****************************************************************************
 * app/edgesight/config.c
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to you under the Apache License, Version
 * 2.0 (the "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
 * implied.  See the License for the specific language governing
 * permissions and limitations under the License.
 *
 * EdgeSight - Runtime configuration implementation.
 * Simple key=value parser for SD card config file.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include "config.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static void trim(char *s)
{
  char *end;

  /* Trim leading spaces */

  while (*s == ' ' || *s == '\t')
    {
      memmove(s, s + 1, strlen(s));
    }

  /* Trim trailing spaces/newline */

  end = s + strlen(s) - 1;
  while (end > s && (*end == ' ' || *end == '\t' ||
         *end == '\n' || *end == '\r'))
    {
      *end-- = '\0';
    }
}

static int parse_line(struct edgesight_config_s *cfg,
                      const char *key, const char *value)
{
  /* Fall detection */

  if (strcmp(key, "fall.angle_thresh") == 0)
    {
      cfg->fall.torso_angle_thresh = (float)atof(value);
    }
  else if (strcmp(key, "fall.cog_thresh") == 0)
    {
      cfg->fall.cog_ratio_thresh = (float)atof(value);
    }
  else if (strcmp(key, "fall.bbox_thresh") == 0)
    {
      cfg->fall.bbox_ratio_thresh = (float)atof(value);
    }
  else if (strcmp(key, "fall.confidence") == 0)
    {
      cfg->fall.confidence_thresh = (float)atof(value);
    }
  else if (strcmp(key, "fall.frames") == 0)
    {
      cfg->fall.consecutive_frames = (uint8_t)atoi(value);
    }

  /* Recording */

  else if (strcmp(key, "record.duration") == 0)
    {
      cfg->record.max_duration_s = (uint32_t)atoi(value);
    }
  else if (strcmp(key, "record.pre_event") == 0)
    {
      cfg->record.pre_event_s = (uint32_t)atoi(value);
    }
  else if (strcmp(key, "record.bitrate") == 0)
    {
      cfg->record.bitrate_kbps = (uint32_t)atoi(value);
    }
  else if (strcmp(key, "record.fps") == 0)
    {
      cfg->record.fps = (uint8_t)atoi(value);
    }
  else if (strcmp(key, "record.gop") == 0)
    {
      cfg->record.gop_size = (uint8_t)atoi(value);
    }

  /* Network */

  else if (strcmp(key, "mqtt.broker") == 0)
    {
      strncpy(cfg->network.mqtt_broker, value, 47);
      cfg->network.mqtt_broker[47] = '\0';
    }
  else if (strcmp(key, "mqtt.port") == 0)
    {
      cfg->network.mqtt_port = (uint16_t)atoi(value);
    }
  else if (strcmp(key, "mqtt.topic") == 0)
    {
      strncpy(cfg->network.mqtt_topic, value, 47);
      cfg->network.mqtt_topic[47] = '\0';
    }
  else if (strcmp(key, "mqtt.device_id") == 0)
    {
      strncpy(cfg->network.device_id, value, 23);
      cfg->network.device_id[23] = '\0';
    }
  else if (strcmp(key, "mqtt.qos") == 0)
    {
      cfg->network.mqtt_qos = (uint8_t)atoi(value);
    }

  /* Display */

  else if (strcmp(key, "display.bbox") == 0)
    {
      cfg->display.show_bbox = (atoi(value) != 0);
    }
  else if (strcmp(key, "display.skeleton") == 0)
    {
      cfg->display.show_skeleton = (atoi(value) != 0);
    }
  else if (strcmp(key, "display.stats") == 0)
    {
      cfg->display.show_stats = (atoi(value) != 0);
    }
  else if (strcmp(key, "display.alert") == 0)
    {
      cfg->display.show_alert = (atoi(value) != 0);
    }

  /* Global */

  else if (strcmp(key, "log_level") == 0)
    {
      cfg->log_level = (uint8_t)atoi(value);
    }
  else if (strcmp(key, "auto_record") == 0)
    {
      cfg->auto_record = (atoi(value) != 0);
    }
  else if (strcmp(key, "auto_alert") == 0)
    {
      cfg->auto_alert = (atoi(value) != 0);
    }
  else
    {
      return -1;  /* Unknown key */
    }

  return 0;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

void config_set_defaults(struct edgesight_config_s *cfg)
{
  memset(cfg, 0, sizeof(*cfg));

  /* Fall detection defaults */

  cfg->fall.torso_angle_thresh = 45.0f;
  cfg->fall.cog_ratio_thresh = 0.5f;
  cfg->fall.bbox_ratio_thresh = 1.2f;
  cfg->fall.confidence_thresh = 0.6f;
  cfg->fall.consecutive_frames = 3;

  /* Recording defaults */

  cfg->record.max_duration_s = 30;
  cfg->record.pre_event_s = 5;
  cfg->record.bitrate_kbps = 2000;
  cfg->record.fps = 15;
  cfg->record.gop_size = 30;

  /* Network defaults */

  strncpy(cfg->network.mqtt_broker, "192.168.1.1", 47);
  cfg->network.mqtt_port = 1883;
  strncpy(cfg->network.mqtt_topic, "edgesight/alert", 47);
  strncpy(cfg->network.device_id, "edgesight-001", 23);
  cfg->network.mqtt_qos = 1;

  /* Display defaults */

  cfg->display.show_bbox = true;
  cfg->display.show_skeleton = true;
  cfg->display.show_stats = true;
  cfg->display.show_alert = true;
  cfg->display.alert_duration_s = 5;

  /* Global defaults */

  cfg->log_level = 1;  /* INFO */
  cfg->auto_record = true;
  cfg->auto_alert = true;
}

int config_load_file(struct edgesight_config_s *cfg,
                     const char *path)
{
  FILE *fp;
  char line[CONFIG_KEY_MAX + CONFIG_VALUE_MAX + 4];
  char *eq;
  char key[CONFIG_KEY_MAX];
  char value[CONFIG_VALUE_MAX];
  int lineno = 0;

  if (path == NULL)
    {
      path = CONFIG_FILE_PATH;
    }

  fp = fopen(path, "r");
  if (fp == NULL)
    {
      return -1;
    }

  while (fgets(line, sizeof(line), fp) != NULL)
    {
      lineno++;

      /* Skip comments and empty lines */

      trim(line);
      if (line[0] == '#' || line[0] == '\0')
        {
          continue;
        }

      /* Find key=value separator */

      eq = strchr(line, '=');
      if (eq == NULL)
        {
          continue;
        }

      /* Split into key and value */

      *eq = '\0';
      strncpy(key, line, CONFIG_KEY_MAX - 1);
      key[CONFIG_KEY_MAX - 1] = '\0';
      strncpy(value, eq + 1, CONFIG_VALUE_MAX - 1);
      value[CONFIG_VALUE_MAX - 1] = '\0';

      trim(key);
      trim(value);

      parse_line(cfg, key, value);
    }

  fclose(fp);
  printf("[config] Loaded %d lines from %s\n",
         lineno, path);
  return 0;
}

int config_save_file(const struct edgesight_config_s *cfg,
                     const char *path)
{
  FILE *fp;

  if (path == NULL)
    {
      path = CONFIG_FILE_PATH;
    }

  fp = fopen(path, "w");
  if (fp == NULL)
    {
      return -1;
    }

  fprintf(fp, "# EdgeSight Configuration\n\n");

  fprintf(fp, "# Fall Detection\n");
  fprintf(fp, "fall.angle_thresh=%.1f\n",
          (double)cfg->fall.torso_angle_thresh);
  fprintf(fp, "fall.cog_thresh=%.2f\n",
          (double)cfg->fall.cog_ratio_thresh);
  fprintf(fp, "fall.bbox_thresh=%.2f\n",
          (double)cfg->fall.bbox_ratio_thresh);
  fprintf(fp, "fall.confidence=%.2f\n",
          (double)cfg->fall.confidence_thresh);
  fprintf(fp, "fall.frames=%u\n",
          cfg->fall.consecutive_frames);

  fprintf(fp, "\n# Recording\n");
  fprintf(fp, "record.duration=%lu\n",
          (unsigned long)cfg->record.max_duration_s);
  fprintf(fp, "record.pre_event=%lu\n",
          (unsigned long)cfg->record.pre_event_s);
  fprintf(fp, "record.bitrate=%lu\n",
          (unsigned long)cfg->record.bitrate_kbps);
  fprintf(fp, "record.fps=%u\n", cfg->record.fps);
  fprintf(fp, "record.gop=%u\n", cfg->record.gop_size);

  fprintf(fp, "\n# MQTT Network\n");
  fprintf(fp, "mqtt.broker=%s\n",
          cfg->network.mqtt_broker);
  fprintf(fp, "mqtt.port=%u\n", cfg->network.mqtt_port);
  fprintf(fp, "mqtt.topic=%s\n",
          cfg->network.mqtt_topic);
  fprintf(fp, "mqtt.device_id=%s\n",
          cfg->network.device_id);
  fprintf(fp, "mqtt.qos=%u\n", cfg->network.mqtt_qos);

  fprintf(fp, "\n# Display\n");
  fprintf(fp, "display.bbox=%d\n",
          cfg->display.show_bbox);
  fprintf(fp, "display.skeleton=%d\n",
          cfg->display.show_skeleton);
  fprintf(fp, "display.stats=%d\n",
          cfg->display.show_stats);
  fprintf(fp, "display.alert=%d\n",
          cfg->display.show_alert);

  fprintf(fp, "\n# Global\n");
  fprintf(fp, "log_level=%u\n", cfg->log_level);
  fprintf(fp, "auto_record=%d\n", cfg->auto_record);
  fprintf(fp, "auto_alert=%d\n", cfg->auto_alert);

  fclose(fp);
  return 0;
}

void config_dump(const struct edgesight_config_s *cfg)
{
  printf("=== EdgeSight Config ===\n");
  printf("[fall] angle=%.1f cog=%.2f bbox=%.2f"
         " conf=%.2f frames=%u\n",
         (double)cfg->fall.torso_angle_thresh,
         (double)cfg->fall.cog_ratio_thresh,
         (double)cfg->fall.bbox_ratio_thresh,
         (double)cfg->fall.confidence_thresh,
         cfg->fall.consecutive_frames);
  printf("[record] dur=%lus bitrate=%lukbps"
         " fps=%u gop=%u\n",
         (unsigned long)cfg->record.max_duration_s,
         (unsigned long)cfg->record.bitrate_kbps,
         cfg->record.fps,
         cfg->record.gop_size);
  printf("[mqtt] %s:%u topic=%s id=%s\n",
         cfg->network.mqtt_broker,
         cfg->network.mqtt_port,
         cfg->network.mqtt_topic,
         cfg->network.device_id);
  printf("[display] bbox=%d skel=%d stats=%d"
         " alert=%d\n",
         cfg->display.show_bbox,
         cfg->display.show_skeleton,
         cfg->display.show_stats,
         cfg->display.show_alert);
  printf("[global] log=%u auto_rec=%d"
         " auto_alert=%d\n",
         cfg->log_level,
         cfg->auto_record,
         cfg->auto_alert);
}
