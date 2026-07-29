# Contest 2026 team 137 TIM PWM+capture loopback demo
if(CONFIG_LVX_USE_DEMO_CONTEST2026_137_TIM_CAP_APP)
  nuttx_add_application(
    NAME tim_cap
    SRCS tim_cap_main.c
    STACKSIZE 2048)
endif()
