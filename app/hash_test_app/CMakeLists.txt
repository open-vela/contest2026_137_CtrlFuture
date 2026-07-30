# Contest 2026 team 137 HASH SHA-256 self-test
if(CONFIG_LVX_USE_DEMO_CONTEST2026_137_HASH_TEST_APP)
  nuttx_add_application(
    NAME hash_test
    SRCS hash_test_main.c
    STACKSIZE 2048)
endif()
