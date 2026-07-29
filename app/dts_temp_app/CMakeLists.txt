# Contest 2026 team 137 DTS temperature self-test
if(CONFIG_LVX_USE_DEMO_CONTEST2026_137_DTS_TEMP_APP)
  nuttx_add_application(
    NAME dts_temp
    SRCS dts_temp_main.c
    STACKSIZE 2048)
endif()
