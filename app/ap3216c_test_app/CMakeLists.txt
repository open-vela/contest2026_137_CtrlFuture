# Contest 2026 team 137 AP3216C I2C4 self-test
if(CONFIG_LVX_USE_DEMO_CONTEST2026_137_AP3216C_TEST_APP)
  nuttx_add_application(
    NAME ap3216c_test
    SRCS ap3216c_test_main.c
    STACKSIZE 2048)
endif()
