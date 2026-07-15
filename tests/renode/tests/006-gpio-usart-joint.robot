*** Settings ***
Resource        resources/stm32n6-common.robot

*** Test Cases ***
GPIOE Clock Enabled Via RCC AHB4ENR
    [Tags]    L2-state
    Start STM32N6
    # Enable GPIOE clock via AHB4ENR @ 0x4602825C, bit 4 = GPIOEEN
    Execute Command    sysbus WriteDoubleWord 0x4602825C 0x00000010
    ${val}=    Execute Command    sysbus ReadDoubleWord 0x4602825C
    ${val}=    Strip String    ${val}
    ${val}=    Convert To Integer    ${val}
    Should Be Equal As Integers    ${val}    0x00000010

GPIOE Port Accessible
    [Tags]    L2-state
    Start STM32N6
    # gpioPortE @ 0x46021000
    ${val}=    Execute Command    sysbus ReadDoubleWord 0x46021000
    ${val}=    Strip String    ${val}
    Should Be True    int(${val}) >= 0

USART1 AF7 Configurable on GPIOE Pins 5-6
    [Tags]    L2-state
    Start STM32N6
    # GPIOE AFRL @ 0x46021020 controls pins 0-7
    # Each pin uses 4 bits: pin5 = bits 20-23, pin6 = bits 24-27
    # AF7 for pin5 = 0x7 at bits 20-23 -> 0x700000
    # AF7 for pin6 = 0x7 at bits 24-27 -> 0x7000000
    # Combined: 0x07700000
    Execute Command    sysbus WriteDoubleWord 0x46021020 0x07700000
    ${val}=    Execute Command    sysbus ReadDoubleWord 0x46021020
    ${val}=    Strip String    ${val}
    ${val}=    Convert To Integer    ${val}
    Should Be Equal As Integers    ${val}    0x07700000

Boot Regression
    [Tags]    boot-regression
    Start STM32N6
    Wait For NSH
