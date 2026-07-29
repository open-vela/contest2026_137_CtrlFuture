# Contest 2026 team 137 LPTIM Stop-mode wakeup demo
if(CONFIG_LVX_USE_DEMO_CONTEST2026_137_LPTIM_STOP_APP)
  nuttx_add_application(
    NAME lptim_stop
    SRCS lptim_stop_main.c
    STACKSIZE 2048)
endif()
