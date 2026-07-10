*** Settings ***
Resource        resources/stm32n6-common.robot

*** Variables ***
${USART2_BASE}  0x40004400
${USART6_BASE}  0x42001400

*** Test Cases ***
USART2 Register Accessible
    Start STM32N6
    ${val}=    Execute Command    sysbus ReadDoubleWord ${USART2_BASE}
    ${val}=    Strip String    ${val}
    Should Be True    int(${val}) >= 0

USART6 Register Accessible
    Start STM32N6
    ${val}=    Execute Command    sysbus ReadDoubleWord ${USART6_BASE}
    ${val}=    Strip String    ${val}
    Should Be True    int(${val}) >= 0

USART1 Console Works
    Start STM32N6
    Wait For NSH
    Run NSH Command    help    Builtin Apps:

Boot Regression
    Start STM32N6
    Wait For NSH
