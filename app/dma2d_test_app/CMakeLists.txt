# Contest 2026 team 137 DMA2D de-risk probe
if(CONFIG_LVX_USE_DEMO_CONTEST2026_137_DMA2D_TEST_APP)
  nuttx_add_application(
    NAME dma2d_test
    SRCS dma2d_test_main.c
    STACKSIZE 2048)
endif()
