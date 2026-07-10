############################################################################
# app/edgesight/Makefile
#
# SPDX-License-Identifier: Apache-2.0
#
############################################################################

include $(APPDIR)/Make.defs

PROGNAME  = edgesight
PRIORITY  = $(CONFIG_EDGESIGHT_PRIORITY)
STACKSIZE = $(CONFIG_EDGESIGHT_STACKSIZE)
MODULE    = $(CONFIG_APP_EDGESIGHT)

MAINSRC = edgesight_main.c

CSRCS  = fall_detect.c postprocess.c npu_hal.c camera_hal.c

include $(APPDIR)/Application.mk
