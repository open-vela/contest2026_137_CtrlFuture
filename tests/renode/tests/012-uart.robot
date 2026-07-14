*** Settings ***
Resource        resources/stm32n6-common.robot
# Tags: L1-register | L3-functional | boot-regression
Force Tags      uart

*** Variables ***
${USART2_BASE}  0x40004400
${USART6_BASE}  0x42001400

*** Test Cases ***
USART2 Register Accessible
    [Tags]    L1-register
    Start STM32N6
    ${val}=    Execute Command    sysbus ReadDoubleWord ${USART2_BASE}
    ${val}=    Strip String    ${val}
    Should Be True    int(${val}) >= 0

USART6 Register Accessible
    [Tags]    L1-register
    Start STM32N6
    ${val}=    Execute Command    sysbus ReadDoubleWord ${USART6_BASE}
    ${val}=    Strip String    ${val}
    Should Be True    int(${val}) >= 0

USART1 Console Works
    [Tags]    L2-state    L3-functional
    Start STM32N6
    Wait For NSH
    Run NSH Command    help    Builtin Apps:

USART1 Hello Builtin Works
    [Tags]    L3-functional
    Start STM32N6
    Wait For NSH
    Run NSH Command    hello    Hello, World!!

Boot Regression
    [Tags]    boot-regression
    Start STM32N6
    Wait For NSH
