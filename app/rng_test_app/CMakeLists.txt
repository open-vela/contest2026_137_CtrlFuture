# Contest 2026 team 137 RNG entropy self-test
if(CONFIG_LVX_USE_DEMO_CONTEST2026_137_RNG_TEST_APP)
  nuttx_add_application(
    NAME rng_test
    SRCS rng_test_main.c
    STACKSIZE 2048)
endif()
